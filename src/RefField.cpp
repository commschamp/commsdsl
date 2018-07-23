#include "RefField.h"

#include <type_traits>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string AliasTemplate(
    "#^#PREFIX#$#"
    "using #^#CLASS_NAME#$# =\n"
    "    #^#REF_FIELD#$#<\n"
    "       #^#OPTS#$#\n"
    "   >;\n"
);

const std::string StructTemplate(
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    #^#REF_FIELD#$#<\n"
    "       #^#OPTS#$#\n"
    "   >\n"
    "{\n"
    "    #^#NAME_FUNC#$#\n"
    "};\n"
);

} // namespace

void RefField::updateIncludesImpl(IncludesList& includes) const
{
    auto inc =
        generator().headerfileForField(refFieldDslObj().field().externalRef(), false);
    common::mergeInclude(inc, includes);
}

void RefField::updatePluginIncludesImpl(IncludesList& includes) const
{
    auto inc =
        generator().headerfileForFieldInPlugin(refFieldDslObj().field().externalRef(), false);
    common::mergeInclude(inc, includes);
}

std::size_t RefField::minLengthImpl() const
{
    auto refObj = refFieldDslObj().field();
    auto fieldPtr = generator().findField(refObj.externalRef());
    if (fieldPtr == nullptr) {
        assert(!"Unexpected");
        return dslObj().minLength();
    }

    return fieldPtr->minLength();
}

std::size_t RefField::maxLengthImpl() const
{
    auto refObj = refFieldDslObj().field();
    auto fieldPtr = generator().findField(refObj.externalRef());
    if (fieldPtr == nullptr) {
        assert(!"Unexpected");
        return dslObj().maxLength();
    }

    return fieldPtr->maxLength();
}

std::string RefField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    auto refObj = refFieldDslObj().field();
    auto fieldPtr = generator().findField(refObj.externalRef());
    if (fieldPtr == nullptr) {
        assert(!"Unexpected");
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("NAME_FUNC", getNameFunc()));
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(suffix)));
    replacements.insert(
        std::make_pair(
            "REF_FIELD",
            generator().scopeForField(
                refFieldDslObj().field().externalRef(), true, true)));
    replacements.insert(std::make_pair("OPTS", getOpts(scope)));

    auto* templ = &AliasTemplate;
    if (getDisplayName() != fieldPtr->getDisplayName()) {
        templ = &StructTemplate;
    }
    // TODO: check display name is empty

    return common::processTemplate(*templ, replacements);
}

std::string RefField::getCompareToValueImpl(
    const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto obj = refFieldDslObj();
    auto field = obj.field();
    if (!field.valid()) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    auto* fieldPtr = generator().findField(field.externalRef());
    if (fieldPtr == nullptr) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    bool versionOptional = forcedVersionOptional || isVersionOptional();
    return fieldPtr->getCompareToValue(op, value, usedName, versionOptional);
}

std::string RefField::getCompareToFieldImpl(
    const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    auto obj = refFieldDslObj();
    auto dslField = obj.field();
    if (!dslField.valid()) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    auto* fieldPtr = generator().findField(dslField.externalRef());
    if (fieldPtr == nullptr) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    bool versionOptional = forcedVersionOptional || isVersionOptional();
    return fieldPtr->getCompareToField(op, field, usedName, versionOptional);
}

std::string RefField::getPluginPropsDefFuncBodyImpl(
    const std::string& scope,
    bool externalName,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    static const std::string Templ =
        "return #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$##^#SER_HIDDEN#$#);\n";

    static const std::string TemplWithField =
        "using Field = #^#FIELD_SCOPE#$##^#CLASS_NAME#$##^#TEMPL_PARAMS#$#;\n"
        "return #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$##^#SER_HIDDEN#$#);\n";

    static const std::string VerOptTempl =
        "using InnerField = #^#FIELD_SCOPE#$##^#CLASS_NAME#$#Field;\n"
        "auto props = #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$##^#SER_HIDDEN#$#);\n\n"
        "using Field = #^#FIELD_SCOPE#$##^#CLASS_NAME#$##^#TEMPL_PARAMS#$#;\n"
        "return\n"
        "    cc::property::field::ForField<Field>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        .field(std::move(props))\n"
        "        .asMap();\n";

    bool verOptional = isVersionOptional();
    auto* templ = &Templ;
    if (verOptional) {
        templ = &VerOptTempl;
    }
    else if (!externalName) {
        templ = &TemplWithField;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PLUGIN_SCOPE", generator().scopeForFieldInPlugin(refFieldDslObj().field().externalRef())));
    replacements.insert(std::make_pair("REF_NAME", common::nameToAccessCopy(refFieldDslObj().field().name())));
    if (!scope.empty()) {
        replacements.insert(std::make_pair("FIELD_SCOPE", scope));
    }
    else {
        replacements.insert(std::make_pair("FIELD_SCOPE", generator().scopeForField(externalRef(), true, true)));
    }

    if (externalName) {
        replacements.insert(std::make_pair("NAME_PROP", "name"));
        replacements.insert(std::make_pair("TEMPL_PARAMS", "<>"));
    }
    else if (verOptional) {
        replacements.insert(std::make_pair("NAME_PROP", "InnerField::name()"));
    }
    else {
        replacements.insert(std::make_pair("NAME_PROP", "Field::name()"));
    }

    if (forcedSerialisedHidden) {
        replacements.insert(std::make_pair("SER_HIDDEN", ", true"));
    }
    else if (serHiddenParam) {
        replacements.insert(std::make_pair("SER_HIDDEN", ", serHidden"));
    }

    return common::processTemplate(*templ, replacements);
}

std::string RefField::getOpts(const std::string& scope) const
{
    StringsList options;
    options.push_back("TOpt");
    updateExtraOptions(scope, options);
    return common::listToString(options, ",\n", common::emptyString());
}

} // namespace commsdsl2comms
