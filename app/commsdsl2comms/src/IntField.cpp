//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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
    "    #^#PUBLIC#$#\n"
    "    #^#SPECIALS#$#\n"
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
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::IntValue<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_TYPE#$#\n"
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
        hasNoValue("SPECIALS") &&
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
    static_assert(TypeMapSize == static_cast<decltype(TypeMapSize)>(commsdsl::IntField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(value);
    if (TypeMapSize <= idx) {
        assert(!"Should not happen");
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


void IntField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/IntValue.h",
        "<cstdint>"
    };

    common::mergeIncludes(List, includes);
}

std::string IntField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("SPECIALS", getSpecials()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));

    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["FIELD_TYPE"] += ',';
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
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
        assert(!"Should not happen");
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

    if (props.empty()) {
        return common::emptyString();
    }

    return common::listToString(props, "\n", common::emptyString());
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
        assert(!"Should not happen");
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

std::string IntField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    checkDefaultValueOpt(options);
    checkLengthOpt(options);
    checkSerOffsetOpt(options);
    checkScalingOpt(options);
    checkUnitsOpt(options);
    checkValidRangesOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string IntField::getSpecials() const
{
    auto obj = intFieldDslObj();
    auto& specials = obj.specialValues();
    std::string result;
    for (auto& s : specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        if (!result.empty()) {
            result += '\n';
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr typename Base::ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return static_cast<typename Base::ValueType>(#^#SPEC_VAL#$#);\n"
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
            conds.push_back("(static_cast<typename Base::ValueType>(" + minVal + ") == Base::value())");
        }
        else {
            conds.push_back("(static_cast<typename Base::ValueType>(" + minVal + ") <= Base::value())");
            conds.push_back("(Base::value() <= static_cast<typename Base::ValueType>(" + maxVal + "))");
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("COND", common::listToString(conds, " &&\n", common::emptyString())));
        rangesChecks += common::processTemplate(RangeTempl, replacements);
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("RANGES_CHECKS", std::move(rangesChecks)));
    return common::processTemplate(Templ, replacements);
}

void IntField::checkDefaultValueOpt(StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto defaultValue = obj.defaultValue();
    if ((defaultValue == 0) &&
        (semanticType() == commsdsl::Field::SemanticType::Version)) {
        std::string opt = "comms::option::DefaultNumValue<";
        opt += common::numToString(generator().schemaVersion());
        opt += '>';
        list.push_back(std::move(opt));
        return;
    }

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.type();
    if ((type == commsdsl::IntField::Type::Uint64) ||
        (type == commsdsl::IntField::Type::Uintvar)) {
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

void IntField::checkLengthOpt(StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto type = obj.type();
    if ((type == commsdsl::IntField::Type::Intvar) ||
        (type == commsdsl::IntField::Type::Uintvar)) {
        auto str =
            "comms::option::VarLength<" +
            common::numToString(obj.minLength()) +
            ", " +
            common::numToString(obj.maxLength()) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    if (obj.bitLength() != 0U) {
        auto str =
            "comms::option::FixedBitLength<" +
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
    static_assert(LengthMapSize == static_cast<decltype(LengthMapSize)>(commsdsl::IntField::Type::NumOfValues),
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
            "comms::option::FixedLength<" +
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
        "comms::option::NumValueSerOffset<" +
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
        assert(!"Should not happen");
        return;
    }

    auto str =
        "comms::option::ScalingRatio<" +
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
        list.push_back("comms::option::VersionStorage");

        if (!addedRangeOpt) {
            list.push_back("comms::option::InvalidByDefault");
        }
    }
}

bool IntField::isUnsigned() const
{
    auto obj = intFieldDslObj();
    auto type = obj.type();
    return isUnsignedType(type);
}

} // namespace commsdsl2comms
