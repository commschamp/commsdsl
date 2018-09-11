#include "OptionalField.h"

#include <type_traits>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string MembersDefTemplate =
    "/// @brief Scope for all the member fields of @ref #^#CLASS_NAME#$# optional.\n"
    "#^#EXTRA_PREFIX#$#\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#FIELD_DEF#$#\n"
    "};\n";

const std::string MembersOptionsTemplate =
    "/// @brief Extra options for all the member fields of @ref #^#SCOPE#$##^#CLASS_NAME#$# optional.\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#OPTIONS#$#\n"
    "};\n";

const std::string ClassTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::Optional<\n"
    "        #^#FIELD_REF#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::Optional<\n"
    "            #^#FIELD_REF#$##^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#PUBLIC#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "#^#PROTECTED#$#\n"
    "#^#PRIVATE#$#\n"
    "};\n"
);

const std::string StructTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::Optional<\n"
    "        #^#FIELD_REF#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#NAME#$#\n"
    "};\n"
);

bool shouldUseStruct(const common::ReplacementMap& replacements)
{
    auto hasNoValue =
        [&replacements](const std::string& val)
        {
            auto iter = replacements.find(val);
            return (iter == replacements.end()) || iter->second.empty();
        };

    return
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH") &&
        hasNoValue("PUBLIC") &&
        hasNoValue("PRIVATE") &&
        hasNoValue("PROTECTED");
}

} // namespace

bool OptionalField::prepareImpl()
{
    auto obj = optionalFieldDslObj();
    auto field = obj.field();
    assert(field.valid());

    if (!field.externalRef().empty()) {
        return true;
    }

    auto ptr = create(generator(), field);
    if (!ptr) {
        assert(!"should not happen");
        return false;
    }

    if (!ptr->prepare(obj.sinceVersion())) {
        return false;
    }

    m_field = std::move(ptr);
    return true;
}

void OptionalField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/Optional.h"
    };

    common::mergeIncludes(List, includes);

    if (m_field) {
        assert(m_field->externalRef().empty());
        m_field->updateIncludes(includes);
        return;
    }


    common::mergeInclude(generator().headerfileForField(optionalFieldDslObj().field().externalRef(), false), includes);
}

void OptionalField::updatePluginIncludesImpl(Field::IncludesList& includes) const
{
    if (m_field) {
        m_field->updatePluginIncludes(includes);
        return;
    }

    common::mergeInclude(generator().headerfileForFieldInPlugin(optionalFieldDslObj().field().externalRef(), false), includes);
}

std::string OptionalField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    if (!m_field) {
        // Find to mark it as used
        auto* foundField = generator().findField(optionalFieldDslObj().field().externalRef());
        if (foundField == nullptr) {
            assert(!"Should not happen");
        }
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("FIELD_REF", getFieldRef()));
    replacements.insert(std::make_pair("MEMBERS_STRUCT_DEF", getMembersDef(scope)));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["COMMA"] = ',';
    }

    if (!externalRef().empty()) {
        replacements.insert(std::make_pair("MEMBERS_OPT", "<TOpt>"));
    }

    auto* templ = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templ = &StructTemplate;
    }
    return common::processTemplate(*templ, replacements);
}

std::string OptionalField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    if (!m_field) {
        return common::emptyString();
    }

    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    auto fieldOptions = m_field->getDefaultOptions(memberScope);

    if (fieldOptions.empty()) {
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("OPTIONS", std::move(fieldOptions)));
    return common::processTemplate(MembersOptionsTemplate, replacements);
}

std::string OptionalField::getPluginAnonNamespaceImpl(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    if (!m_field) {
        return common::emptyString();
    }

    auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr();
    if (!externalRef().empty()) {
        fullScope += "<>";
    }
    fullScope += "::";

    auto prop = m_field->getPluginCreatePropsFunc(fullScope, forcedSerialisedHidden, serHiddenParam);

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#PROP#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROP", std::move(prop)));
    return common::processTemplate(Templ, replacements);
}

std::string OptionalField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    common::StringsList options;
    auto obj = optionalFieldDslObj();
    if (obj.cond().valid() || obj.externalModeCtrl()) {
        options.push_back(".uncheckable()");
    }

    if (m_field) {
        auto prefix =
            common::nameToClassCopy(name()) + common::membersSuffixStr() +
            "::createProps_";

        auto str = ".field(" + prefix + common::nameToAccessCopy(m_field->name()) + "(";
        if (serHiddenParam) {
            str += common::serHiddenStr();
        }
        str +="))";
        options.push_back(std::move(str));
        return common::listToString(options, "\n", common::emptyString());
    }

    auto field = optionalFieldDslObj().field();
    auto extRef = field.externalRef();
    auto& name = field.name();
    auto dispName = common::displayName(field.displayName(), name);

    auto str =
        ".field(" +
        generator().scopeForFieldInPlugin(extRef) +
        "createProps_" + common::nameToAccessCopy(name) + "(\"" +
        dispName + "\"";
    if (serHiddenParam) {
        str += ", " + common::serHiddenStr();
    }
    str += "))";
    options.push_back(std::move(str));
    return common::listToString(options, "\n", common::emptyString());
}

std::string OptionalField::getPrivateRefreshBodyImpl(const Field::FieldsList& fields) const
{
    auto c = cond();
    if (!c.valid()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "auto mode = comms::field::OptionalMode::Missing;\n"
        "if (#^#COND#$#) {\n"
        "    mode = comms::field::OptionalMode::Exists;\n"
        "}\n\n"
        "if (field_#^#NAME#$#()#^#FIELD_ACC#$#.getMode() == mode) {\n"
        "    return false;\n"
        "}\n\n"
        "field_#^#NAME#$#()#^#FIELD_ACC#$#.setMode(mode);\n"
        "return true;\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(name())));
    replacements.insert(std::make_pair("COND", dslCondToString(fields, c)));

    if (isVersionOptional()) {
        replacements.insert(std::make_pair("FIELD_ACC", ".field()"));
    }

    return common::processTemplate(Templ, replacements);
}

bool OptionalField::hasCustomReadRefreshImpl() const
{
    return cond().valid();
}

std::string OptionalField::getReadPreparationImpl(const FieldsList& fields) const
{
    static_cast<void>(fields);
    return "refresh_" + common::nameToAccessCopy(name()) + "();\n";
}

std::string OptionalField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);
    checkModeOpt(options);

    return common::listToString(options, ",\n", common::emptyString());
}

std::string OptionalField::getMembersDef(const std::string& scope) const
{
    if (!m_field) {
        return common::emptyString();
    }

    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    std::string fieldDef = m_field->getClassDefinition(memberScope);

    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().mainNamespace() + "::" + common::defaultOptionsStr() + ">";
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("FIELD_DEF", std::move(fieldDef)));
    return common::processTemplate(MembersDefTemplate, replacements);

}

std::string OptionalField::getFieldRef() const
{
    if (!m_field) {
        return generator().scopeForField(optionalFieldDslObj().field().externalRef(), true, true) + "<TOpt>";
    }

    std::string extraOpt;
    if (!externalRef().empty()) {
        extraOpt = "<TOpt>";
    }

    return
        "typename " + common::nameToClassCopy(name()) + common::membersSuffixStr() +
        extraOpt + "::" + common::nameToClassCopy(m_field->name());
}

void OptionalField::checkModeOpt(OptionalField::StringsList& options) const
{
    static const std::string Map[] = {
        common::emptyString(),
        "comms::option::ExistsByDefault",
        "comms::option::MissingByDefault"
    };

    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;

    static_assert(MapSize == static_cast<decltype(MapSize)>(commsdsl::OptionalField::Mode::NumOfValues), "Invalid map");

    auto obj = optionalFieldDslObj();
    auto mode = obj.defaultMode();
    auto idx = static_cast<decltype(MapSize)>(mode);
    if (MapSize <= idx) {
        assert(!"Should not happen");
        idx = 0U;
    }

    if (Map[idx].empty()) {
        return;
    }

    options.push_back(Map[idx]);
}

} // namespace commsdsl2comms
