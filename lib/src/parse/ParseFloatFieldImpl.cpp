//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "ParseFloatFieldImpl.h"

#include "ParseProtocolImpl.h"
#include "parse_common.h"
#include "parse_util.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <limits>
#include <map>

namespace commsdsl
{

namespace parse
{

namespace
{

bool parseCompareLess(double val1, double val2)
{
    static_assert(-std::numeric_limits<double>::infinity() < std::numeric_limits<double>::infinity(), "Invalid assumption");

    if (std::isnan(val1)) {
        return false;
    }

    if (std::isnan(val2)) {
        return true;
    }

    if (std::isinf(val1)) {
        if (val1 < 0) {
            return (!std::isinf(val2)) || (0 < val2);
        }

        return false;
    }

    if (std::isinf(val2)) {
        if (val2 < 0) {
            return false;
        }

        return true;
    }

    return val1 < val2;
}

double parseMinValueForType(ParseFloatFieldImpl::Type value)
{
    static const double Values[] = {
        /* Type::Float */ double(std::numeric_limits<float>::lowest()),
        /* Type::Double */ std::numeric_limits<double>::lowest(),
    };

    static const std::size_t ValuesSize = std::extent<decltype(Values)>::value;

    static_assert(ValuesSize == util::toUnsigned(ParseFloatFieldImpl::Type::NumOfValues), "Invalid map");

    if (ValuesSize <= util::toUnsigned(value)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        value = ParseFloatFieldImpl::Type::Float;
    }

    return Values[util::toUnsigned(value)];
}

double parseMaxValueForType(ParseFloatFieldImpl::Type value)
{
    static const double Values[] = {
        /* Type::Float */ double(std::numeric_limits<float>::max()),
        /* Type::Double */ std::numeric_limits<double>::max(),
    };

    static const std::size_t ValuesSize = std::extent<decltype(Values)>::value;

    static_assert(ValuesSize == util::toUnsigned(ParseFloatFieldImpl::Type::NumOfValues), "Invalid map");

    if (ValuesSize <= util::toUnsigned(value)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        value = ParseFloatFieldImpl::Type::Float;
    }

    return Values[util::toUnsigned(value)];
}

} // namespace

ParseFloatFieldImpl::ParseFloatFieldImpl(xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
    m_state.m_nonUniqueSpecialsAllowed = !protocol.parseIsNonUniqueSpecialsAllowedSupported();
}

bool ParseFloatFieldImpl::parseHasNonUniqueSpecials() const
{
    if (!m_state.m_nonUniqueSpecialsAllowed) {
        return false;
    }


    std::vector<double> specValues;
    specValues.reserve(m_state.m_specials.size());

    for (auto& s : m_state.m_specials) {
        if (std::isnan(s.second.m_value)) {
            continue;
        }

        specValues.push_back(s.second.m_value);
    }

    if ((specValues.size() + 1U) < m_state.m_specials.size()) {
        // More than one NaN inside
        return true;
    }

    std::sort(specValues.begin(), specValues.end());

    bool firstValue = true;
    double prevValue = 0.0;
    for (auto s : specValues) {
        if ((!firstValue) && (prevValue == s)) {
            return true;
        }
        firstValue = false;
        prevValue = s;
    }
    return false;
}

ParseFieldImpl::Kind ParseFloatFieldImpl::parseKindImpl() const
{
    return Kind::Float;
}

ParseFloatFieldImpl::ParseFloatFieldImpl(const ParseFloatFieldImpl&) = default;

ParseFieldImpl::Ptr ParseFloatFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseFloatFieldImpl(*this));
}

const ParseXmlWrap::NamesList&ParseFloatFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::parseTypeStr(),
        common::parseDefaultValueStr(),
        common::parseEndianStr(),
        common::parseValidRangeStr(),
        common::parseValidFullRangeStr(),
        common::parseValidValueStr(),
        common::parseValidMinStr(),
        common::parseValidMaxStr(),
        common::parseValidCheckVersionStr(),
        common::parseUnitsStr(),
        common::parseDisplayDesimalsStr(),
        common::parseNonUniqueSpecialsAllowedStr(),
        common::parseDisplaySpecialsStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList&ParseFloatFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::parseSpecialStr()
    };

    return List;
}

bool ParseFloatFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseFloatFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool ParseFloatFieldImpl::parseImpl()
{
    return
        parseUpdateType() &&
        parseUpdateEndian() &&
        parseUpdateLength() &&
        parseUpdateMinMaxValues() &&
        parseUpdateNonUniqueSpecialsAllowed() &&
        parseUpdateSpecials() &&
        parseUpdateDefaultValue() &&
        parseUpdateValidCheckVersion() &&
        parseUpdateValidRanges() &&
        parseUpdateUnits() &&
        parseUpdateDisplayDecimals() &&
        parseUpdateDisplaySpecials();
}

std::size_t ParseFloatFieldImpl::parseMinLengthImpl() const
{
    return m_state.m_length;
}

bool ParseFloatFieldImpl::parseIsComparableToValueImpl(const std::string& val) const
{
    double value = 0.0;
    return parseStrToValue(val, value, true);
}

bool ParseFloatFieldImpl::parseStrToFpImpl(const std::string& ref, double& val) const
{
    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
        return false;
    }

    if (ref.empty()) {
        val = m_state.m_defaultValue;
        return true;
    }

    auto iter = m_state.m_specials.find(ref);
    if (iter == m_state.m_specials.end()) {
        return false;
    }

    val = iter->second.m_value;
    return true;
}

ParseFloatFieldImpl::FieldRefInfo ParseFloatFieldImpl::parseProcessInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());
    FieldRefInfo info;
    auto iter = m_state.m_specials.find(refStr);
    if (iter != m_state.m_specials.end()) {
        info.m_field = this;
        info.m_valueName = refStr;
        info.m_refType = FieldRefType_InnerValue;
    }

    return info;
}

bool ParseFloatFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool ParseFloatFieldImpl::parseUpdateType()
{
    bool mustHave = (m_state.m_type == Type::NumOfValues);
    if (!parseValidateSinglePropInstance(common::parseTypeStr(), mustHave)) {
        return false;
    }

    auto propsIter = parseProps().find(common::parseTypeStr());
    if (propsIter == parseProps().end()) {
        assert(!mustHave);
        return true;
    }

    static const std::string Map[] = {
        /* Type::Float */ "float",
        /* Type::Double */ "double"
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(Type::NumOfValues), "Invalid map");

    auto typeStr = common::parseToLowerCopy(propsIter->second);
    auto iter = std::find(std::begin(Map), std::end(Map), typeStr);
    if (iter == std::end(Map)) {
        parseReportUnexpectedPropertyValue(common::parseTypeStr(), propsIter->second);
        return false;
    }

    auto newType = static_cast<decltype(m_state.m_type)>(std::distance(std::begin(Map), iter));
    if (m_state.m_type == Type::NumOfValues) {
        m_state.m_type = newType;
        return true;
    }

    if (newType == m_state.m_type) {
        return true;
    }

    parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool ParseFloatFieldImpl::parseUpdateEndian()
{
    if (!parseValidateSinglePropInstance(common::parseEndianStr())) {
        return false;
    }

    auto& endianStr = common::parseGetStringProp(parseProps(), common::parseEndianStr());
    m_state.m_endian = common::parseEndian(endianStr, parseProtocol().parseCurrSchema().parseEndian());
    if (m_state.m_endian == ParseEndian_NumOfValues) {
        parseReportUnexpectedPropertyValue(common::parseEndianStr(), endianStr);
        return false;
    }

    return true;
}

bool ParseFloatFieldImpl::parseUpdateLength()
{
    static const std::size_t Map[] = {
        /* Type::Float */ sizeof(float),
        /* Type::Double */ sizeof(double),
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(Type::NumOfValues), "Invalid map");
    static_assert(sizeof(float) == 4U, "Invalid size assumption");
    static_assert(sizeof(double) == 8U, "Invalid size assumption");

    if (MapSize <= util::toUnsigned(m_state.m_type)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return false;
    }

    m_state.m_length = Map[util::toUnsigned(m_state.m_type)];
    return true;
}

bool ParseFloatFieldImpl::parseUpdateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = parseMinValueForType(m_state.m_type);
    m_state.m_typeAllowedMaxValue = parseMaxValueForType(m_state.m_type);
    return true;
}

bool ParseFloatFieldImpl::parseUpdateDefaultValue()
{
    if (!parseValidateSinglePropInstance(common::parseDefaultValueStr())) {
        return false;
    }

    auto& valueStr = common::parseGetStringProp(parseProps(), common::parseDefaultValueStr());
    if (valueStr.empty()) {
        return true;
    }

    if (!parseStrToValue(valueStr, m_state.m_defaultValue)) {
        parseReportUnexpectedPropertyValue(common::parseDefaultValueStr(), valueStr);
        return false;
    }

    bool isSpecial = std::isnan(m_state.m_defaultValue) || std::isinf(m_state.m_defaultValue);
    if (isSpecial) {
        return true;
    }

    if (m_state.m_typeAllowedMaxValue < m_state.m_defaultValue) {
        parseLogError() << "Value of property \"" << common::parseDefaultValueStr() <<
                        "\" is greater than the type's maximal value.";
        return false;
    }

    if (m_state.m_defaultValue < m_state.m_typeAllowedMinValue) {
        parseLogError() << "Value of property \"" << common::parseDefaultValueStr() <<
                        "\" is less than the type's minimal value.";
        return false;
    }

    return true;
}

bool ParseFloatFieldImpl::parseUpdateValidCheckVersion()
{
    return parseValidateAndUpdateBoolPropValue(common::parseValidCheckVersionStr(), m_state.m_validCheckVersion);
}

bool ParseFloatFieldImpl::parseUpdateValidRanges()
{
    auto attrs = ParseXmlWrap::parseNodeProps(parseGetNode());
    bool result =
        parseCheckFullRangeProps(attrs) &&
        parseCheckValidRangeProps(attrs) &&
        parseCheckValidValueProps(attrs) &&
        parseCheckValidMinProps(attrs) &&
        parseCheckValidMaxProps(attrs);
    if (!result) {
        return false;
    }

    // sort by version
    assert(std::isinf(-std::numeric_limits<double>::infinity()));
    std::sort(
        m_state.m_validRanges.begin(), m_state.m_validRanges.end(),
        [](auto& elem1, auto& elem2)
        {
            assert(elem1.m_deprecatedSince != 0U);
            assert(elem2.m_deprecatedSince != 0U);
            if (elem1.m_sinceVersion != elem2.m_sinceVersion) {
                return elem1.m_sinceVersion < elem2.m_sinceVersion;
            }

            if (elem1.m_deprecatedSince != elem2.m_deprecatedSince) {
                return elem1.m_deprecatedSince < elem2.m_deprecatedSince;
            }

            if (parseCompareLess(elem1.m_min, elem2.m_min)) {
                return true;
            }

            if (parseCompareLess(elem2.m_min, elem1.m_min)) {
                return false;
            }

            return parseCompareLess(elem1.m_max, elem2.m_max);
        });

    // Merge
    for (auto iter = m_state.m_validRanges.begin(); iter != m_state.m_validRanges.end(); ++iter) {
        if (iter->m_deprecatedSince == 0U) {
            continue;
        }

        for (auto nextIter = iter + 1; nextIter != m_state.m_validRanges.end(); ++nextIter) {
            if (nextIter->m_deprecatedSince == 0U) {
                continue;
            }

            auto isSpecial =
                    [](double val)
                    {
                        return std::isinf(val) || std::isnan(val);
                    };

            if ((iter->m_sinceVersion != nextIter->m_sinceVersion) ||
                (iter->m_deprecatedSince != nextIter->m_deprecatedSince) ||
                (isSpecial(iter->m_max)) ||
                (isSpecial(nextIter->m_min)) ||
                (parseCompareLess(iter->m_max, nextIter->m_min))) {
                break;
            }

            assert(!isSpecial(nextIter->m_max));
            nextIter->m_deprecatedSince = 0U; // invalidate next range
            iter->m_max = std::max(iter->m_max, nextIter->m_max);
        }
    }

    // Remove invalid
    m_state.m_validRanges.erase(
        std::remove_if(m_state.m_validRanges.begin(), m_state.m_validRanges.end(),
                    [](auto& elem)
                    {
                        return elem.m_deprecatedSince == 0U;
                    }),
        m_state.m_validRanges.end());

    // Sort by min/max value
    std::sort(
        m_state.m_validRanges.begin(), m_state.m_validRanges.end(),
        [](auto& elem1, auto& elem2)
        {
            assert(elem1.m_deprecatedSince != 0U);
            assert(elem2.m_deprecatedSince != 0U);
            if (parseCompareLess(elem1.m_min, elem2.m_min)) {
                return true;
            }

            if (parseCompareLess(elem2.m_min, elem1.m_min)) {
                return false;
            }

            if (parseCompareLess(elem1.m_max, elem2.m_max)) {
                return true;
            }

            if (parseCompareLess(elem2.m_max, elem1.m_max)) {
                return false;
            }

            if (elem1.m_sinceVersion != elem2.m_sinceVersion) {
                return elem1.m_sinceVersion < elem2.m_sinceVersion;
            }

            assert(elem1.m_deprecatedSince != elem2.m_deprecatedSince);
            return elem1.m_deprecatedSince < elem2.m_deprecatedSince;
        });

    return true;
}

bool ParseFloatFieldImpl::parseUpdateNonUniqueSpecialsAllowed()
{
    return parseValidateAndUpdateBoolPropValue(common::parseNonUniqueSpecialsAllowedStr(), m_state.m_nonUniqueSpecialsAllowed);
}

bool ParseFloatFieldImpl::parseUpdateSpecials()
{
    auto specials = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseSpecialStr());

    using RecSpecials = std::multimap<double, std::string>;
    RecSpecials recSpecials;
    std::string prevNanSpecial;

    for (auto* s : specials) {
        static const ParseXmlWrap::NamesList PropNames = {
            common::parseNameStr(),
            common::parseValStr(),
            common::parseSinceVersionStr(),
            common::parseDeprecatedStr(),
            common::parseDescriptionStr(),
            common::parseDisplayNameStr()
        };

        auto props = ParseXmlWrap::parseNodeProps(s);
        if (!ParseXmlWrap::parseChildrenAsProps(s, PropNames, parseProtocol().parseLogger(), props)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::parseNameStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::parseValStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::parseSinceVersionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::parseDeprecatedStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::parseDescriptionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::parseDisplayNameStr(), parseProtocol().parseLogger())) {
            return false;
        }

        auto nameIter = props.find(common::parseNameStr());
        assert(nameIter != props.end());

        if (!common::parseIsValidName(nameIter->second)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                  "Property \"" << common::parseNameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto specialsIter = m_state.m_specials.find(nameIter->second);
        if (specialsIter != m_state.m_specials.end()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(s) << "Special with name \"" << nameIter->second <<
                          "\" was already assigned to \"" << parseName() << "\" element.";
            return false;
        }

        auto valIter = props.find(common::parseValStr());
        assert(valIter != props.end());

        double val = 0.0;
        if (!parseStrToValue(valIter->second, val)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                "Value of \"" << nameIter->second << "\" (" << valIter->second << ") cannot be recognized.";
            return false;
        }

        if (!m_state.m_nonUniqueSpecialsAllowed) {
            bool reportError = false;
            const std::string* prevDefName = nullptr;
            do {
                if (std::isnan(val)) {
                    reportError = !prevNanSpecial.empty();
                    prevNanSpecial = nameIter->second;
                    prevDefName = &prevNanSpecial;
                    break;
                }

                auto recIter = recSpecials.find(val);
                if (recIter != recSpecials.end()) {
                    reportError = true;
                    prevDefName = &(recIter->second);
                }
                else {
                    recSpecials.insert(std::make_pair(val, nameIter->second));
                }
            } while (false);

            if (reportError) {
                assert(prevDefName != nullptr);
                parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                    "Value of special \"" << nameIter->second <<
                    "\" (" << valIter->second << ") has already been defined as \"" <<
                    *prevDefName << "\".";
                return false;
            }
        }

        bool isSpecial = std::isnan(val) || std::isinf(val);
        if ((!isSpecial) &&
            ((val < m_state.m_typeAllowedMinValue) || (m_state.m_typeAllowedMaxValue < val))) {
            this->parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                "Special value \"" << nameIter->second << "\" (" <<
                valIter->second << ") is outside the range of available values within a type.";
            return false;
        }

        SpecialValueInfo info;
        info.m_value = val;
        info.m_sinceVersion = parseGetSinceVersion();
        info.m_deprecatedSince = parseGetDeprecated();

        if (!ParseXmlWrap::parseGetAndCheckVersions(s, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
            return false;
        }

        auto descIter = props.find(common::parseDescriptionStr());
        if (descIter != props.end()) {
            info.m_description = descIter->second;
        }

        auto displayNameIter = props.find(common::parseDisplayNameStr());
        if (displayNameIter != props.end()) {
            info.m_displayName = displayNameIter->second;
        }

        m_state.m_specials.emplace(nameIter->second, info);
    }

    return true;
}

bool ParseFloatFieldImpl::parseUpdateUnits()
{
    if (!parseValidateSinglePropInstance(common::parseUnitsStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseUnitsStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_units = common::parseStrToUnits(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseUnitsStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseFloatFieldImpl::parseUpdateDisplayDecimals()
{
    if (!parseValidateSinglePropInstance(common::parseDisplayDesimalsStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseDisplayDesimalsStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_displayDecimals = common::parseStrToUnsigned(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseDisplayDesimalsStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseFloatFieldImpl::parseUpdateDisplaySpecials()
{
    parseCheckAndReportDeprecatedPropertyValue(common::parseDisplaySpecialsStr());
    return true;
}

bool ParseFloatFieldImpl::parseCheckFullRangeAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::parseValidFullRangeStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    bool ok = false;
    bool fullRange = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseValidFullRangeStr(), iter->second);
        return false;
    }

    if (!fullRange) {
        return true;
    }

    ValidRangeInfo info;
    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();
    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckFullRangeAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    bool ok = false;
    bool fullRange = common::parseStrToBool(str, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseValidFullRangeStr(), str);
        return false;
    }

    if (!fullRange) {
        return true;
    }

    ValidRangeInfo info;
    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckFullRangeProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseValidateSinglePropInstance(common::parseValidFullRangeStr())) {
        return false;
    }

    if (!parseCheckFullRangeAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseValidFullRangeStr());
    for (auto* c : children) {
        if (!parseCheckFullRangeAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseFloatFieldImpl::parseCheckValidRangeAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::parseValidRangeStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;

    if (!parseValidateValidRangeStr(iter->second, info.m_min, info.m_max)) {
        return false;
    }

    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();
    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidRangeAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    ValidRangeInfo info;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!parseValidateValidRangeStr(str, info.m_min, info.m_max)) {
        return false;
    }

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidRangeProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidRangeAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseValidRangeStr());
    for (auto* c : children) {
        if (!parseCheckValidRangeAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseFloatFieldImpl::parseCheckValidValueAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::parseValidValueStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!parseValidateValidValueStr(iter->second, common::parseValidValueStr(), info.m_min)) {
        return false;
    }

    info.m_max = info.m_min;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidValueAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!parseValidateValidValueStr(str, common::parseValidValueStr(), info.m_min)) {
        return false;
    }

    info.m_max = info.m_min;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidValueProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidValueAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseValidValueStr());
    for (auto* c : children) {
        if (!parseCheckValidValueAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseFloatFieldImpl::parseCheckValidMinAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::parseValidMinStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!parseValidateValidValueStr(iter->second, common::parseValidMinStr(), info.m_min, false)) {
        return false;
    }

    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidMinAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!parseValidateValidValueStr(str, common::parseValidMinStr(), info.m_min, false)) {
        return false;
    }

    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidMinProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidMinAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseValidMinStr());
    for (auto* c : children) {
        if (!parseCheckValidMinAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseFloatFieldImpl::parseCheckValidMaxAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::parseValidMaxStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!parseValidateValidValueStr(iter->second, common::parseValidMaxStr(), info.m_max, false)) {
        return false;
    }

    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidMaxAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!parseValidateValidValueStr(str, common::parseValidMaxStr(), info.m_max, false)) {
        return false;
    }

    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseFloatFieldImpl::parseCheckValidMaxProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidMaxAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseValidMaxStr());
    for (auto* c : children) {
        if (!parseCheckValidMaxAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseFloatFieldImpl::parseValidateValidRangeStr(const std::string& str, double& min, double& max)
{
    bool ok = false;
    auto range = common::parseRange(str, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseValidRangeStr(), str);
        return false;
    }

    if (!parseStrToValue(range.first, min, false)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Invalid min value in valid range (" << str << ").";
        return false;
    }

    if (!parseStrToValue(range.second, max, false)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Invalid max value in valid range (" << str << ").";
        return false;
    }

    if (max < min) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Min value must be less than max in valid range (" << str << ").";
        return false;
    }

    return true;
}

bool ParseFloatFieldImpl::parseValidateValidValueStr(
    const std::string& str,
    const std::string& type,
    double& val,
    bool allowSpecials)
{
    if (!parseStrToValue(str, val, allowSpecials)) {
        parseReportUnexpectedPropertyValue(type, str);
        return false;
    }

    if (std::isnan(val) || std::isinf(val)) {
        return true;
    }

    if (m_state.m_typeAllowedMaxValue < val) {
        parseLogError() << "Value of property \"" << type <<
                        "\" is greater than the type's maximal value.";
        return false;
    }

    if (val < m_state.m_typeAllowedMinValue) {
        parseLogError() << "Value of property \"" << type <<
                        "\" is less than the type's minimal value.";
        return false;
    }

    return true;
}

bool ParseFloatFieldImpl::parseStrToValue(
    const std::string& str,
    double& val,
    bool allowSpecials) const
{
    auto strCpy = common::parseToLowerCopy(str);
    do {
        if (allowSpecials && common::parseIsFpSpecial(strCpy)) {
            break;
        }

        if (common::parseIsValidName(str)) {
            auto iter = m_state.m_specials.find(str);
            if (iter != m_state.m_specials.end()) {
                val = iter->second.m_value;
                return true;
            }
        }

        if (common::parseIsValidRefName(str)) {

             if (!parseProtocol().parseStrToFp(str, false, val)) {
                 return false;
             }

             return true;
         }

    } while (false);

    bool ok = false;
    val = common::parseStrToDouble(strCpy, &ok, allowSpecials);
    return ok;
}

} // namespace parse

} // namespace commsdsl
