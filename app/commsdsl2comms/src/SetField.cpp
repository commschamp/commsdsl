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

#include "SetField.h"

#include <type_traits>
#include <algorithm>
#include <iterator>

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
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::BitmaskValue<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::BitmaskValue<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#BITS_ACCESS#$#\n"
    "    #^#PUBLIC#$#\n"
    "    #^#NAME#$#\n"
    "    #^#BIT_NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "#^#PROTECTED#$#\n"
    "#^#PRIVATE#$#\n"
    "};\n"
);

const auto MaxBits = std::numeric_limits<std::uintmax_t>::digits;

} // namespace

void SetField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/BitmaskValue.h",
        "<type_traits>"
    };

    common::mergeIncludes(List, includes);
}

std::string SetField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className, false, getExtraDoc())));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("BITS_ACCESS", getBitsAccess()));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("BIT_NAME", getBitNameWrap(scope)));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));

    return common::processTemplate(ClassTemplate, replacements);
}

std::string SetField::getCompareToValueImpl(
    const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    auto obj = setFieldDslObj();
    auto& bits = obj.bits();
    auto iter = bits.find(value);
    if (iter == bits.end()) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    assert(op.empty() || (op == "!"));
    bool versionOptional = forcedVersionOptional || isVersionOptional();
    if (!versionOptional) {
        return op + "field_" + usedName + "().getBitValue_" + value + "()";
    }

    return
        "(field_" + usedName + "().doesExist()) &&\n"
        "(" + op + "field_" + usedName + "().field().getBitValue_" + value + "())";
}

std::string SetField::getCompareToFieldImpl(
    const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    assert(!"Should not be called");
    return Base::getCompareToFieldImpl(op, field, nameOverride, forcedVersionOptional);
}

std::string SetField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    static_cast<void>(serHiddenParam);
    StringsList props;
    auto obj = setFieldDslObj();
    auto& bits = obj.bits();
    auto& revBits = obj.revBits();
    props.reserve(bits.size());
    unsigned prevBitIdx = std::numeric_limits<unsigned>::max();
    for (auto& rBit : revBits) {
        if (prevBitIdx == rBit.first) {
            continue;
        }

        auto iter = bits.find(rBit.second);
        assert(iter != bits.end());
        auto& b = *iter;

        if (!generator().doesElementExist(b.second.m_sinceVersion, b.second.m_deprecatedSince, true)) {
            continue;
        }

        prevBitIdx = rBit.first;

        auto* bitName = &b.second.m_displayName;
        if (bitName->empty()) {
            bitName = &b.first;
        }
        props.push_back(".add(" + common::numToString(rBit.first) + ", \"" + *bitName + "\")");
    }
    return common::listToString(props, "\n", common::emptyString());
}

std::string SetField::getCommonPreDefinitionImpl(const std::string& scope) const
{
    static const std::string Templ =
        "/// @brief Common functions for\n"
        "///     @ref #^#SCOPE#$##^#CLASS_NAME#$# field.\n"
        "struct #^#CLASS_NAME#$#Common\n"
        "{\n"
        "    #^#BIT_NAME_FUNC#$#\n"
        "};\n";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", adjustScopeWithNamespace(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    repl.insert(std::make_pair("BIT_NAME_FUNC", getBitName()));

    return common::processTemplate(Templ, repl);
}

std::string SetField::getExtraDoc() const
{
    common::StringsList extraDocList;
    auto obj = setFieldDslObj();
    auto& bits = obj.bits();
    for (auto& b : bits) {
        if (b.second.m_description.empty()) {
            continue;
        }

        std::string str =
            "@li @b " + b.first + " - " + b.second.m_description;
        str = common::makeMultilineCopy(str);
        ba::replace_all(str, "\n", "\n" + common::indentStr());
        extraDocList.push_back(std::move(str));
    }

    if (extraDocList.empty()) {
        return common::emptyString();
    }

    static const std::string Prefix =
        "The documented bits are:\n";

    return Prefix + common::listToString(extraDocList, "\n", common::emptyString());
}

std::string SetField::getFieldBaseParams() const
{
    auto obj = setFieldDslObj();
    auto endian = obj.endian();
    return getCommonFieldBaseParams(endian);
}

std::string SetField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    checkLengthOpt(options);
    checkDefaultValueOpt(options);
    checkReservedBitsOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string SetField::getBitsAccess() const
{
    auto obj = setFieldDslObj();
    std::uintmax_t usedBits = 0U;
    common::StringsList names;
    for (auto& bitInfo : obj.revBits()) {
        auto idx = bitInfo.first;
        if (MaxBits <= idx) {
            assert(!"Should not happen");
            continue;
        }

        auto mask = static_cast<std::uintmax_t>(1) << idx;
        usedBits |= mask;
        names.push_back(bitInfo.second);
    }

    if (obj.isUnique() && (((usedBits + 1) & usedBits) == 0U)) {
        // sequential
        static const std::string Templ =
            "/// @brief Provides names and generates access functions for internal bits.\n"
            "/// @details See definition of @b COMMS_BITMASK_BITS_SEQ macro\n"
            "///     related to @b comms::field::BitmaskValue class from COMMS library\n"
            "///     for details.\n"
            "///\n"
            "///      The generated enum values and functions are:\n"
            "#^#ACCESS_DOC#$#\n"
            "COMMS_BITMASK_BITS_SEQ(\n"
            "    #^#NAMES#$#\n"
            ");\n";

        common::StringsList accessDoc;
        accessDoc.reserve(names.size());
        std::transform(
            names.begin(), names.end(), std::back_inserter(accessDoc),
            [](auto& n)
            {
                return "///      @li @b BitIdx_" + n + ", @b getBitValue_" + n + "() and @b setBitValue_" + n + "().";
            });

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("ACCESS_DOC", common::listToString(accessDoc, "\n", common::emptyString())));
        replacements.insert(std::make_pair("NAMES", common::listToString(names, ",\n", common::emptyString())));
        return common::processTemplate(Templ, replacements);
    }

    static const std::string Templ =
        "/// @brief Provide names for internal bits.\n"
        "/// @details See definition of @b COMMS_BITMASK_BITS macro\n"
        "///     related to @b comms::field::BitmaskValue class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///      The generated enum values:\n"
        "#^#BITS_DOC#$#\n"
        "COMMS_BITMASK_BITS(\n"
        "    #^#BITS#$#\n"
        ");\n\n"
        "/// @brief Generates independent access functions for internal bits.\n"
        "/// @details See definition of @b COMMS_BITMASK_BITS_ACCESS macro\n"
        "///     related to @b comms::field::BitmaskValue class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated access functions are:\n"
        "#^#ACCESS_DOC#$#\n"
        "COMMS_BITMASK_BITS_ACCESS(\n"
        "    #^#NAMES#$#\n"
        ");\n";

    common::StringsList bitsDoc;
    bitsDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsDoc),
        [](auto& n)
        {
            return "///      @li @b BitIdx_" + n + ".";
        });

    common::StringsList accessDoc;
    accessDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(accessDoc),
        [](auto& n)
        {
            return "///      @li @b getBitValue_" + n + "() and @b setBitValue_" + n + "().";
        });

    auto bits = obj.bits();
    common::StringsList bitsList;
    bitsList.reserve(bits.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsList),
        [&bits](auto& n)
        {
            auto iter = bits.find(n);
            if (iter == bits.end()) {
                assert(!"Should not happen");
                return common::emptyString();
            }

            return n + "=" + std::to_string(iter->second.m_idx);
        });

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ACCESS_DOC", common::listToString(accessDoc, "\n", common::emptyString())));
    replacements.insert(std::make_pair("NAMES", common::listToString(names, ",\n", common::emptyString())));
    replacements.insert(std::make_pair("BITS_DOC", common::listToString(bitsDoc, "\n", common::emptyString())));
    replacements.insert(std::make_pair("BITS", common::listToString(bitsList, ",\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string SetField::getValid() const
{
    auto custom = getCustomValid();
    if (!custom.empty()) {
        return custom;
    }

    auto obj = setFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        return common::emptyString();
    }

    using Key = std::tuple<unsigned, unsigned>;
    struct Value
    {
        std::uintmax_t m_reservedMask = 0U;
        std::uintmax_t m_reservedValue = 0U;
    };

    using Map = std::map<Key, Value>;
    Map bitsToCheck;

    std::uintmax_t allReservedMask = 0U;
    std::uintmax_t repeatingReservedMask = 0U;
    std::vector<unsigned> repeatingBits;
    auto& bits = obj.bits();
    for (auto& b : bits) {
        if ((b.second.m_sinceVersion <= obj.sinceVersion()) &&
            (obj.deprecatedSince() <= b.second.m_deprecatedSince)) {
            continue;
        }

        if (!generator().doesElementExist(b.second.m_sinceVersion, b.second.m_deprecatedSince, false)) {
            continue; // already handled
        }

        assert(!b.second.m_reserved); // Should be handled in options

        auto key = std::make_tuple(b.second.m_sinceVersion, b.second.m_deprecatedSince);
        auto& elem = bitsToCheck[key];

        auto bitMask = static_cast<std::uintmax_t>(1U) << b.second.m_idx;
        elem.m_reservedMask |= bitMask;

        if (b.second.m_reservedValue) {
            elem.m_reservedValue |= bitMask;
        }

        if (((allReservedMask & bitMask) != 0U) &&
            ((repeatingReservedMask & bitMask) == 0U)) {
            repeatingReservedMask |= bitMask;
            repeatingBits.push_back(b.second.m_idx);
        }

        allReservedMask |= bitMask;
    }

    if (repeatingReservedMask != 0U) {
        for (auto& b : bitsToCheck) {
            b.second.m_reservedMask &= ~repeatingReservedMask;
            b.second.m_reservedValue &= ~repeatingReservedMask;
        }
    }

    common::StringsList conditions;
    for (auto& info : bitsToCheck) {
        if (info.second.m_reservedMask == 0U) {
            continue;
        }

            static const std::string VersionBothCondTempl =
                "if (((Base::getVersion() < #^#FROM_VERSION#$#) || (#^#UNTIL_VERSION#$# <= Base::getVersion())) && \n"
                "    ((Base::value() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

            static const std::string VersionFromCondTempl =
                "if ((Base::getVersion() < #^#FROM_VERSION#$#) &&\n"
                "    ((Base::value() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

            static const std::string VersionUntilCondTempl =
                "if ((#^#UNTIL_VERSION#$# <= Base::getVersion()) &&\n"
                "    ((Base::value() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

        auto* condTempl = &VersionBothCondTempl;
        if (std::get<0>(info.first) == 0U) {
            assert(std::get<1>(info.first) != commsdsl::Protocol::notYetDeprecated());
            condTempl = &VersionUntilCondTempl;
        }
        else if (commsdsl::Protocol::notYetDeprecated() <= std::get<1>(info.first)) {
            condTempl = &VersionFromCondTempl;
        }


        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("BITS_MASK", common::numToString(info.second.m_reservedMask, true)));
        replacements.insert(std::make_pair("VALUE_MASK", common::numToString(info.second.m_reservedValue, true)));
        replacements.insert(std::make_pair("FROM_VERSION", common::numToString(std::get<0>(info.first))));
        replacements.insert(std::make_pair("UNTIL_VERSION", common::numToString(std::get<1>(info.first))));
        conditions.push_back(common::processTemplate(*condTempl, replacements));
    }

    for (auto idx : repeatingBits) {
        auto elems = obj.revBits().equal_range(idx);
        assert(elems.first != elems.second);
        std::vector<std::pair<unsigned, unsigned> > versionRanges;
        for (auto iter = elems.first; iter != elems.second; ++iter) {
            auto& bitName = iter->second;
            auto bitIter = bits.find(bitName);
            if (bitIter == bits.end()) {
                assert(!"Should not happen");
                continue;
            }

            versionRanges.push_back(std::make_pair(bitIter->second.m_sinceVersion, bitIter->second.m_deprecatedSince));
        }

        std::sort(
            versionRanges.begin(), versionRanges.end(),
            [](auto& e1, auto& e2)
            {
                if (e1.first != e2.first) {
                    return e1.first < e2.first;
                }

                return e1.second < e2.second;
            });

        unsigned prev = 0U;
        for (auto& r : versionRanges) {
            auto newPrev = r.second;
            r.second = r.first;
            r.first = prev;
            prev = newPrev;
        }

        if (prev < obj.deprecatedSince()) {
            versionRanges.push_back(std::make_pair(prev, obj.deprecatedSince()));
        }

        // unify ranges;
        assert(!versionRanges.empty());
        for (auto rIdx = 0U; rIdx < versionRanges.size(); ++rIdx) {
            auto& thisRange = versionRanges[rIdx];
            if ((thisRange.second == 0) || (thisRange.first == thisRange.second)) {
                continue;
            }

            for (auto nextIdx = rIdx + 1; nextIdx < versionRanges.size(); ++nextIdx) {
                auto& nextRange = versionRanges[nextIdx];
                if ((nextRange.second == 0) || (nextRange.first == nextRange.second)) {
                    continue;
                }

                if (thisRange.second < nextRange.first) {
                    break;
                }

                thisRange.second = nextRange.second;
                nextRange.second = 0U; // invalidate

            }
        }

        common::StringsList extraConds;
        for (auto& r : versionRanges) {
            if ((r.second == 0U) || (r.first == r.second)) {
                continue; // ignore invalid ranges
            }

            static const std::string Templ =
                "if ((#^#FROM_VERSION#$# <= Base::getVersion()) &&\n"
                "    (Base::getVersion() < #^#UNTIL_VERSION#$#)) {\n"
                "    return false;\n"
                "}";

            common::ReplacementMap replacements;
            replacements.insert(std::make_pair("FROM_VERSION", common::numToString(r.first)));
            replacements.insert(std::make_pair("UNTIL_VERSION", common::numToString(r.second)));
            extraConds.push_back(common::processTemplate(Templ, replacements));
        }

        static const std::string Templ =
            "if ((Base::value() & #^#RESERVED_MASK#$#) != #^#RESERVED_VALUE#$#) {\n"
            "    #^#CONDITIONS#$#\n"
            "}\n";

        auto bitMask = static_cast<std::uintmax_t>(1U) << idx;
        std::uintmax_t bitValue = 0U;

        auto& bitInfoName = elems.first->second;
        auto bitInfoIter = bits.find(bitInfoName);
        assert(bitInfoIter != bits.end());
        if (bitInfoIter->second.m_reservedValue) {
            bitValue |= bitMask;
        }

        if (!extraConds.empty()) {
            common::ReplacementMap replacements;
            replacements.insert(std::make_pair("RESERVED_MASK", common::numToString(bitMask, true)));
            replacements.insert(std::make_pair("RESERVED_VALUE", common::numToString(bitValue, true)));
            replacements.insert(std::make_pair("CONDITIONS", common::listToString(extraConds, "\n\n", common::emptyString())));

            conditions.push_back(common::processTemplate(Templ, replacements));
        }
    }

    if (conditions.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Validity check function.\n"
        "bool valid() const\n"
        "{\n"
        "    if (!Base::valid()) {\n"
        "        return false;\n"
        "    }\n\n"
        "    #^#CONDITIONS#$#\n"
        "    return true;\n"
        "}\n";

    std::string condStr = common::listToString(conditions, "\n", common::emptyString());
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CONDITIONS", std::move(condStr)));
    return common::processTemplate(Templ, replacements);
}

std::string SetField::getBitName() const
{
    auto obj = setFieldDslObj();
    auto& bits = obj.bits();
    auto& revBits = obj.revBits();
    std::intmax_t nextBit = 0;
    StringsList names;
    for (auto& b : revBits) {
        if (b.first < nextBit) {
            continue;
        }

        while (nextBit < b.first) {
            names.push_back("nullptr");
            ++nextBit;
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

        ++nextBit;
        auto addElementNameFunc = 
            [&names, getDisplayNameFunc](auto& infoPair) 
            {
                names.push_back('\"' + getDisplayNameFunc(infoPair) + '\"');
            };

        auto bitIter = bits.find(b.second);
        assert(bitIter != bits.end());
        if ((!obj.isNonUniqueAllowed()) || 
            (generator().schemaVersion() < bitIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*bitIter);
            continue;
        }

        auto allRevBits = revBits.equal_range(b.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevBits.first; iter != allRevBits.second; ++iter) {
            auto bIter = bits.find(iter->second);
            assert(bIter != bits.end());
            if (generator().schemaVersion() < bIter->second.m_deprecatedSince) {
                addElementNameFunc(*bIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*bitIter);
    }

    std::string namesStr = common::listToString(names, ",\n", common::emptyString());

    static const std::string Templ =
        "/// @brief Retrieve name of the bit\n"
        "static const char* bitName(std::size_t idx)\n"
        "{\n"
        "    static const char* Map[] = {\n"
        "        #^#NAMES#$#\n"
        "    };\n\n"
        "    static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n"
        "    if (MapSize <= idx) {\n"
        "        return nullptr;\n"
        "    }\n\n"
        "    return Map[idx];\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAMES", std::move(namesStr)));
    return common::processTemplate(Templ, replacements);
}

std::string SetField::getBitNameWrap(const std::string& scope) const
{
    static const std::string Templ =
        "/// @brief Retrieve name of the bit\n"
        "static const char* bitName(BitIdx idx)\n"
        "{\n"
        "    return\n"
        "        #^#SCOPE#$##^#CLASS_NAME#$#Common::bitName(\n"
        "            static_cast<std::size_t>(idx));\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("SCOPE", scopeForCommon(adjustScopeWithNamespace(scope))));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Templ, replacements);
}

void SetField::checkLengthOpt(StringsList& list) const
{
    auto bitLength = dslObj().bitLength();
    if (bitLength != 0U) {
        list.push_back("comms::option::def::FixedBitLength<" + common::numToString(bitLength) + '>');
        return;
    }

    list.push_back("comms::option::def::FixedLength<" + common::numToString(setFieldDslObj().minLength()) + '>');
}

void SetField::checkDefaultValueOpt(StringsList& list) const
{
    auto obj = setFieldDslObj();
    std::uintmax_t defaultValue = 0U;
    if (obj.defaultBitValue()) {
        defaultValue = ~defaultValue;
    }

    for (auto& bitInfo : obj.bits()) {
        if (MaxBits <= bitInfo.second.m_idx) {
            assert(!"Should not happen");
            continue;
        }

        auto bitMask = static_cast<decltype(defaultValue)>(1U) << bitInfo.second.m_idx;
        if (bitInfo.second.m_defaultValue) {
            defaultValue |= bitMask;
        }
        else {
            defaultValue &= ~(bitMask);
        }
    }

    auto bitLength = obj.bitLength();
    if (bitLength == 0U) {
        bitLength = obj.minLength() * 8U;
    }

    auto mask = ~static_cast<std::uintmax_t>(0);
    if (bitLength < MaxBits) {
        mask = (static_cast<decltype(defaultValue)>(1U) << bitLength) - 1;
    }

    defaultValue &= mask;

    if (defaultValue == 0U) {
        return;
    }

    auto type = obj.type();
    if ((type == commsdsl::SetField::Type::Uint64) || (type == commsdsl::SetField::Type::Uintvar)) {
        auto str =
            "comms::option::def::DefaultBigUnsignedNumValue<" +
            common::numToString(static_cast<std::uintmax_t>(defaultValue), true) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::def::DefaultNumValue<" +
        common::numToString(defaultValue, true) +
        '>';
    list.push_back(std::move(str));
}

void SetField::checkReservedBitsOpt(SetField::StringsList& list) const
{
    auto obj = setFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    std::uintmax_t reservedMask = ~static_cast<std::uintmax_t>(0U);
    std::uintmax_t reservedValue = 0U;
    if (obj.reservedBitValue()) {
        reservedValue = ~reservedValue;
    }

    bool mustHandleBitsInValidFunc = false;

    for (auto& bitInfo : obj.bits()) {
        if (MaxBits <= bitInfo.second.m_idx) {
            assert(!"Should not happen");
            continue;
        }

        if (validCheckVersion &&
            (!generator().doesElementExist(bitInfo.second.m_sinceVersion, bitInfo.second.m_deprecatedSince, true))) {
            continue;
        }

        auto bitMask = static_cast<decltype(reservedValue)>(1U) << bitInfo.second.m_idx;
        if (validCheckVersion &&
            ((obj.sinceVersion() < bitInfo.second.m_sinceVersion) || (bitInfo.second.m_deprecatedSince < obj.deprecatedSince()))) {
            mustHandleBitsInValidFunc = true;
            reservedMask &= ~(bitMask);
            reservedValue &= ~(bitMask);
            continue;
        }


        if (!bitInfo.second.m_reserved) {
            reservedMask &= ~(bitMask);
            reservedValue &= ~(bitMask);
            continue;
        }

        if (bitInfo.second.m_reservedValue) {
            reservedValue |= bitMask;
        }
        else {
            reservedValue &= ~(bitMask);
        }
    }

    reservedValue &= reservedMask;

    auto bitLength = obj.bitLength();
    if (bitLength == 0U) {
        bitLength = obj.minLength() * 8U;
    }

    auto mask = ~static_cast<std::uintmax_t>(0);
    if (bitLength < MaxBits) {
        mask = (static_cast<decltype(reservedValue)>(1U) << bitLength) - 1;
    }

    reservedValue &= mask;
    reservedMask &= mask;

    if (mustHandleBitsInValidFunc) {
        list.push_back("comms::option::def::VersionStorage");
    }

    if (reservedMask == 0U) {
        return;
    }

    std::string str =
        "comms::option::def::BitmaskReservedBits<" +
        common::numToString(reservedMask, true) +
        ", " +
        common::numToString(reservedValue, true) +
        '>';
    list.push_back(std::move(str));
}


} // namespace commsdsl2comms
