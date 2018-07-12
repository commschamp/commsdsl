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

std::string RefField::getOpts(const std::string& scope) const
{
    StringsList options;
    options.push_back("TOpt");
    updateExtraOptions(scope, options);
    return common::listToString(options, ",\n", common::emptyString());
}

} // namespace commsdsl2comms
