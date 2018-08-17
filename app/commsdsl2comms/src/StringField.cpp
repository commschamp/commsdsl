#include "StringField.h"

#include <type_traits>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#PREFIX_FIELD#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::String<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<>#^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::String<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<>#^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#CONSTRUCTOR#$#\n"
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
    "#^#PREFIX_FIELD#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::String<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<>#^#COMMA#$#\n"
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
        hasNoValue("CONSTRUCTOR") &&
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH") &&
        hasNoValue("PUBLIC") &&
        hasNoValue("PROTECTED") &&
        hasNoValue("PRIVATE");
}

} // namespace

bool StringField::prepareImpl()
{
    auto obj = stringFieldDslObj();
    if (!obj.hasLengthPrefixField()) {
        return true;
    }

    auto prefix = obj.lengthPrefixField();
    if (!prefix.externalRef().empty()) {
        return true;
    }

    m_prefix = create(generator(), prefix);
    if (!m_prefix->prepare(dslObj().sinceVersion())) {
        return false;
    }

    return true;
}

void StringField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/String.h"
    };

    common::mergeIncludes(List, includes);

    auto obj = stringFieldDslObj();
    if (obj.hasZeroTermSuffix()) {
        static const IncludesList TermFieldList = {
            "comms/field/IntValue.h",
            "<cstdint>"
        };

        common::mergeIncludes(TermFieldList, includes);
    }

    if (m_prefix) {
        m_prefix->updateIncludes(includes);
        return;
    }

    if (obj.hasLengthPrefixField()) {
        auto prefix = obj.lengthPrefixField();
        assert(prefix.valid());
        auto prefixRef = prefix.externalRef();
        assert(!prefixRef.empty());
        common::mergeInclude(generator().headerfileForField(prefixRef, false), includes);
    }
}

std::size_t StringField::maxLengthImpl() const
{
    auto obj = stringFieldDslObj();
    if (obj.fixedLength() != 0U) {
        return Base::maxLengthImpl();
    }

    return std::numeric_limits<std::size_t>::max();
}

std::string StringField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
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
    replacements.insert(std::make_pair("CONSTRUCTOR", getConstructor(className)));
    replacements.insert(std::make_pair("PREFIX_FIELD", getPrefixField(scope)));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements.insert(std::make_pair("COMMA", ","));
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string StringField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    if (!m_prefix) {
        return common::emptyString();
    }

    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    auto fieldOptions = m_prefix->getDefaultOptions(memberScope);

    const std::string Templ =
        "/// @brief Extra options for all the member fields of @ref #^#SCOPE#$##^#CLASS_NAME#$# string.\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#OPTIONS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("OPTIONS", std::move(fieldOptions)));
    return common::processTemplate(Templ, replacements);
}

std::string StringField::getCompareToValueImpl(
    const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    bool versionOptional = forcedVersionOptional || isVersionOptional();
    if (versionOptional) {
        return
            "field_" + usedName + "().doesExist() &&\n"
            "(field_" + usedName + "().field().value() " +
            op + " \"" + value + "\")";
    }

    return
        "field_" + usedName + "().value() " +
        op + " \"" + value + '\"';
}

std::string StringField::getCompareToFieldImpl(
    const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    bool thisOptional = forcedVersionOptional || isVersionOptional();
    bool otherOptional = field.isVersionOptional();

    auto fieldName = common::nameToAccessCopy(field.name());

    std::string thisFieldValue;
    if (thisOptional) {
        thisFieldValue = "field_" + usedName + "().field().value() ";
    }
    else {
        thisFieldValue = "field_" + usedName + "().value() ";
    }

    std::string otherFieldValue;
    if (otherOptional) {
        otherFieldValue = " field_" + fieldName + "().field().value()";
    }
    else {
        otherFieldValue = " field_" + fieldName + "().value()";
    }

    auto compareExpr = thisFieldValue + op + otherFieldValue;

    if ((!thisOptional) && (!otherOptional)) {
        return compareExpr;
    }


    if ((!thisOptional) && (otherOptional)) {
        return
            "field_" + fieldName + "().doesExist() &&\n(" +
            compareExpr + ")";
    }

    if ((thisOptional) && (!otherOptional)) {
        return
            "field_" + usedName + "().doesExist() &&\n(" +
            compareExpr + ")";
    }

    return
        "field_" + usedName + "().doesExist() &&\n" +
        "field_" + fieldName + "().doesExist() &&\n(" +
        compareExpr + ")";
}

std::string StringField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);
    checkFixedLengthOpt(options);
    checkPrefixOpt(options);
    checkSuffixOpt(options);

    return common::listToString(options, ",\n", common::emptyString());
}

std::string StringField::getConstructor(const std::string& className) const
{
    auto obj = stringFieldDslObj();
    auto& defaultValue = obj.defaultValue();
    if (defaultValue.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Default constructor\n"
        "#^#CLASS_NAME#$#()\n"
        "{\n"
        "    static const char Str[] = \"#^#STR#$#\";\n"
        "    static const std::size_t StrSize = std::extent<decltype(Str)>::value;\n"
        "    Base::value() = typename Base::ValueType(&Str[0], StrSize - 1);\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("STR", defaultValue));
    return common::processTemplate(Templ, replacements);
}

std::string StringField::getPrefixField(const std::string& scope) const
{
    if (!m_prefix) {
        return common::emptyString();
    }

    auto membersScope =
        scope + common::nameToClassCopy(name()) +
        common::membersSuffixStr() + "::";

    auto fieldDef = m_prefix->getClassDefinition(membersScope);
    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().mainNamespace() + "::" + common::defaultOptionsStr() + ">";
    }

    static const std::string Templ =
        "/// @brief Scope for all the member fields of @ref #^#CLASS_NAME#$# string.\n"
        "#^#EXTRA_PREFIX#$#\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#FIELD_DEF#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("FIELD_DEF", std::move(fieldDef)));
    return common::processTemplate(Templ, replacements);
}

void StringField::checkFixedLengthOpt(StringField::StringsList& list) const
{
    auto obj = stringFieldDslObj();
    auto fixedLen = obj.fixedLength();
    if (fixedLen == 0U) {
        return;
    }

    auto str =
        "comms::option::SequenceFixedSize<" +
        common::numToString(static_cast<std::uintmax_t>(fixedLen)) +
        ">";
    list.push_back(std::move(str));
}

void StringField::checkPrefixOpt(StringField::StringsList& list) const
{
    auto obj = stringFieldDslObj();
    if (!obj.hasLengthPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_prefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_prefix->name());
    }
    else {
        auto prefixField = obj.lengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    list.push_back("comms::option::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void StringField::checkSuffixOpt(StringField::StringsList& list) const
{
    auto obj = stringFieldDslObj();
    if (!obj.hasZeroTermSuffix()) {
        return;
    }

    static const std::string Templ =
        "comms::option::SequenceTerminationFieldSuffix<\n"
        "    comms::field::IntValue<\n"
        "        #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "        std::uint8_t,\n"
        "        comms::option::ValidNumValueRange<0, 0>\n"
        "    >\n"
        ">";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    list.push_back(common::processTemplate(Templ, replacements));
}

} // namespace commsdsl2comms
