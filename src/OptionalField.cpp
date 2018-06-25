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
    "/// @brief Scope for all the member fields of @ref #^#CLASS_NAME#$# bitfield.\n"
    "#^#EXTRA_PREFIX#$#\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#FIELD_DEF#$#\n"
    "};\n";

const std::string MembersOptionsTemplate =
    "/// @brief Extra options for all the member fields of @ref #^#SCOPE#$##^#CLASS_NAME#$# bitfield.\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#OPTIONS#$#\n"
    "};\n";

const std::string ClassTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::Optional<\n"
    "        typename #^#CLASS_NAME#$#Members::#^#FIELD_NAME#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::Optional<\n"
    "            typename #^#CLASS_NAME#$#Members::#^#FIELD_NAME#$##^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "};\n"
);

const std::string StructTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::Optional<\n"
    "        typename #^#CLASS_NAME#$#Members::#^#FIELD_NAME#$##^#COMMA#$#\n"
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
        hasNoValue("REFRESH");
}

} // namespace

bool OptionalField::prepareImpl()
{
    auto obj = optionalFieldDslObj();
    auto field = obj.field();
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

    if (m_field->externalRef().empty()) {
        m_field->updateIncludes(includes);
    }
    else {
        common::mergeInclude(generator().headerfileForField(m_field->externalRef(), false), includes);
    }
}

std::string OptionalField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    assert(m_field);
    auto fieldRef = m_field->externalRef();
    if (!fieldRef.empty()) {
        // Find to mark it as used
        auto* foundField = generator().findField(fieldRef);
        if (foundField == nullptr) {
            assert(!"Should not happen");
        }
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(suffix)));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("FIELD_NAME", common::nameToClassCopy(m_field->name())));
    replacements.insert(std::make_pair("MEMBERS_STRUCT_DEF", getMembersDef(scope, suffix)));
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
    assert(m_field);
    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    auto fieldOptions = m_field->getDefaultOptions(memberScope);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("OPTIONS", std::move(fieldOptions)));
    return common::processTemplate(MembersOptionsTemplate, replacements);
}

std::string OptionalField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);
    checkModeOpt(options);

    return common::listToString(options, ",\n", common::emptyString());
}

std::string OptionalField::getMembersDef(const std::string& scope, const std::string& suffix) const
{
    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    assert(m_field);
    std::string fieldDef = m_field->getClassDefinition(memberScope);

    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().mainNamespace() + "::" + common::defaultOptionsStr() + ">";
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name() + suffix)));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("FIELD_DEF", std::move(fieldDef)));
    return common::processTemplate(MembersDefTemplate, replacements);

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
