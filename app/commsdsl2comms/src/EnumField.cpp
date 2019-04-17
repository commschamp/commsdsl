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

#include "EnumField.h"

#include <type_traits>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "IntField.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#ENUMERATION#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::EnumValue<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#ENUM_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::EnumValue<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#ENUM_TYPE#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#PUBLIC#$#\n"
    "    #^#NAME#$#\n"
    "    #^#VALUE_NAME#$#\n"
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
    "#^#ENUMERATION#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::EnumValue<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#ENUM_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#NAME#$#\n"
    "    #^#VALUE_NAME#$#\n"
    "};\n"
);

const std::size_t MaxRangesInOpts = 5U;

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
        hasNoValue("PROTECTED") &&
        hasNoValue("PRIVATE");
}

} // namespace

common::StringsList EnumField::getValuesList() const
{
    if (!prepareRanges()) {
        return common::StringsList();
    }

    auto obj = enumFieldDslObj();
    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::EnumField::Type::Uint64) ||
        (type == commsdsl::EnumField::Type::Uintvar);

    common::StringsList valuesStrings;
    auto& values = obj.values();
    for (auto& v : obj.revValues()) {
        auto iter = values.find(v.second);
        if (iter == values.end()) {
            assert(!"Should not happen");
            continue;
        }

        bool exists =
            generator().doesElementExist(
                iter->second.m_sinceVersion,
                iter->second.m_deprecatedSince,
                false);

        if (!exists) {
            continue;
        }

        static const std::string Templ =
            "#^#NAME#$# = #^#VALUE#$#, \n";

        unsigned hexW = hexWidth();

        std::string valStr;
        if ((bigUnsigned) || (0U < hexW)) {
            valStr = common::numToString(static_cast<std::uintmax_t>(v.first), hexW);
        }
        else {
            valStr = common::numToString(v.first);
        }

        std::string docStr;
        if (!iter->second.m_description.empty()) {
            docStr = "///< " + iter->second.m_description;
            docStr = common::makeMultilineCopy(docStr, 40);
            ba::replace_all(docStr, "\n", "\n///  ");
        }
        else if (dslObj().semanticType() == commsdsl::Field::SemanticType::MessageId) {
            if (!iter->second.m_displayName.empty()) {
                docStr = "///< message id <b>" + iter->second.m_displayName + "</b>.";
            }
            else {
                docStr = "///< message id @b " + v.second;
            }
        }
        else if (!iter->second.m_displayName.empty()) {
            docStr = "///< value <b>" + iter->second.m_displayName + "</b>.";
        }
        else {
            docStr = "///< value @b " + v.second;
        }

        assert(!valStr.empty());
        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("NAME", v.second));
        replacements.insert(std::make_pair("VALUE", std::move(valStr)));
        auto templ = common::processTemplate(Templ, replacements);
        assert(2U <= templ.size());
        static const std::string DocElem("#^#DOC#$#");
        templ.insert((templ.end() - 1U), DocElem.begin(), DocElem.end());

        common::ReplacementMap docRepl;
        docRepl.insert(std::make_pair("DOC", std::move(docStr)));
        valuesStrings.push_back(common::processTemplate(templ, docRepl));
    }
    return valuesStrings;
}

std::string EnumField::getValuesDefinition() const
{
    common::StringsList values = getValuesList();

    return common::listToString(values, common::emptyString(), common::emptyString());
}

std::string EnumField::getValueName(std::intmax_t value) const
{
    auto obj = enumFieldDslObj();
    auto& revValues = obj.revValues();
    auto iter = revValues.find(value);
    if (iter != revValues.end()) {
        return iter->second;
    }

    return common::emptyString();
}

const std::string& EnumField::underlyingType() const
{
    auto obj = enumFieldDslObj();
    return IntField::convertType(obj.type());
}

bool EnumField::isUnsignedUnderlyingType() const
{
    auto obj = enumFieldDslObj();
    return IntField::isUnsignedType(obj.type());
}

unsigned EnumField::hexWidth() const
{
    auto obj = enumFieldDslObj();

    unsigned hexWidth = 0U;
    if (obj.hexAssign()) {
        hexWidth = obj.maxLength() * 2U;
    }
    return hexWidth;
}

void EnumField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/EnumValue.h",
        "<cstdint>"
    };

    common::mergeIncludes(List, includes);
    if (dslObj().semanticType() == commsdsl::Field::SemanticType::MessageId) {
        auto inc =
            generator().mainNamespace() + '/' +
            common::msgIdEnumNameStr() + common::headerSuffix();
        common::mergeInclude(inc, includes);
    }

    prepareRanges();

    if (MaxRangesInOpts < m_validRanges.size()) {
        common::mergeInclude("<iterator>", includes);
        common::mergeInclude("<algorithm>", includes);
    }

    if (isDirectValueNameMapping()) {
        common::mergeInclude("<type_traits>", includes);       
    }
    else {
        common::mergeInclude("<iterator>", includes);
        common::mergeInclude("<algorithm>", includes);
        common::mergeInclude("<utility>", includes);
    }
}

std::string EnumField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    if (!prepareRanges()) {
        return common::emptyString();
    }
    
    std::string extraDoxStr;
    if (dslObj().semanticType() != commsdsl::Field::SemanticType::MessageId) {
        auto scopeStr = scope + getEnumType(common::nameToClassCopy(name()));
        static const std::string OptPrefix("TOpt");
        if (ba::starts_with(scopeStr, OptPrefix)) {
            scopeStr = generator().mainNamespace() + scopeStr.substr(OptPrefix.size());
        }

        extraDoxStr = "@see @ref " + scopeStr;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ENUMERATION", getEnumeration(scope)));
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className, true, std::string(), extraDoxStr)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("ENUM_TYPE", getEnumType(common::nameToClassCopy(name()))));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("VALUE_NAME", getValueNameFunc()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));

    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["ENUM_TYPE"] += ',';
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string EnumField::getCompareToValueImpl(
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

    auto strGenFunc =
        [&op, &usedName, versionOptional](const std::string& v)
        {
            if (!versionOptional) {
                return
                    "field_" + usedName + "().value() " +
                    op + " static_cast<typename std::decay<decltype(field_" +
                    usedName + "().value())>::type>(" + v + ')';
            }

            return
                "field_" + usedName + "().doesExist() &&\n"
                "(field_" + usedName + "().field().value() " +
                op + " static_cast<typename std::decay<decltype(field_" +
                usedName + "().field().value())>::type>(" + v + "))";

        };

    try {
        auto val = static_cast<std::intmax_t>(std::stoll(value));
        return strGenFunc(common::numToString(val));
    }
    catch (...) {
        // nothing to do
    }

    auto obj = enumFieldDslObj();
    auto& values = obj.values();
    auto iter = values.find(value);
    if (iter != values.end()) {
        return strGenFunc(common::numToString(iter->second.m_value));
    }

    auto lastDot = value.find_last_of(".");
    if (lastDot == std::string::npos) {
        assert(!"Should not happen");
        return Base::getCompareToValueImpl(op, value, nameOverride, forcedVersionOptional);
    }

    auto* otherEnum = generator().findField(std::string(value, 0, lastDot), false);
    if ((otherEnum == nullptr) || (otherEnum->kind() != commsdsl::Field::Kind::Enum)) {
        assert(!"Should not happen");
        return Base::getCompareToValueImpl(op, value, nameOverride, forcedVersionOptional);
    }

    auto& castedOtherEnum = static_cast<const EnumField&>(*otherEnum);
    auto& otherValues = castedOtherEnum.enumFieldDslObj().values();
    auto otherIter = otherValues.find(std::string(value, lastDot + 1));
    if (otherIter == otherValues.end()) {
        assert(!"Should not happen");
        return Base::getCompareToValueImpl(op, value, nameOverride, forcedVersionOptional);
    }

    return strGenFunc(common::numToString(otherIter->second.m_value));
}

std::string EnumField::getCompareToFieldImpl(
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
        thisFieldValue = "field_" + usedName + "().field().value()";
    }
    else {
        thisFieldValue = "field_" + usedName + "().value()";
    }

    std::string otherFieldValue;
    if (otherOptional) {
        otherFieldValue = "field_" + fieldName + "().field().value()";
    }
    else {
        otherFieldValue = "field_" + fieldName + "().value()";
    }

    std::string compExpr =
        thisFieldValue + ' ' + op +
        " static_cast<typename std::decay<decltype(" + thisFieldValue + ")>::type>(" + otherFieldValue + ')';

    if ((!thisOptional) && (!otherOptional)) {
        return compExpr;
    }

    if ((!thisOptional) && (otherOptional)) {
        return
            "field_" + fieldName + "().doesExist() &&\n(" +
            compExpr + ')';
    }

    if ((thisOptional) && (!otherOptional)) {
        return
            "field_" + usedName + "().doesExist() &&\n(" +
            compExpr + ')';
    }


    return
        "field_" + usedName + "().doesExist() &&\n" +
        "field_" + fieldName + "().doesExist() &&\n(" +
            compExpr + ')';
}

std::string EnumField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    static_cast<void>(serHiddenParam);
    common::StringsList props;
    auto obj = enumFieldDslObj();
    auto& values = obj.values();
    auto& revValues = obj.revValues();
    props.reserve(revValues.size());
    std::intmax_t prevValue = 0;
    bool prevValueValid = false;
    for (auto& rVal : revValues) {
        if ((prevValueValid) && (prevValue == rVal.first)) {
            continue;
        }

        auto iter = values.find(rVal.second);
        assert(iter != values.end());
        auto& v = *iter;

        if (!generator().doesElementExist(v.second.m_sinceVersion, v.second.m_deprecatedSince, true)) {
            continue;
        }

        prevValueValid = true;
        prevValue = rVal.first;

        auto* valName = &v.second.m_displayName;
        if (valName->empty()) {
            valName = &v.first;
        }
        props.push_back(".add(\"" + *valName + "\", " + common::numToString(v.second.m_value) + ")");
    }
    return common::listToString(props, "\n", common::emptyString());
}

std::string EnumField::getEnumeration(const std::string& scope) const
{
    if (dslObj().semanticType() == commsdsl::Field::SemanticType::MessageId) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Values enumerator for @ref #^#SCOPE#$# field.\n"
        "enum class #^#NAME#$#Val : #^#TYPE#$#\n"
        "{\n"
        "    #^#VALUES#$#\n"
        "};\n";

    auto scopeStr = scope + common::nameToClassCopy(name());
    static const std::string OptPrefix("TOpt");
    if (ba::starts_with(scopeStr, OptPrefix)) {
        scopeStr = generator().mainNamespace() + scopeStr.substr(OptPrefix.size());
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", std::move(scopeStr)));
    replacements.insert(std::make_pair("TYPE", IntField::convertType(enumFieldDslObj().type())));
    replacements.insert(std::make_pair("VALUES", getValuesDefinition()));
    return common::processTemplate(Templ, replacements);
}

std::string EnumField::getFieldBaseParams() const
{
    auto obj = enumFieldDslObj();
    auto endian = obj.endian();
    return getCommonFieldBaseParams(endian);
}

std::string EnumField::getEnumType(const std::string& className) const
{
    if (dslObj().semanticType() == commsdsl::Field::SemanticType::MessageId) {
        return generator().mainNamespace() + "::" + common::msgIdEnumNameStr();
    }

    return className + "Val";
}

std::string EnumField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    checkDefaultValueOpt(options);
    checkLengthOpt(options);
    checkValidRangesOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string EnumField::getValid() const
{
    auto custom = getCustomValid();
    if (!custom.empty()) {
        return custom;
    }

    auto obj = enumFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {

        if (m_validRanges.size() <= MaxRangesInOpts) {
            return common::emptyString(); // Already in options
        }

        static const std::string Templ =
            "/// @brief Validity check function.\n"
            "bool valid() const\n"
            "{\n"
            "    if (!Base::valid()) {\n"
            "        return false;\n"
            "    }\n\n"
            "    static const typename Base::ValueType Values[] = {\n"
            "        #^#VALUES#$#\n"
            "    };\n\n"
            "    auto iter =\n"
            "        std::lower_bound(std::begin(Values), std::end(Values), Base::value());\n\n"
            "    if ((iter == std::end(Values)) || (*iter != Base::value())) {\n"
            "        return false;\n"
            "    }\n\n"
            "    return true;\n"
            "}";

        common::StringsList valuesStrings;

        bool isMessageId =
            obj.semanticType() == commsdsl::Field::SemanticType::MessageId;
        auto& revValues = obj.revValues();
        auto prevIter = revValues.end();
        for (auto iter = revValues.begin(); iter != revValues.end(); ++iter) {

            if ((prevIter != revValues.end()) && (prevIter->first == iter->first)) {
                continue;
            }

            std::string prefix;
            if (isMessageId) {
                 prefix = generator().mainNamespace() + "::" + common::msgIdPrefixStr();
            }
            else {
                prefix = common::nameToClassCopy(name()) + "Val::";
            }

            valuesStrings.push_back(prefix + iter->second);
            prevIter = iter;
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("VALUES", common::listToString(valuesStrings, ",\n", common::emptyString())));
        return common::processTemplate(Templ, replacements);
    }

    // version must be taken into account
    std::vector<decltype(m_validRanges)> rangesToProcess;
    for (auto& r : m_validRanges) {
        if ((r.m_sinceVersion == 0U) &&
            (r.m_deprecatedSince == commsdsl::Protocol::notYetDeprecated())) {
            continue;
        }

        if ((rangesToProcess.empty()) ||
            (rangesToProcess.back().back().m_sinceVersion != r.m_sinceVersion) ||
            (rangesToProcess.back().back().m_deprecatedSince != r.m_deprecatedSince)){
            rangesToProcess.resize(rangesToProcess.size() + 1);
        }

        rangesToProcess.back().push_back(r);
    }

    static const std::string VersionBothCondTempl =
        "if ((#^#FROM_VERSION#$# <= Base::getVersion()) &&\n"
        "    (Base::getVersion() < #^#UNTIL_VERSION#$#)) {\n"
        "    #^#COMPARISONS#$#\n"
        "}\n";

    static const std::string VersionFromCondTempl =
        "if (#^#FROM_VERSION#$# <= Base::getVersion()) {\n"
        "    #^#COMPARISONS#$#\n"
        "}\n";

    static const std::string VersionUntilCondTempl =
        "if (Base::getVersion() < #^#UNTIL_VERSION#$#) {\n"
        "    #^#COMPARISONS#$#\n"
        "}\n";

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::EnumField::Type::Uint64) ||
        (type == commsdsl::EnumField::Type::Uintvar);


    common::StringsList conditions;
    for (auto& l : rangesToProcess) {
        assert(!l.empty());
        auto* condTempl = &VersionBothCondTempl;
        if (l.front().m_sinceVersion == 0U) {
            assert(l.front().m_deprecatedSince != commsdsl::Protocol::notYetDeprecated());
            condTempl = &VersionUntilCondTempl;
        }
        else if (commsdsl::Protocol::notYetDeprecated() <= l.front().m_deprecatedSince) {
            condTempl = &VersionFromCondTempl;
        }

        common::StringsList comparisons;
        for (auto& r : l) {
            static const std::string ValueBothCompTempl =
                "if ((static_cast<typename Base::ValueType>(#^#MIN_VALUE#$#) <= Base::value()) &&\n"
                "    (Base::value() <= static_cast<typename Base::ValueType>(#^#MAX_VALUE#$#))) {\n"
                "    return true;\n"
                "}";

            static const std::string ValueSingleCompTempl =
                "if (Base::value() == static_cast<typename Base::ValueType>(#^#MIN_VALUE#$#)) {\n"
                "    return true;\n"
                "}";


            std::string minValue;
            std::string maxValue;
            if (bigUnsigned) {
                minValue = common::numToString(static_cast<std::uintmax_t>(r.m_min));
                maxValue = common::numToString(static_cast<std::uintmax_t>(r.m_max));
            }
            else {
                minValue = common::numToString(r.m_min);
                maxValue = common::numToString(r.m_max);
            }

            common::ReplacementMap repl;
            repl.insert(std::make_pair("MIN_VALUE", std::move(minValue)));
            repl.insert(std::make_pair("MAX_VALUE", std::move(maxValue)));

            auto* templ = &ValueBothCompTempl;
            if (r.m_min == r.m_max) {
                templ = &ValueSingleCompTempl;
            }
            comparisons.push_back(common::processTemplate(*templ, repl));
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("COMPARISONS", common::listToString(comparisons, "\n\n", common::emptyString())));
        replacements.insert(std::make_pair("FROM_VERSION", common::numToString(l.front().m_sinceVersion)));
        replacements.insert(std::make_pair("UNTIL_VERSION", common::numToString(l.front().m_deprecatedSince)));
        conditions.push_back(common::processTemplate(*condTempl, replacements));
    }

    static const std::string Templ =
        "/// @brief Validity check function.\n"
        "bool valid() const\n"
        "{\n"
        "    if (Base::valid()) {\n"
        "        return true;\n"
        "    }\n\n"
        "    #^#CONDITIONS#$#\n"
        "    return false;\n"
        "}\n";

    std::string condStr = common::listToString(conditions, "\n", common::emptyString());
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CONDITIONS", std::move(condStr)));
    return common::processTemplate(Templ, replacements);
}

std::string EnumField::getValueNameFunc() const
{
    auto obj = enumFieldDslObj();
    std::string body;
    if (isDirectValueNameMapping()) {
        body = getValueNameFuncDirectBody();
    }
    else {
        body = getValueNameFuncBinSearchBody();
    }

    static const std::string Templ = 
        "/// @brief Retrieve name of the enum value\n"
        "static const char* valueName(#^#ENUM_TYPE#$# val)\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";


    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ENUM_TYPE", getEnumType(common::nameToClassCopy(name()))));
    replacements.insert(std::make_pair("BODY", std::move(body)));
    return common::processTemplate(Templ, replacements);
}

std::string EnumField::getValueNameFuncDirectBody() const
{
    auto obj = enumFieldDslObj();
    auto& revValues = obj.revValues();
    auto& values = obj.values();
    assert(!revValues.empty());

    std::intmax_t nextValue = 0;
    StringsList names;
    for (auto& v : revValues) {
        if (v.first < nextValue) {
            continue;
        }

        while (nextValue < v.first) {
            names.push_back("nullptr");
            ++nextValue;
        }

        auto getDisplayNameFunc = 
            [](auto& infoPair) -> const std::string&
            {
                if (infoPair.second.m_displayName.empty()) {
                    return infoPair.first;
                }

                if (infoPair.second.m_displayName == "_") {
                    return common::emptyString();
                }

                return infoPair.second.m_displayName;
            };

        ++nextValue;
        auto addElementNameFunc = 
            [&names, getDisplayNameFunc](auto& infoPair) 
            {
                names.push_back('\"' + getDisplayNameFunc(infoPair) + '\"');
            };

        auto valIter = values.find(v.second);
        assert(valIter != values.end());
        if ((!obj.isNonUniqueAllowed()) ||
            (generator().schemaVersion() < valIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*valIter);
            continue;
        }

        auto allRevValues = revValues.equal_range(v.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevValues.first; iter != allRevValues.second; ++iter) {
            auto vIter = values.find(iter->second);
            assert(vIter != values.end());
            if (generator().schemaVersion() < vIter->second.m_deprecatedSince) {
                addElementNameFunc(*vIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*valIter);
    }

    std::string namesStr = common::listToString(names, ",\n", common::emptyString());

    static const std::string Templ = 
        "static const char* Map[] = {\n"
        "    #^#NAMES#$#\n"
        "};\n"
        "static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "if (MapSize <= static_cast<std::size_t>(val)) {\n"
        "    return nullptr;\n"
        "}\n\n"
        "return Map[static_cast<std::size_t>(val)];";


    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAMES", std::move(namesStr)));
    return common::processTemplate(Templ, replacements);
}

std::string EnumField::getValueNameFuncBinSearchBody() const
{
    auto obj = enumFieldDslObj();
    auto type = obj.type();

    std::string names;
    if ((type == commsdsl::EnumField::Type::Uint64) ||
        ((type == commsdsl::EnumField::Type::Uintvar) && (sizeof(std::uint64_t) <= obj.maxLength()))) {
        names = getBigUnsignedValueNameBinSearchPairs();
    }
    else {
        names = getValueNameBinSearchPairs();
    }

    static const std::string Templ = 
        "using NameInfo = std::pair<#^#ENUM_NAME#$#, const char*>;\n"
        "static const NameInfo Map[] = {\n"
        "    #^#NAMES#$#\n"
        "};\n\n"
        "auto iter = std::lower_bound(\n"
        "    std::begin(Map), std::end(Map), val,\n"
        "    [](const NameInfo& info, #^#ENUM_NAME#$# v) -> bool\n"
        "    {\n"
        "        return info.first < v;\n"
        "    });\n\n"
        "if ((iter == std::end(Map)) || (iter->first != val)) {\n"
        "    return nullptr;\n"
        "}\n\n"
        "return iter->second;";


    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAMES", std::move(names)));
    replacements.insert(std::make_pair("ENUM_NAME", getEnumType(common::nameToClassCopy(name()))));
    return common::processTemplate(Templ, replacements);
}

std::string EnumField::getValueNameBinSearchPairs() const
{
    auto obj = enumFieldDslObj();
    auto& revValues = obj.revValues();
    auto& values = obj.values();
    assert(!revValues.empty());
    bool isMessageId =
        obj.semanticType() == commsdsl::Field::SemanticType::MessageId;    

    bool firstElem = true;
    std::intmax_t lastValue = std::numeric_limits<std::intmax_t>::min();
    StringsList names;
    for (auto& v : revValues) {
        if ((!firstElem) && (lastValue == v.first)) {
            continue;
        }

        auto getDisplayNameFunc = 
            [](auto& infoPair) -> const std::string&
            {
                if (infoPair.second.m_displayName.empty()) {
                    return infoPair.first;
                }

                if (infoPair.second.m_displayName == "_") {
                    return common::emptyString();
                }

                return infoPair.second.m_displayName;
            };

        auto getValueStrFunc = 
            [this, isMessageId](const std::string& s)
            {
                if (isMessageId) {
                    return generator().mainNamespace() + "::" + common::msgIdPrefixStr() + s;
                }

                return getEnumType(common::nameToClassCopy(name())) + "::" + s;
            };

        auto addElementNameFunc = 
            [&names, &v, &firstElem, &lastValue, getDisplayNameFunc, &getValueStrFunc](auto& infoPair) 
            {
                auto str = 
                    "std::make_pair(" +
                    getValueStrFunc(v.second) +
                    ", \"" + getDisplayNameFunc(infoPair) +
                    "\")";
                names.push_back(std::move(str));
                firstElem = false;
                lastValue = v.first;
            };

        auto valIter = values.find(v.second);
        assert(valIter != values.end());
        if ((!obj.isNonUniqueAllowed()) ||
            (generator().schemaVersion() < valIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*valIter);
            continue;
        }

        auto allRevValues = revValues.equal_range(v.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevValues.first; iter != allRevValues.second; ++iter) {
            auto vIter = values.find(iter->second);
            assert(vIter != values.end());
            if (generator().schemaVersion() < vIter->second.m_deprecatedSince) {
                addElementNameFunc(*vIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*valIter);
    }

    return common::listToString(names, ",\n", common::emptyString());
}

std::string EnumField::getBigUnsignedValueNameBinSearchPairs() const
{
    auto obj = enumFieldDslObj();
    auto& revValues = obj.revValues();
    assert(!revValues.empty());

    std::vector<std::uintmax_t> valuesSeq;
    valuesSeq.reserve(revValues.size());
    for (auto& v : revValues) {
        valuesSeq.push_back(static_cast<std::uintmax_t>(v.first));
    }
    std::sort(valuesSeq.begin(), valuesSeq.end());

    auto& values = obj.values();
    bool firstElem = true;
    std::intmax_t lastValue = std::numeric_limits<std::intmax_t>::min();
    StringsList names;
    for (auto& posV : valuesSeq) {
        auto castedV = static_cast<std::intmax_t>(posV);
        if ((!firstElem) && (lastValue == castedV)) {
            continue;
        }

        auto revIter = revValues.find(castedV);
        assert(revIter != revValues.end());
        auto& v = *revIter;

        auto getDisplayNameFunc = 
            [](auto& infoPair) -> const std::string&
            {
                if (infoPair.second.m_displayName.empty()) {
                    return infoPair.first;
                }

                if (infoPair.second.m_displayName == "_") {
                    return common::emptyString();
                }

                return infoPair.second.m_displayName;
            };

        auto addElementNameFunc = 
            [this, &names, &v, &firstElem, &lastValue, getDisplayNameFunc](auto& infoPair) 
            {
                auto str = 
                    "std::make_pair(" +
                    getEnumType(common::nameToClassCopy(name())) +
                    "::" + v.second + ", \"" + getDisplayNameFunc(infoPair) +
                    "\")";
                names.push_back(std::move(str));
                firstElem = false;
                lastValue = v.first;
            };

        auto valIter = values.find(v.second);
        assert(valIter != values.end());
        if ((!obj.isNonUniqueAllowed()) ||
            (generator().schemaVersion() < valIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*valIter);
            continue;
        }

        auto allRevValues = revValues.equal_range(v.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevValues.first; iter != allRevValues.second; ++iter) {
            auto vIter = values.find(iter->second);
            assert(vIter != values.end());
            if (generator().schemaVersion() < vIter->second.m_deprecatedSince) {
                addElementNameFunc(*vIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*valIter);
    }

    return common::listToString(names, ",\n", common::emptyString());
}

bool EnumField::isDirectValueNameMapping() const
{
    auto obj = enumFieldDslObj();
    auto& revValues = obj.revValues();
    assert(!revValues.empty());
    auto firstIter = revValues.begin();
    if (firstIter->first < 0) {
        // has negative numbers
        return false;
    }


    auto lastIter = revValues.end();
    std::advance(lastIter, -1);
    auto lastVal = static_cast<std::uintmax_t>(lastIter->first);
    auto maxDirectAllowed = static_cast<std::size_t>(revValues.size() * 1.1f);
    if ((maxDirectAllowed <= lastVal) && (10 <= lastVal)) {
        // Too sparse
        return false;
    }

    return true;
}

void EnumField::checkDefaultValueOpt(StringsList& list) const
{
    auto obj = enumFieldDslObj();
    auto defaultValue = obj.defaultValue();

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.type();
    if ((type == commsdsl::EnumField::Type::Uint64) ||
        (type == commsdsl::EnumField::Type::Uintvar)) {
        auto str =
            "comms::option::DefaultBigUnsignedNumValue<" +
            common::numToString(static_cast<std::uintmax_t>(defaultValue)) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::DefaultNumValue<" +
        common::numToString(defaultValue) +
        '>';
    list.push_back(std::move(str));
}

void EnumField::checkLengthOpt(StringsList& list) const
{
    auto obj = enumFieldDslObj();
    auto type = obj.type();

    if ((type == commsdsl::EnumField::Type::Intvar) ||
        (type == commsdsl::EnumField::Type::Uintvar)) {
        auto str =
            "comms::option::VarLength<" +
            common::numToString(obj.minLength()) +
            ", " +
            common::numToString(obj.maxLength()) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    auto bitLength = obj.bitLength();
    if (bitLength != 0U) {
        list.push_back("comms::option::FixedBitLength<" + common::numToString(bitLength) + '>');
        return;
    }

    static const unsigned LengthMap[] = {
        /* Int8 */ 1,
        /* Uint8 */ 1,
        /* Int16 */ 2,
        /* Uint16 */ 2,
        /* Int32 */ 4,
        /* Uint32 */ 4,
        /* Int64 */ 8,
        /* Uint64 */ 8,
        /* Intvar */ 0,
        /* Uintvar */ 0
    };

    static const std::size_t LengthMapSize = std::extent<decltype(LengthMap)>::value;
    static_assert(LengthMapSize == static_cast<std::size_t>(commsdsl::IntField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(type);
    if (LengthMapSize <= idx) {
        return;
    }

    assert(LengthMap[idx] != 0);
    if (LengthMap[idx] != obj.minLength()) {
        auto str =
            "comms::option::FixedLength<" +
            common::numToString(obj.minLength()) +
            '>';
        list.push_back(std::move(str));
    }
}

void EnumField::checkValidRangesOpt(EnumField::StringsList& list) const
{
    auto obj = enumFieldDslObj();

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::EnumField::Type::Uint64) ||
        ((type != commsdsl::EnumField::Type::Uintvar) && (obj.maxLength() >= sizeof(std::int64_t)));

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    auto addOptFunc =
        [&list, bigUnsigned](auto& r)
        {
            std::string str = "comms::option::";
            do {
                if (!bigUnsigned) {
                    break;
                }

                bool minInRange =
                    static_cast<std::uintmax_t>(r.m_min) <= static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());

                bool maxInRange =
                    static_cast<std::uintmax_t>(r.m_max) <= static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());
                if (minInRange && maxInRange) {
                    break;
                }

                if (r.m_min == r.m_max) {
                    str += "ValidBigUnsignedNumValue<";
                    str += common::numToString(static_cast<std::uintmax_t>(r.m_min));
                    str += '>';
                }
                else {
                    str += "ValidBigUnsignedNumValueRange<";
                    str += common::numToString(static_cast<std::uintmax_t>(r.m_min));
                    str += ", ";
                    str += common::numToString(static_cast<std::uintmax_t>(r.m_max));
                    str += '>';
                }

                list.push_back(std::move(str));
                return;
            } while (false);

            if (r.m_min == r.m_max) {
                str += "ValidNumValue<";
                str += common::numToString(r.m_min);
                str += '>';
            }
            else {
                str += "ValidNumValueRange<";
                str += common::numToString(r.m_min);
                str += ", ";
                str += common::numToString(r.m_max);
                str += '>';
            }

            list.push_back(std::move(str));
        };

    assert(!m_validRanges.empty());
    if (!validCheckVersion) {
        if (MaxRangesInOpts < m_validRanges.size()) {
            return; // Will be in validity check
        }

        for (auto& range : m_validRanges) {
            addOptFunc(range);
        }
        return;
    }

    auto uncondStartIter =
        std::find_if(
            m_validRanges.begin(), m_validRanges.end(),
            [](auto& elem)
            {
                return
                    (elem.m_sinceVersion == 0U) &&
                    (elem.m_deprecatedSince == commsdsl::Protocol::notYetDeprecated());
            });

    if (uncondStartIter == m_validRanges.end()) {
        // No unconditionals found;
        list.push_back("comms::option::VersionStorage");
        list.push_back("comms::option::InvalidByDefault");
        return;
    }

    auto uncondEndIter =
        std::find_if(
            uncondStartIter + 1, m_validRanges.end(),
            [](auto& elem)
            {
                return
                    (elem.m_sinceVersion != 0U) ||
                    (elem.m_deprecatedSince != commsdsl::Protocol::notYetDeprecated());
            });

    auto uncondCount =
        static_cast<std::size_t>(std::distance(uncondStartIter, uncondEndIter));

    if (uncondCount != m_validRanges.size()) {
        list.push_back("comms::option::VersionStorage");
    }

    for (auto iter = uncondStartIter; iter != uncondEndIter; ++iter) {
        addOptFunc(*iter);
    }
}

bool EnumField::prepareRanges() const
{
    if (!m_validRanges.empty()) {
        return true;
    }

    auto obj = enumFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    auto& values = obj.values();
    m_validRanges.reserve(values.size());
    for (auto& v : values) {
        bool exists =
            generator().doesElementExist(
                v.second.m_sinceVersion,
                v.second.m_deprecatedSince,
                !validCheckVersion);
        if (!exists) {
            continue;
        }

        m_validRanges.emplace_back();
        auto& r = m_validRanges.back();
        r.m_min = v.second.m_value;
        r.m_max = r.m_min;
        r.m_sinceVersion = v.second.m_sinceVersion;
        r.m_deprecatedSince = v.second.m_deprecatedSince;
    }

    if (m_validRanges.empty()) {
        generator().logger().error("Enum \"" + name() + "\" doesn't define any value");
        return false;
    }

    if (m_validRanges.size() <= 1U) {
        return true;
    }

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::EnumField::Type::Uint64) ||
        (type == commsdsl::EnumField::Type::Uintvar);


    // Sort
    std::sort(
        m_validRanges.begin(), m_validRanges.end(),
        [bigUnsigned, validCheckVersion](auto& e1, auto& e2)
        {
            if (validCheckVersion) {
                if (e1.m_sinceVersion != e2.m_sinceVersion) {
                    return e1.m_sinceVersion < e2.m_sinceVersion;
                }

                if (e1.m_deprecatedSince != e2.m_deprecatedSince) {
                    return e1.m_deprecatedSince < e2.m_deprecatedSince;
                }
            }

            if (bigUnsigned) {
                return static_cast<std::uintmax_t>(e1.m_min) < (static_cast<std::uintmax_t>(e2.m_min));
            }

            return e1.m_min < e2.m_min;
        });

    // Merge
    for (auto iter = m_validRanges.begin(); iter != m_validRanges.end(); ++iter) {
        if (iter->m_deprecatedSince == 0U) {
            continue;
        }

        for (auto nextIter = iter + 1; nextIter != m_validRanges.end(); ++nextIter) {
            if (nextIter->m_deprecatedSince == 0U) {
                continue;
            }

            if (validCheckVersion) {
                if ((iter->m_sinceVersion != nextIter->m_sinceVersion) ||
                    (iter->m_deprecatedSince != nextIter->m_deprecatedSince)) {
                    break;
                }
            }

            if ((iter->m_max + 1) < nextIter->m_min) {
                break;
            }

            assert(iter->m_min <= nextIter->m_min);
            nextIter->m_deprecatedSince = 0U; // invalidate next range
            iter->m_max = std::max(iter->m_max, nextIter->m_max);
        }
    }

    // Remove invalid
    m_validRanges.erase(
        std::remove_if(
            m_validRanges.begin(), m_validRanges.end(),
            [](auto& elem)
            {
                return elem.m_deprecatedSince == 0U;
            }),
        m_validRanges.end());

    return true;
}


} // namespace commsdsl2comms
