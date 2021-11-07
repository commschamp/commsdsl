//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "IntField.h"

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
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::IntValue<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::IntValue<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#FIELD_TYPE#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    /// @brief Re-definition of the value type.\n"
    "    using ValueType = typename Base::ValueType;\n\n"
    "    #^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
    "    #^#PUBLIC#$#\n"
    "    #^#HAS_SPECIALS#$#\n"
    "    #^#SPECIALS#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "    #^#SPECIAL_NAMES_MAP#$#\n"
    "    #^#DISPLAY_DECIMALS#$#\n"
    "#^#PROTECTED#$#\n"
    "#^#PRIVATE#$#\n"
    "};\n"
);

} // namespace

const std::string& IntField::convertType(commsdsl::IntField::Type value, std::size_t len)
{
    static const std::string TypeMap[] = {
        /* Int8 */ "std::int8_t",
        /* Uint8 */ "std::uint8_t",
        /* Int16 */ "std::int16_t",
        /* Uint16 */ "std::uint16_t",
        /* Int32 */ "std::int32_t",
        /* Uint32 */ "std::uint32_t",
        /* Int64 */ "std::int64_t",
        /* Uint64 */ "std::uint64_t",
        /* Intvar */ common::emptyString(),
        /* Uintvar */ common::emptyString()
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<std::size_t>(commsdsl::IntField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(value);
    if (TypeMapSize <= idx) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return common::emptyString();
    }

    auto& typeStr = TypeMap[idx];
    if (!typeStr.empty()) {
        return typeStr;
    }

    // Variable length
    auto offset = idx - static_cast<decltype(idx)>(commsdsl::IntField::Type::Intvar);
    assert(offset < 2U);

    if (len <= 2U) {
        auto base = static_cast<decltype(idx)>(commsdsl::IntField::Type::Int16);
        return TypeMap[base + offset];
    }

    if (len <= 4U) {
        auto base = static_cast<decltype(idx)>(commsdsl::IntField::Type::Int32);
        return TypeMap[base + offset];
    }

    auto base = static_cast<decltype(idx)>(commsdsl::IntField::Type::Int64);
    return TypeMap[base + offset];
}

bool IntField::isUnsignedType(commsdsl::IntField::Type value)
{
    static const commsdsl::IntField::Type Map[] = {
        commsdsl::IntField::Type::Uint8,
        commsdsl::IntField::Type::Uint16,
        commsdsl::IntField::Type::Uint32,
        commsdsl::IntField::Type::Uint64,
        commsdsl::IntField::Type::Uintvar,
    };

    auto iter = std::find(std::begin(Map), std::end(Map), value);
    return iter != std::end(Map);
}

bool IntField::isUnsignedType() const
{
    return isUnsignedType(intFieldDslObj().type());
}

bool IntField::isValidPropKey() const
{
    auto obj = intFieldDslObj();
    if (!obj.isFailOnInvalid()) {
        return false;
    }

    if (obj.isPseudo()) {
        return false;
    }

    auto& validRanges = obj.validRanges();
    if (validRanges.size() != 1U) {
        return false;
    }

    auto& r = validRanges.front();
    if (r.m_min != r.m_max) {
        return false;
    }

    if (r.m_min != obj.defaultValue()) {
        return false;
    }

    return true;
}

std::string IntField::getPropKeyType() const
{
    assert(isValidPropKey());

    if ((!getCustomRead().empty()) ||
        (!getCustomRefresh().empty()) ||
        (isCustomizable())) {
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(common::emptyString(), true)));

    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["FIELD_TYPE"] += ',';
    }

    static const std::string Templ =
        "comms::field::IntValue<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    #^#FIELD_TYPE#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";
    return common::processTemplate(Templ, replacements);
}

std::string IntField::getPropKeyValueStr(bool asHex) const
{
    auto val = intFieldDslObj().defaultValue();
    if (isUnsigned()) {
        unsigned hexWidth = 0U;
        if (asHex) {
            hexWidth = static_cast<decltype(hexWidth)>(intFieldDslObj().maxLength() * 2U);
        }
        return common::numToString(static_cast<std::uintmax_t>(val), hexWidth);
    }
    return common::numToString(val);
}

bool IntField::prepareImpl()
{
    auto obj = intFieldDslObj();
    auto& specials = obj.specialValues();
    m_specials.assign(specials.begin(), specials.end());
    bool compareUnsigned = isUnsignedType();
    std::sort(
        m_specials.begin(), m_specials.end(),
        [compareUnsigned](auto& elem1, auto& elem2)
        {
            if (elem1.second.m_value == elem2.second.m_value) {
                return elem1.first < elem2.first;
            }

            if (compareUnsigned) {
                return static_cast<std::uintmax_t>(elem1.second.m_value) < static_cast<std::uintmax_t>(elem2.second.m_value);
            }

            return elem1.second.m_value < elem2.second.m_value;
        });
    return true;
}

void IntField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/IntValue.h",
        "<cstdint>"
    };

    common::mergeIncludes(List, includes);
}

void IntField::updateIncludesCommonImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "<cstdint>",
        "<type_traits>",
        "<utility>"
    };

    common::mergeIncludes(List, includes);
}

std::string IntField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    auto adjScope = adjustScopeWithNamespace(scope);
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameCommonWrapFunc(adjScope)));
    replacements.insert(std::make_pair("HAS_SPECIALS", getHasSpecialsFunc(adjScope)));
    replacements.insert(std::make_pair("SPECIALS", getSpecials(adjScope)));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getRefresh()));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("DISPLAY_DECIMALS", getDisplayDecimals()));

    if (!m_specials.empty()) {
        replacements.insert(std::make_pair("SPECIAL_VALUE_NAMES_MAP_DEFS", getSpecialNamesMapDefs(adjScope)));
        replacements.insert(std::make_pair("SPECIAL_NAMES_MAP", getSpacialNamesMapFunc(adjScope)));
    }

    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["FIELD_TYPE"] += ',';
    }

    return common::processTemplate(ClassTemplate, replacements);
}

std::string IntField::getCompareToValueImpl(
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
    auto compareValFunc =
        [&op, &usedName, versionOptional](auto v)
        {
            if (versionOptional) {
                return
                    "field_" + usedName + "().doesExist() &&\n"
                    "(field_" + usedName + "().field().value() " +
                    op + " " + common::numToString(v) + ')';
            }

            return
                "field_" + usedName + "().value() " +
                op + " " + common::numToString(v);
        };

    try {
        if (isUnsigned()) {
            auto val = static_cast<std::uintmax_t>(std::stoull(value));
            return compareValFunc(val);
        }

        auto val = static_cast<std::intmax_t>(std::stoll(value));
        return compareValFunc(val);
    }
    catch (...) {
        generator().logger().error("Unexpected numeric value: " + value);
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return common::emptyString();
    }
}

std::string IntField::getCompareToFieldImpl(
    const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    auto strGenFunc =
        [this, &op, &field, &usedName, forcedVersionOptional](const std::string& type)
        {
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
                otherFieldValue = "field_" + fieldName + "().field().value()";
            }
            else {
                otherFieldValue = "field_" + fieldName + "().value()";
            }

            auto compareExpr = thisFieldValue + op + " static_cast<" + type + ">(" + otherFieldValue + ')';

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
        };

    if (field.kind() != kind()) {
        return strGenFunc(getFieldType());
    }

    const IntField& otherField = static_cast<const IntField&>(field);
    if (isUnsigned() == otherField.isUnsigned()) {
        return Base::getCompareToFieldImpl(op, field, usedName, forcedVersionOptional);
    }

    return strGenFunc(otherField.getFieldChangedSignType());
}

std::string IntField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    common::StringsList props;
    static_cast<void>(serHiddenParam);
    auto obj = intFieldDslObj();
    auto decimals = obj.displayDecimals();
    auto offset = obj.displayOffset();
    if (decimals != 0U) {
        props.push_back(".scaledDecimals(" + common::numToString(decimals) + ')');
    }

    if (offset != 0) {
        props.push_back(".displayOffset(" + common::numToString(offset) + ')');
    }

    if (!m_specials.empty() && (obj.displaySpecials())) {
        auto type = obj.type();
        bool bigUnsigned =
            (type == commsdsl::IntField::Type::Uint64) ||
            (type == commsdsl::IntField::Type::Uintvar);

        auto addSpecDisplayNameFunc =
            [&props, bigUnsigned](std::intmax_t val, const std::string& name, const std::string& displayName)
            {
                auto valStr = common::numToString(val);
                if (bigUnsigned) {
                    valStr = "static_cast<long long>(" + common::numToString(static_cast<std::uintmax_t>(val)) + ")";
                }

                auto* nameToAdd = &displayName;
                if (nameToAdd->empty()) {
                    nameToAdd = &name;
                }

                props.push_back(".addSpecial(\"" + *nameToAdd + "\", " + valStr + ")");
            };

        for (auto& s : m_specials) {
            addSpecDisplayNameFunc(s.second.m_value, s.first, s.second.m_displayName);
        }
    }

    if (props.empty()) {
        return common::emptyString();
    }

    return common::listToString(props, "\n", common::emptyString());
}

std::string IntField::getCommonDefinitionImpl(const std::string& fullScope) const
{
    auto obj = intFieldDslObj();
    common::StringsList specialsList;
    for (auto& s : m_specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string SpecialTempl(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return static_cast<ValueType>(#^#SPEC_VAL#$#);\n"
            "}\n"
        );

        std::string specVal;
        auto type = obj.type();
        if ((type == commsdsl::IntField::Type::Uint64) ||
            (type == commsdsl::IntField::Type::Uintvar)) {
            specVal = common::numToString(static_cast<std::uintmax_t>(s.second.m_value));
        }
        else {
            specVal = common::numToString(s.second.m_value);
        }

        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = common::makeMultilineCopy(desc);
            ba::replace_all(desc, "\n", "\n///     ");
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("SPEC_NAME", s.first));
        replacements.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        replacements.insert(std::make_pair("SPEC_VAL", std::move(specVal)));
        replacements.insert(std::make_pair("SPECIAL_DOC", std::move(desc)));

        specialsList.push_back(common::processTemplate(SpecialTempl, replacements));
    }

    static const std::string Templ =
        "/// @brief Common types and functions for\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "struct #^#NAME#$#Common\n"
        "{\n"
        "    /// @brief Re-definition of the value type used by\n"
        "    ///     #^#SCOPE#$# field.\n"
        "    using ValueType = #^#VALUE_TYPE#$#;\n\n"
        "    #^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
        "    #^#NAME_FUNC#$#\n"
        "    #^#HAS_SPECIAL_FUNC#$#\n"
        "    #^#SPECIALS#$#\n"
        "    #^#SPECIAL_NAMES_MAP#$#\n"
        "};\n";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("NAME", common::nameToClassCopy(name())));
    repl.insert(std::make_pair("SCOPE", fullScope));
    repl.insert(std::make_pair("VALUE_TYPE", getFieldType()));
    repl.insert(std::make_pair("NAME_FUNC", getCommonNameFunc(fullScope)));
    repl.insert(std::make_pair("HAS_SPECIAL_FUNC", getHasSpecialsFunc()));
    if (!specialsList.empty()) {
        repl.insert(std::make_pair("SPECIAL_VALUE_NAMES_MAP_DEFS", getSpecialNamesMapDefs()));
        repl.insert(std::make_pair("SPECIALS", common::listToString(specialsList, "\n", "\n")));
        repl.insert(std::make_pair("SPECIAL_NAMES_MAP", getSpacialNamesMapFunc()));
    }
    return common::processTemplate(Templ, repl);
}

std::string IntField::getFieldBaseParams() const
{
    auto obj = intFieldDslObj();
    auto endian = obj.endian();
    return getCommonFieldBaseParams(endian);
}

const std::string& IntField::getFieldType() const
{
    auto obj = intFieldDslObj();
    return convertType(obj.type(), obj.maxLength());
}

std::string IntField::getFieldChangedSignType() const
{
    auto str = getFieldType();
    assert(str.find("std::") == 0U);
    if (str.size() < 6) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return str;
    }

    if (str[5] == 'u') {
        str.erase(str.begin() + 5);
    }
    else {
        str.insert(str.begin() + 5, 'u');
    }

    return str;
}

std::string IntField::getFieldOpts(const std::string& scope, bool reduced) const
{
    StringsList options;

    updateExtraOptions(scope, options, reduced);

    checkLengthOpt(options);
    checkSerOffsetOpt(options);
    checkScalingOpt(options);
    checkUnitsOpt(options);
    checkRefreshOpt(options);

    if (!reduced) {
        checkDefaultValueOpt(options);
        checkValidRangesOpt(options);
    }

    return common::listToString(options, ",\n", common::emptyString());
}

std::string IntField::getSpecials(const std::string& scope) const
{
    std::string result;
    for (auto& s : m_specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        if (!result.empty()) {
            result += '\n';
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return #^#SCOPE#$##^#CLASS_NAME#$#Common::value#^#SPEC_ACC#$#();\n"
            "}\n\n"
            "/// @brief Check the value is equal to special @ref value#^#SPEC_ACC#$#().\n"
            "bool is#^#SPEC_ACC#$#() const\n"
            "{\n"
            "    return Base::value() == value#^#SPEC_ACC#$#();\n"
            "}\n\n"
            "/// @brief Assign special value @ref value#^#SPEC_ACC#$#() to the field.\n"
            "void set#^#SPEC_ACC#$#()\n"
            "{\n"
            "    Base::value() = value#^#SPEC_ACC#$#();\n"
            "}\n"
        );

        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = common::makeMultilineCopy(desc);
            ba::replace_all(desc, "\n", "\n///     ");
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("SPEC_NAME", s.first));
        replacements.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
        replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
        replacements.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        replacements.insert(std::make_pair("SPECIAL_DOC", std::move(desc)));

        result += common::processTemplate(Templ, replacements);
    }
    return result;
}

std::string IntField::getValid() const
{
    auto custom = getCustomValid();
    if (!custom.empty()) {
        return custom;
    }

    auto obj = intFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        return common::emptyString();
    }

    auto validRanges = obj.validRanges(); // copy
    validRanges.erase(
        std::remove_if(
            validRanges.begin(), validRanges.end(),
            [this](auto& r)
            {
                return !this->generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince);
            }),
        validRanges.end());

    if (validRanges.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Custom validity check.\n"
        "bool valid() const\n"
        "{\n"
        "    if (Base::valid()) {\n"
        "        return true;\n"
        "    }\n\n"
        "    #^#RANGES_CHECKS#$#\n"
        "    return false;\n"
        "}\n";

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::IntField::Type::Uint64) ||
        (type == commsdsl::IntField::Type::Uintvar);

    std::string rangesChecks;
    for (auto& r : validRanges) {
        if (!rangesChecks.empty()) {
            rangesChecks += '\n';
        }

        static const std::string RangeTempl =
            "if (#^#COND#$#) {\n"
            "    return true;\n"
            "}\n";

        std::string minVal;
        std::string maxVal;

        if (bigUnsigned) {
            minVal = common::numToString(static_cast<std::uintmax_t>(r.m_min));
            maxVal = common::numToString(static_cast<std::uintmax_t>(r.m_max));
        }
        else {
            minVal = common::numToString(r.m_min);
            maxVal = common::numToString(r.m_max);
        }

        common::StringsList conds;
        if (0U < r.m_sinceVersion) {
            conds.push_back('(' + common::numToString(r.m_sinceVersion) + " <= Base::getVersion())");
        }

        if (r.m_deprecatedSince < commsdsl::Protocol::notYetDeprecated()) {
            conds.push_back("(Base::getVersion() < " + common::numToString(r.m_deprecatedSince) + ")");
        }

        if (r.m_min == r.m_max) {
            conds.push_back("(static_cast<ValueType>(" + minVal + ") == Base::value())");
        }
        else {
            conds.push_back("(static_cast<ValueType>(" + minVal + ") <= Base::value())");
            conds.push_back("(Base::value() <= static_cast<ValueType>(" + maxVal + "))");
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("COND", common::listToString(conds, " &&\n", common::emptyString())));
        rangesChecks += common::processTemplate(RangeTempl, replacements);
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("RANGES_CHECKS", std::move(rangesChecks)));
    return common::processTemplate(Templ, replacements);
}

std::string IntField::getRefresh() const
{
    auto custom = getCustomRefresh();
    if (!custom.empty()) {
        return custom;
    }

    if (!requiresFailOnInvalidRefresh()) {
        return std::string();
    }

    auto obj = intFieldDslObj();
    auto& validRanges = obj.validRanges();

    static const std::string Templ = 
        "/// @brief Refresh functionality.\n"
        "/// @details Make sure the value is valid.\n"
        "bool refresh()\n"
        "{\n"
        "    bool updated = Base::refresh();\n"
        "    if (Base::valid()) {\n"
        "        return updated;\n"
        "    };\n"
        "    Base::value() = static_cast<ValueType>(#^#VALID_VALUE#$#);\n"
        "    return true;\n"
        "}";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("VALID_VALUE", common::numToString(validRanges.front().m_min)));
    return common::processTemplate(Templ, repl);
}

std::string IntField::getDisplayDecimals() const
{
    auto obj = intFieldDslObj();
    auto scaling = obj.scaling();
    std::string result;
    if (scaling.first == scaling.second) {
        return result;
    }

    static const std::string Templ = 
        "/// @brief Requested number of digits after decimal point when value\n"
        "///     is displayed.\n"
        "static constexpr unsigned displayDecimals()\n"
        "{\n"
        "    return #^#DISPLAY_DECIMALS#$#;\n"
        "}";
        
    common::ReplacementMap repl;
    repl.insert(std::make_pair("DISPLAY_DECIMALS", std::to_string(obj.displayDecimals())));
    return common::processTemplate(Templ, repl);
}

void IntField::checkDefaultValueOpt(StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto defaultValue = obj.defaultValue();
    if ((defaultValue == 0) &&
        (semanticType() == commsdsl::Field::SemanticType::Version)) {
        std::string opt = "comms::option::def::DefaultNumValue<";
        opt += common::numToString(generator().schemaVersion());
        opt += '>';
        list.push_back(std::move(opt));
        return;
    }

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.type();
    if ((defaultValue < 0) &&
        ((type == commsdsl::IntField::Type::Uint64) || (type == commsdsl::IntField::Type::Uintvar))) {
        auto str =
            "comms::option::def::DefaultBigUnsignedNumValue<" +
            common::numToString(static_cast<std::uintmax_t>(defaultValue)) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::def::DefaultNumValue<" +
        common::numToString(defaultValue) +
        '>';
    list.push_back(std::move(str));
}

void IntField::checkLengthOpt(StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto type = obj.type();
    if ((type == commsdsl::IntField::Type::Intvar) ||
        (type == commsdsl::IntField::Type::Uintvar)) {
        auto str =
            "comms::option::def::VarLength<" +
            common::numToString(obj.minLength()) +
            ", " +
            common::numToString(obj.maxLength()) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    if (obj.bitLength() != 0U) {
        auto str =
            "comms::option::def::FixedBitLength<" +
            common::numToString(obj.bitLength()) +
            '>';
        list.push_back(std::move(str));
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
        std::string secondParam;
        if (!obj.signExt()) {
            secondParam = ", false";
        }
        auto str =
            "comms::option::def::FixedLength<" +
            common::numToString(obj.minLength()) + secondParam +
            '>';
        list.push_back(std::move(str));
    }
}

void IntField::checkSerOffsetOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto serOffset = obj.serOffset();
    if (serOffset == 0) {
        return;
    }

    auto str =
        "comms::option::def::NumValueSerOffset<" +
        common::numToString(serOffset) +
        '>';
    list.push_back(std::move(str));
}

void IntField::checkScalingOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto scaling = obj.scaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return;
    }

    if ((num == 0) || (denom == 0)) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return;
    }

    auto str =
        "comms::option::def::ScalingRatio<" +
        common::numToString(num) +
        ", " +
        common::numToString(denom) +
        '>';
    list.push_back(std::move(str));
}

void IntField::checkUnitsOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto units = obj.units();
    auto& str = common::dslUnitsToOpt(units);
    if (!str.empty()) {
        list.push_back(str);
    }
}

void IntField::checkValidRangesOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto validRanges = obj.validRanges(); // copy
    if (validRanges.empty()) {
        return;
    }

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::IntField::Type::Uint64) ||
        ((type != commsdsl::IntField::Type::Uintvar) && (obj.maxLength() >= sizeof(std::int64_t)));

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        // unify
        std::size_t idx = 0U;
        while (idx < (validRanges.size() - 1U)) {
            auto& thisRange = validRanges[idx];
            auto& nextRange = validRanges[idx + 1];


            auto needToMergeCheck =
                [](auto min1, auto max1, auto min2, auto max2) -> bool
                {
                    static_cast<void>(min1);
                    static_cast<void>(max2);
                    assert(min1 <= min2);
                    if (min2 <= (max1 + 1)) {
                        assert(max1 <= max2);
                        return true;
                    }

                    return false;
                };

            bool merge = false;
            if (bigUnsigned) {
                merge =
                    needToMergeCheck(
                        static_cast<std::uintmax_t>(thisRange.m_min),
                        static_cast<std::uintmax_t>(thisRange.m_max),
                        static_cast<std::uintmax_t>(nextRange.m_min),
                        static_cast<std::uintmax_t>(nextRange.m_max));
            }
            else {
                merge = needToMergeCheck(thisRange.m_min, thisRange.m_max, nextRange.m_min, nextRange.m_max);
            }

            if (!merge) {
                ++idx;
                continue;
            }

            auto needToUpdateCheck =
                [](auto max1, auto max2) -> bool
                {
                    return max1 < max2;
                };

            bool update = false;
            if (bigUnsigned) {
                update =
                    needToUpdateCheck(
                        static_cast<std::uintmax_t>(thisRange.m_max),
                        static_cast<std::uintmax_t>(nextRange.m_max));
            }
            else {
                update = needToUpdateCheck(thisRange.m_max, nextRange.m_max);
            }

            if (update) {
                thisRange.m_max = nextRange.m_max;
            }

            validRanges.erase(validRanges.begin() + idx + 1);
        }
    }

    bool versionStorageRequired = false;
    bool addedRangeOpt = false;
    for (auto& r : validRanges) {
        if (!generator().doesElementExist(r.m_sinceVersion, r.m_deprecatedSince, !validCheckVersion)) {
            continue;
        }

        if (validCheckVersion && (generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince))) {
            versionStorageRequired = true;
            continue;
        }

        bool big = false;
        std::string str = "comms::option::def::";
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
            big = true;
            addedRangeOpt = true;
        } while (false);

        if (big) {
            continue;
        }

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
        addedRangeOpt = true;
    }

    if (versionStorageRequired) {
        list.push_back("comms::option::def::VersionStorage");

        if (!addedRangeOpt) {
            list.push_back("comms::option::def::InvalidByDefault");
        }
    }
}

void IntField::checkRefreshOpt(StringsList& list) const
{
    if (requiresFailOnInvalidRefresh()) {
        list.push_back("comms::option::def::HasCustomRefresh");
    }
}

bool IntField::requiresFailOnInvalidRefresh() const
{
    if (!dslObj().isFailOnInvalid()) {
        return false;
    }

    auto obj = intFieldDslObj();
    auto& validRanges = obj.validRanges();
    if (validRanges.empty()) {
        return false;
    }

    return true;
}

bool IntField::isUnsigned() const
{
    auto obj = intFieldDslObj();
    auto type = obj.type();
    return isUnsignedType(type);
}

std::string IntField::getSpecialNamesMapDefs(const std::string& scope) const
{
    static const std::string Templ = 
        "/// @brief Single special value name info entry.\n"
        "using SpecialNameInfo = #^#INFO_DEF#$#;\n\n"
        "/// @brief Type returned from @ref specialNamesMap() member function.\n"
        "/// @details The @b first value of the pair is pointer to the map array,\n"
        "///     The @b second value of the pair is the size of the array.\n"
        "using SpecialNamesMapInfo = #^#MAP_DEF#$#;\n";

    common::ReplacementMap repl;
    if (scope.empty()) {
        repl.insert(std::make_pair("INFO_DEF", getCommonSpecialNameInfoDef()));
        repl.insert(std::make_pair("MAP_DEF", getCommonSpecialNamesMapDef()));
    }
    else {
        repl.insert(std::make_pair("INFO_DEF", getSpecialNameInfoDef(scope)));
        repl.insert(std::make_pair("MAP_DEF", getSpecialNamesMapDef(scope)));
    }
    return common::processTemplate(Templ, repl);
}

const std::string& IntField::getCommonSpecialNameInfoDef()
{
    static const std::string Str = "std::pair<ValueType, const char*>";
    return Str;
}

const std::string& IntField::getCommonSpecialNamesMapDef()
{
    static const std::string Str = "std::pair<const SpecialNameInfo*, std::size_t>";
    return Str;
}

std::string IntField::getSpecialNameInfoDef(const std::string& scope) const
{
    static const std::string Temp = 
        "#^#SCOPE#$##^#CLASS_NAME#$#Common::SpecialNameInfo";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl);   
}

std::string IntField::getSpecialNamesMapDef(const std::string& scope) const
{
    static const std::string Temp = 
        "#^#SCOPE#$##^#CLASS_NAME#$#Common::SpecialNamesMapInfo";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl);    
}

std::string IntField::getSpacialNamesMapFunc(const std::string& scope) const
{
    static const std::string Templ = 
        "/// @brief Retrieve map of special value names\n"
        "static SpecialNamesMapInfo specialNamesMap()\n"
        "{\n"
        "    #^#BODY#$#;\n"
        "}\n";

    std::string body;
    if (scope.empty()) {
        body = getSpacialNamesMapFuncCommonBody();
    }
    else {
        body = getSpacialNamesMapFuncBody(scope);
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("BODY", std::move(body)));
    return common::processTemplate(Templ, replacements);
}

std::string IntField::getSpacialNamesMapFuncCommonBody() const
{
    common::StringsList specialInfos;
    for (auto& s : m_specials) {
        static const std::string SpecTempl = 
            "std::make_pair(value#^#SPEC_ACC#$#(), \"#^#SPEC_NAME#$#\")";

        common::ReplacementMap specRepl;
        specRepl.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        specRepl.insert(std::make_pair("SPEC_NAME", s.first));
        specialInfos.push_back(common::processTemplate(SpecTempl, specRepl));
    }

    assert(!specialInfos.empty());
    static const std::string Templ = 
        "static const SpecialNameInfo Map[] = {\n"
        "    #^#INFOS#$#\n"
        "};\n"
        "static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "return std::make_pair(&Map[0], MapSize);\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("INFOS", common::listToString(specialInfos, ",\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string IntField::getSpacialNamesMapFuncBody(const std::string& scope) const
{
    static const std::string Temp = 
        "return #^#SCOPE#$##^#CLASS_NAME#$#Common::specialNamesMap();";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl); 
}

std::string IntField::getHasSpecialsFunc(const std::string& scope) const
{
    static const std::string Temp = 
        "/// @brief Compile time detection of special values presence.\n"
        "static constexpr bool hasSpecials()\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    std::string body;
    if (scope.empty()) {
        body = getHasSpecialsFuncCommonBody();
    }
    else {
        body = getHasSpecialsFuncBody(scope);
    }

    common::ReplacementMap repl;
    repl.insert(std::make_pair("BODY", std::move(body)));
    return common::processTemplate(Temp, repl);
}

std::string IntField::getHasSpecialsFuncCommonBody() const
{
    static const std::string Temp = 
        "return #^#RESULT#$#;";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("RESULT", m_specials.empty() ? "false" : "true"));
    return common::processTemplate(Temp, repl);
}

std::string IntField::getHasSpecialsFuncBody(const std::string& scope) const
{
    static const std::string Temp = 
        "return #^#SCOPE#$##^#CLASS_NAME#$#Common::hasSpecials();";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl);   
}

} // namespace commsdsl2comms
