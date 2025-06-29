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

#include "ParseEnumFieldImpl.h"

#include "parse_common.h"
#include "ParseProtocolImpl.h"
#include "ParseIntFieldImpl.h"
#include "parse_util.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <type_traits>

namespace commsdsl
{

namespace parse
{

namespace
{

const std::size_t BitsInByte =
        std::numeric_limits<std::uint8_t>::digits;
static_assert(BitsInByte == 8U, "Invalid assumption");    

} // namespace

ParseEnumFieldImpl::ParseEnumFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

bool ParseEnumFieldImpl::parseIsUnique() const
{
    std::intmax_t prevKey = 0;
    bool firstElem = true;
    for (auto& v : m_state.m_revValues) {
        if (firstElem) {
            prevKey = v.first;
            firstElem = false;
            continue;
        }

        if (prevKey == v.first) {
            assert(parseIsNonUniqueAllowed());
            return false;
        }

        prevKey = v.first;
    }

    return true;
}

ParseFieldImpl::Kind ParseEnumFieldImpl::parseKindImpl() const
{
    return Kind::Enum;
}

ParseEnumFieldImpl::ParseEnumFieldImpl(const ParseEnumFieldImpl&) = default;

ParseFieldImpl::Ptr ParseEnumFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseEnumFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseEnumFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::typeStr(),
        common::defaultValueStr(),
        common::endianStr(),
        common::lengthStr(),
        common::bitLengthStr(),
        common::nonUniqueAllowedStr(),
        common::validCheckVersionStr(),
        common::hexAssignStr(),
        common::availableLengthLimitStr()
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseEnumFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::validValueStr()
    };

    return List;
}

bool ParseEnumFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseEnumFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool ParseEnumFieldImpl::parseImpl()
{
    return
        parseUpdateType() &&
        parseUpdateEndian() &&
        parseUpdateLength() &&
        parseUpdateBitLength() &&
        parseUpdateNonUniqueAllowed() &&
        parseUpdateValidCheckVersion() &&
        parseUpdateMinMaxValues() &&
        parseUpdateValues() &&
        parseUpdateDefaultValue() &&
        parseUpdateHexAssign() &&
        parseUpdateAvailableLengthLimit();
}

std::size_t ParseEnumFieldImpl::parseMinLengthImpl() const
{
    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        return 1U;
    }

    return m_state.m_length;
}

std::size_t ParseEnumFieldImpl::parseMaxLengthImpl() const
{
    return m_state.m_length;
}

std::size_t ParseEnumFieldImpl::parseBitLengthImpl() const
{
    if (parseIsBitfieldMember()) {
        return m_state.m_bitLength;
    }
    return Base::parseBitLengthImpl();
}

bool ParseEnumFieldImpl::parseIsComparableToValueImpl(const std::string& val) const
{
    std::intmax_t value = 0;
    return parseStrToValue(val, value);
}

bool ParseEnumFieldImpl::parseIsComparableToFieldImpl(const ParseFieldImpl& field) const
{
    auto fieldKind = field.parseKind();
    return ((fieldKind == Kind::Int) || (fieldKind == Kind::Enum));
}

bool ParseEnumFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (ref.empty() && (!parseProtocol().parseIsFieldValueReferenceSupported())) {
        return false;
    }

    auto updateIsBigUnsignedFunc =
        [this, &val, &isBigUnsigned]()
        {
            static const std::uintmax_t BigUnsignedThreshold =
                 static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());

            isBigUnsigned =
                ParseIntFieldImpl::parseIsBigUnsigned(m_state.m_type) &&
                (BigUnsignedThreshold < static_cast<std::uintmax_t>(val));
        };

    if (ref.empty()) {
        val = m_state.m_defaultValue;
        updateIsBigUnsignedFunc();
        return true;
    }

    auto iter = m_state.m_values.find(ref);
    if (iter == m_state.m_values.end()) {
        return false;
    }

    val = iter->second.m_value;
    updateIsBigUnsignedFunc();
    return true;
}

bool ParseEnumFieldImpl::parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const
{
    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
                      "Bitfield member cannot have variable length type.";
        return false;
    }

    assert(0U < m_state.m_length);
    auto maxBitLength = m_state.m_length * BitsInByte;
    if (maxBitLength < bitLength) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
                      "Value of property \"" << common::bitLengthStr() << "\" exceeds "
                      "maximal length available by the type and/or forced serialisation length.";
        return false;
    }

    return true;
}

bool ParseEnumFieldImpl::parseVerifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    return type == SemanticType::MessageId;
}

ParseEnumFieldImpl::FieldRefInfo ParseEnumFieldImpl::parseProcessInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());

    FieldRefInfo info;
    auto iter = m_state.m_values.find(refStr);
    if (iter != m_state.m_values.end()) {
        info.m_field = this;
        info.m_valueName = refStr;
        info.m_refType = FieldRefType_InnerValue;
    }

    return info;
}

bool ParseEnumFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool ParseEnumFieldImpl::parseUpdateType()
{
    bool mustHave = (m_state.m_type == Type::NumOfValues);
    if (!parseValidateSinglePropInstance(common::typeStr(), mustHave)) {
        return false;
    }

    auto propsIter = parseProps().find(common::typeStr());
    if (propsIter == parseProps().end()) {
        assert(m_state.m_type != Type::NumOfValues);
        return true;
    }

    auto newType = ParseIntFieldImpl::parseTypeValue(propsIter->second);
    if (newType == Type::NumOfValues) {
        parseReportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    if (mustHave) {
        m_state.m_type = newType;
        return true;
    }

    if (m_state.m_type == newType) {
        return true;
    }

    parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool ParseEnumFieldImpl::parseUpdateEndian()
{
    if (!parseValidateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(parseProps(), common::endianStr());
    if ((endianStr.empty()) && (m_state.m_endian != ParseEndian_NumOfValues)) {
        return true;
    }

    m_state.m_endian = common::parseEndian(endianStr, parseProtocol().parseCurrSchema().parseEndian());
    if (m_state.m_endian == ParseEndian_NumOfValues) {
        parseReportUnexpectedPropertyValue(common::endianStr(), endianStr);
        return false;
    }
    return true;
}

bool ParseEnumFieldImpl::parseUpdateLength()
{
    if (!parseValidateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto maxLength = ParseIntFieldImpl::parseMaxTypeLength(m_state.m_type);
    auto& lengthStr = common::getStringProp(parseProps(), common::lengthStr());
    if (lengthStr.empty()) {
        if (m_state.m_length == 0) {
            m_state.m_length = maxLength;
            return true;
        }

        assert(m_state.m_length <= ParseIntFieldImpl::parseMaxTypeLength(m_state.m_type));
        return true;
    }

    bool ok = false;
    auto newLength = static_cast<decltype(m_state.m_length)>(common::strToUintMax(lengthStr, &ok));

    if ((!ok) || (newLength == 0)) {
        parseReportUnexpectedPropertyValue(common::lengthStr(), lengthStr);
        return false;
    }

    if (m_state.m_length == newLength) {
        return true;
    }

    if (m_state.m_length != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Length cannot be changed after reuse";
        return false;
    }

    m_state.m_length = newLength;

    assert(0U < maxLength);
    if (maxLength < m_state.m_length) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << "Length of the \"" << parseName() << "\" element (" << lengthStr << ") cannot exceed "
                      "max length allowed by the type (" << maxLength << ").";
        return false;
    }

    assert (m_state.m_length != 0U);
    return true;
}

bool ParseEnumFieldImpl::parseUpdateBitLength()
{
    if (!parseValidateSinglePropInstance(common::bitLengthStr())) {
        return false;
    }

    auto maxBitLength = m_state.m_length * BitsInByte;
    assert((m_state.m_bitLength == 0) || (m_state.m_bitLength == maxBitLength));
    auto& valStr = common::getStringProp(parseProps(), common::bitLengthStr());
    if (valStr.empty()) {
        assert(0 < m_state.m_length);
        if (m_state.m_bitLength == 0) {
            m_state.m_bitLength = maxBitLength;
            return true;
        }

        assert(m_state.m_bitLength <= maxBitLength);
        return true;
    }

    if (!parseIsBitfieldMember()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix((parseGetNode())) <<
                        "The property \"" << common::bitLengthStr() << "\" is "
                        "applicable only to the members of \"" << common::bitfieldStr() << "\"";
        m_state.m_bitLength = maxBitLength;
        return true;
    }

    bool ok = false;
    m_state.m_bitLength = common::parseStrToUnsigned(valStr, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::bitLengthStr(), valStr);
        return false;
    }    

    if (!parseValidateBitLengthValue(m_state.m_bitLength)) {
        return false;
    }

    return true;
}

bool ParseEnumFieldImpl::parseUpdateNonUniqueAllowed()
{
    return parseValidateAndUpdateBoolPropValue(common::nonUniqueAllowedStr(), m_state.m_nonUniqueAllowed);
}

bool ParseEnumFieldImpl::parseUpdateValidCheckVersion()
{
    return parseValidateAndUpdateBoolPropValue(common::validCheckVersionStr(), m_state.m_validCheckVersion);
}

bool ParseEnumFieldImpl::parseUpdateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = ParseIntFieldImpl::parseMinTypeValue(m_state.m_type);
    m_state.m_typeAllowedMaxValue = ParseIntFieldImpl::parseMaxTypeValue(m_state.m_type);

    m_state.m_minValue = ParseIntFieldImpl::parseCalcMinValue(m_state.m_type, m_state.m_bitLength);
    m_state.m_maxValue = ParseIntFieldImpl::parseCalcMaxValue(m_state.m_type, m_state.m_bitLength);

    return true;
}

bool ParseEnumFieldImpl::parseUpdateValues()
{
    auto validValues = ParseXmlWrap::parseGetChildren(parseGetNode(), common::validValueStr());
    if (validValues.empty()) {
        if (!m_state.m_values.empty()) {
            assert(!m_state.m_revValues.empty());
            return true; // already has values
        }

        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "The enum \"" << parseName() << "\" doesn't list any valid value.";
        return false;
    }

    for (auto* vNode : validValues) {
        static const ParseXmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::valStr(),
            common::sinceVersionStr(),
            common::deprecatedStr(),
            common::descriptionStr(),
            common::displayNameStr()
        };

        auto props = ParseXmlWrap::parseNodeProps(vNode);
        if (!ParseXmlWrap::parseChildrenAsProps(vNode, PropNames, parseProtocol().parseLogger(), props)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(vNode, props, common::nameStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(vNode, props, common::valStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(vNode, props, common::sinceVersionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(vNode, props, common::deprecatedStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(vNode, props, common::descriptionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(vNode, props, common::displayNameStr(), parseProtocol().parseLogger())) {
            return false;
        }

        [[maybe_unused]] auto extraAttr = ParseXmlWrap::parseGetExtraAttributes(vNode, PropNames, parseProtocol());
        [[maybe_unused]] auto extraChildren = ParseXmlWrap::parseGetExtraChildren(vNode, PropNames, parseProtocol());

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(vNode) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto valuesIter = m_state.m_values.find(nameIter->second);
        if (valuesIter != m_state.m_values.end()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(vNode) << "Value with name \"" << nameIter->second <<
                          "\" has already been defined for enum \"" << parseName() << "\".";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        std::intmax_t val = 0;
        if (!parseStrToValue(valIter->second, val)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(vNode) << "Value of \"" << nameIter->second <<
                          "\" (" << valIter->second << ") cannot be recognized.";
            return false;
        }

        auto checkValueInRangeFunc =
            [this, vNode, &nameIter](auto v) -> bool
            {
                if ((v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) ||
                    (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v)) {
                    this->parseLogError() << ParseXmlWrap::parseLogPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of available values within a type.";
                    return false;
                }

                if ((v < static_cast<decltype(v)>(m_state.m_minValue)) ||
                    (static_cast<decltype(v)>(m_state.m_maxValue) < v)) {
                    this->parseLogWarning() << ParseXmlWrap::parseLogPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of correctly serializable values.";
                }

                return true;
            };

        bool checkResult = false;
        if (ParseIntFieldImpl::parseIsBigUnsigned(m_state.m_type)) {
            checkResult = checkValueInRangeFunc(static_cast<std::uintmax_t>(val));
        }
        else {
            checkResult = checkValueInRangeFunc(val);
        }

        if (!checkResult) {
            return false;
        }

        if (!m_state.m_nonUniqueAllowed) {
            auto revIter = m_state.m_revValues.find(val);
            if (revIter != m_state.m_revValues.end()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(vNode) <<
                              "Value \"" << valIter->second << "\" has been already defined "
                              "as \"" << revIter->second << "\".";
                return false;
            }
        }

        ValueInfo info;
        info.m_value = val;
        info.m_sinceVersion = parseGetSinceVersion();
        info.m_deprecatedSince = parseGetDeprecated();

        if (!ParseXmlWrap::parseGetAndCheckVersions(vNode, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
            return false;
        }

        auto descIter = props.find(common::descriptionStr());
        if (descIter != props.end()) {
            info.m_description = descIter->second;
        }

        auto dispNameIter = props.find(common::displayNameStr());
        if ((dispNameIter != props.end()) &&
            (!parseProtocol().parseStrToStringValue(dispNameIter->second, info.m_displayName))) {
            ParseXmlWrap::parseReportUnexpectedPropertyValue(vNode, nameIter->second, common::displayNameStr(), dispNameIter->second, parseProtocol().parseLogger());
            return false;
        }

        m_state.m_values.emplace(nameIter->second, info);
        m_state.m_revValues.emplace(val, nameIter->second);
    }
    return true;
}

bool ParseEnumFieldImpl::parseUpdateDefaultValue()
{
    if (!parseValidateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(parseProps(), common::defaultValueStr());
    if (valueStr.empty()) {
        return true;
    }

    auto reportErrorFunc =
        [this, &valueStr]()
        {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << "The default value of the \"" << parseName() <<
                          "\" is not within type boundaries (" << valueStr << ").";
        };

    auto reportWarningFunc =
        [this, &valueStr]()
        {
            parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << "The default value of the \"" << parseName() <<
                          "\" is too small or big and will not be serialised correctly (" << valueStr << ").";
        };

    auto checkValueFunc =
        [this, &reportErrorFunc, &reportWarningFunc](auto v) -> bool
        {
            auto castedTypeAllowedMinValue = static_cast<decltype(v)>(m_state.m_typeAllowedMinValue);
            auto castedTypeAllowedMaxValue = static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue);

            if (v < castedTypeAllowedMinValue) {
                reportErrorFunc();
                return false;
            }

            if (castedTypeAllowedMaxValue < v) {
                reportErrorFunc();
                return false;
            }

            auto castedMinValue = static_cast<decltype(v)>(m_state.m_minValue);
            auto castedMaxValue = static_cast<decltype(v)>(m_state.m_maxValue);
            if ((v < castedMinValue) ||
                (castedMaxValue < v)) {
                reportWarningFunc();
            }

            m_state.m_defaultValue = static_cast<decltype(m_state.m_defaultValue)>(v);
            return true;
        };

    std::intmax_t val = 0;
    if (!parseStrToValue(valueStr, val)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << "Default value (" << valueStr <<
                      ") cannot be recognized.";
        return false;
    }

    if (ParseIntFieldImpl::parseIsBigUnsigned(m_state.m_type)) {
        return checkValueFunc(static_cast<std::uintmax_t>(val));
    }

    return checkValueFunc(val);
}

bool ParseEnumFieldImpl::parseUpdateHexAssign()
{
    if (!parseValidateAndUpdateBoolPropValue(common::hexAssignStr(), m_state.m_hexAssign)) {
        return false;
    }

    auto& valueStr = common::getStringProp(parseProps(), common::hexAssignStr());
    if (valueStr.empty()) {
        return true;
    }    

    if (!ParseIntFieldImpl::parseIsTypeUnsigned(m_state.m_type)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot set \"" << common::hexAssignStr() << "\" property with signed types.";
        return false;
    }

    return true;
}

bool ParseEnumFieldImpl::parseUpdateAvailableLengthLimit()
{
    return parseValidateAndUpdateBoolPropValue(common::availableLengthLimitStr(), m_state.m_availableLengthLimit);
}

bool ParseEnumFieldImpl::parseStrToValue(
    const std::string& str,
    std::intmax_t& val) const
{
    if (common::isValidName(str)) {
        // Check among specials
        auto iter = m_state.m_values.find(str);
        if (iter != m_state.m_values.end()) {
            val = iter->second.m_value;
            return true;
        }
    }

   if (common::isValidRefName(str)) {
        bool bigUnsigned = false;
        if (!parseProtocol().parseStrToNumeric(str, false, val, bigUnsigned)) {
            return false;
        }

        if ((!bigUnsigned) && (val < 0) && (ParseIntFieldImpl::parseIsUnsigned(m_state.m_type))) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Cannot assign negative value (" << val << " references as " <<
                str << ") to field with positive type.";

            return false;
        }

        if (bigUnsigned && (!ParseIntFieldImpl::parseIsBigUnsigned(m_state.m_type))) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Cannot assign such big positive number (" <<
                static_cast<std::uintmax_t>(val) << " referenced as " <<
                str << ").";

            return false;
        }
        return true;
    }


    bool ok = false;
    if (ParseIntFieldImpl::parseIsBigUnsigned(m_state.m_type)) {
        val = static_cast<std::intmax_t>(common::strToUintMax(str, &ok));
    }
    else {
        val = common::strToIntMax(str, &ok);
    }
    return ok;
}


} // namespace parse

} // namespace commsdsl
