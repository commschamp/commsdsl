//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
    "        #^#OPTS#$#\n"
    "    >;\n"
);

const std::string ClassTemplate(
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    #^#REF_FIELD#$#<\n"
    "        #^#OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base=\n"
    "        #^#REF_FIELD#$#<\n"
    "            #^#OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#PUBLIC#$#\n"
    "    #^#NAME_FUNC#$#\n"
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
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    #^#REF_FIELD#$#<\n"
    "        #^#OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#NAME_FUNC#$#\n"
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

std::string RefField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    auto refObj = refFieldDslObj().field();
    auto fieldPtr = generator().findField(refObj.externalRef());
    if (fieldPtr == nullptr) {
        assert(!"Unexpected");
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));

    if (displayName() != fieldPtr->displayName()) {
        replacements.insert(std::make_pair("NAME_FUNC", getNameFunc()));
    }

    replacements.insert(
        std::make_pair(
            "REF_FIELD",
            generator().scopeForField(
                refFieldDslObj().field().externalRef(), true, true)));
    replacements.insert(std::make_pair("OPTS", getOpts(scope)));

    auto* templ = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templ = &StructTemplate;
        if ((replacements.find("NAME_FUNC") == replacements.end()) &&
            (refObj.bitLength() == 0U)) {
            templ = &AliasTemplate;
        }
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
        "#^#SER_HIDDEN_CAST#$#\n"
        "return #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$##^#SER_HIDDEN#$#);\n";

    static const std::string TemplWithField =
        "#^#SER_HIDDEN_CAST#$#\n"
        "using Field = #^#FIELD_SCOPE#$##^#CLASS_NAME#$##^#TEMPL_PARAMS#$#;\n"
        "return #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$##^#SER_HIDDEN#$#);\n";

    static const std::string VerOptTempl =
        "#^#SER_HIDDEN_CAST#$#\n"
        "using InnerField = #^#FIELD_SCOPE#$##^#CLASS_NAME#$#Field;\n"
        "auto props = #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$##^#SER_HIDDEN#$#);\n\n"
        "using Field = #^#FIELD_SCOPE#$##^#CLASS_NAME#$##^#TEMPL_PARAMS#$#;\n"
        "return\n"
        "    cc::property::field::ForField<Field>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        .uncheckable()\n"
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

    static const std::string CastStr("static_cast<void>(serHidden);");
    if (forcedSerialisedHidden) {
        replacements.insert(std::make_pair("SER_HIDDEN", ", true"));
        if (serHiddenParam) {
            replacements.insert(std::make_pair("SER_HIDDEN_CAST", CastStr));
        }
    }
    else if (serHiddenParam) {
        replacements.insert(std::make_pair("SER_HIDDEN", ", serHidden"));
    }

    return common::processTemplate(*templ, replacements);
}

std::string RefField::getOpts(const std::string& scope) const
{
    StringsList options;

    if (isForcedNoOptionsConfig()) {
        options.push_back(generator().scopeForOptions(common::defaultOptionsStr(), true, true));
    }
    else {
        options.push_back("TOpt");
    }

    updateExtraOptions(scope, options);
    auto obj = refFieldDslObj();
    auto bitLength = obj.bitLength();
    if (bitLength != 0U) {
        options.push_back("comms::option::FixedBitLength<" + common::numToString(bitLength) + '>');
    }
    return common::listToString(options, ",\n", common::emptyString());
}

} // namespace commsdsl2comms
