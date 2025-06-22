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

bool ParseEnumFieldImpl::isUnique() const
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
            assert(isNonUniqueAllowed());
            return false;
        }

        prevKey = v.first;
    }

    return true;
}

ParseFieldImpl::Kind ParseEnumFieldImpl::kindImpl() const
{
    return Kind::Enum;
}

ParseEnumFieldImpl::ParseEnumFieldImpl(const ParseEnumFieldImpl&) = default;

ParseFieldImpl::Ptr ParseEnumFieldImpl::cloneImpl() const
{
    return Ptr(new ParseEnumFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseEnumFieldImpl::extraPropsNamesImpl() const
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

const ParseXmlWrap::NamesList& ParseEnumFieldImpl::extraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::validValueStr()
    };

    return List;
}

bool ParseEnumFieldImpl::reuseImpl(const ParseFieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ParseEnumFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool ParseEnumFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateLength() &&
        updateBitLength() &&
        updateNonUniqueAllowed() &&
        updateValidCheckVersion() &&
        updateMinMaxValues() &&
        updateValues() &&
        updateDefaultValue() &&
        updateHexAssign() &&
        updateAvailableLengthLimit();
}

std::size_t ParseEnumFieldImpl::minLengthImpl() const
{
    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        return 1U;
    }

    return m_state.m_length;
}

std::size_t ParseEnumFieldImpl::maxLengthImpl() const
{
    return m_state.m_length;
}

std::size_t ParseEnumFieldImpl::bitLengthImpl() const
{
    if (isBitfieldMember()) {
        return m_state.m_bitLength;
    }
    return Base::bitLengthImpl();
}

bool ParseEnumFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    std::intmax_t value = 0;
    return strToValue(val, value);
}

bool ParseEnumFieldImpl::isComparableToFieldImpl(const ParseFieldImpl& field) const
{
    auto fieldKind = field.kind();
    return ((fieldKind == Kind::Int) || (fieldKind == Kind::Enum));
}

bool ParseEnumFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (ref.empty() && (!protocol().isFieldValueReferenceSupported())) {
        return false;
    }

    auto updateIsBigUnsignedFunc =
        [this, &val, &isBigUnsigned]()
        {
            static const std::uintmax_t BigUnsignedThreshold =
                 static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());

            isBigUnsigned =
                ParseIntFieldImpl::isBigUnsigned(m_state.m_type) &&
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

bool ParseEnumFieldImpl::validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const
{
    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        logError() << ParseXmlWrap::logPrefix(node) <<
                      "Bitfield member cannot have variable length type.";
        return false;
    }

    assert(0U < m_state.m_length);
    auto maxBitLength = m_state.m_length * BitsInByte;
    if (maxBitLength < bitLength) {
        logError() << ParseXmlWrap::logPrefix(node) <<
                      "Value of property \"" << common::bitLengthStr() << "\" exceeds "
                      "maximal length available by the type and/or forced serialisation length.";
        return false;
    }

    return true;
}

bool ParseEnumFieldImpl::verifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    return type == SemanticType::MessageId;
}

ParseEnumFieldImpl::FieldRefInfo ParseEnumFieldImpl::processInnerRefImpl(const std::string& refStr) const
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

bool ParseEnumFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool ParseEnumFieldImpl::updateType()
{
    bool mustHave = (m_state.m_type == Type::NumOfValues);
    if (!validateSinglePropInstance(common::typeStr(), mustHave)) {
        return false;
    }

    auto propsIter = props().find(common::typeStr());
    if (propsIter == props().end()) {
        assert(m_state.m_type != Type::NumOfValues);
        return true;
    }

    auto newType = ParseIntFieldImpl::parseTypeValue(propsIter->second);
    if (newType == Type::NumOfValues) {
        reportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    if (mustHave) {
        m_state.m_type = newType;
        return true;
    }

    if (m_state.m_type == newType) {
        return true;
    }

    logError() << ParseXmlWrap::logPrefix(getNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool ParseEnumFieldImpl::updateEndian()
{
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(props(), common::endianStr());
    if ((endianStr.empty()) && (m_state.m_endian != ParseEndian_NumOfValues)) {
        return true;
    }

    m_state.m_endian = common::parseEndian(endianStr, protocol().currSchema().endian());
    if (m_state.m_endian == ParseEndian_NumOfValues) {
        reportUnexpectedPropertyValue(common::endianStr(), endianStr);
        return false;
    }
    return true;
}

bool ParseEnumFieldImpl::updateLength()
{
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto maxLength = ParseIntFieldImpl::maxTypeLength(m_state.m_type);
    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    if (lengthStr.empty()) {
        if (m_state.m_length == 0) {
            m_state.m_length = maxLength;
            return true;
        }

        assert(m_state.m_length <= ParseIntFieldImpl::maxTypeLength(m_state.m_type));
        return true;
    }

    bool ok = false;
    auto newLength = static_cast<decltype(m_state.m_length)>(common::strToUintMax(lengthStr, &ok));

    if ((!ok) || (newLength == 0)) {
        reportUnexpectedPropertyValue(common::lengthStr(), lengthStr);
        return false;
    }

    if (m_state.m_length == newLength) {
        return true;
    }

    if (m_state.m_length != 0U) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Length cannot be changed after reuse";
        return false;
    }

    m_state.m_length = newLength;

    assert(0U < maxLength);
    if (maxLength < m_state.m_length) {
        logError() << ParseXmlWrap::logPrefix(getNode()) << "Length of the \"" << name() << "\" element (" << lengthStr << ") cannot exceed "
                      "max length allowed by the type (" << maxLength << ").";
        return false;
    }

    assert (m_state.m_length != 0U);
    return true;
}

bool ParseEnumFieldImpl::updateBitLength()
{
    if (!validateSinglePropInstance(common::bitLengthStr())) {
        return false;
    }

    auto maxBitLength = m_state.m_length * BitsInByte;
    assert((m_state.m_bitLength == 0) || (m_state.m_bitLength == maxBitLength));
    auto& valStr = common::getStringProp(props(), common::bitLengthStr());
    if (valStr.empty()) {
        assert(0 < m_state.m_length);
        if (m_state.m_bitLength == 0) {
            m_state.m_bitLength = maxBitLength;
            return true;
        }

        assert(m_state.m_bitLength <= maxBitLength);
        return true;
    }

    if (!isBitfieldMember()) {
        logWarning() << ParseXmlWrap::logPrefix((getNode())) <<
                        "The property \"" << common::bitLengthStr() << "\" is "
                        "applicable only to the members of \"" << common::bitfieldStr() << "\"";
        m_state.m_bitLength = maxBitLength;
        return true;
    }

    bool ok = false;
    m_state.m_bitLength = common::strToUnsigned(valStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::bitLengthStr(), valStr);
        return false;
    }    

    if (!validateBitLengthValue(m_state.m_bitLength)) {
        return false;
    }

    return true;
}

bool ParseEnumFieldImpl::updateNonUniqueAllowed()
{
    return validateAndUpdateBoolPropValue(common::nonUniqueAllowedStr(), m_state.m_nonUniqueAllowed);
}

bool ParseEnumFieldImpl::updateValidCheckVersion()
{
    return validateAndUpdateBoolPropValue(common::validCheckVersionStr(), m_state.m_validCheckVersion);
}

bool ParseEnumFieldImpl::updateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = ParseIntFieldImpl::minTypeValue(m_state.m_type);
    m_state.m_typeAllowedMaxValue = ParseIntFieldImpl::maxTypeValue(m_state.m_type);

    m_state.m_minValue = ParseIntFieldImpl::calcMinValue(m_state.m_type, m_state.m_bitLength);
    m_state.m_maxValue = ParseIntFieldImpl::calcMaxValue(m_state.m_type, m_state.m_bitLength);

    return true;
}

bool ParseEnumFieldImpl::updateValues()
{
    auto validValues = ParseXmlWrap::getChildren(getNode(), common::validValueStr());
    if (validValues.empty()) {
        if (!m_state.m_values.empty()) {
            assert(!m_state.m_revValues.empty());
            return true; // already has values
        }

        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "The enum \"" << name() << "\" doesn't list any valid value.";
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
        if (!ParseXmlWrap::parseChildrenAsProps(vNode, PropNames, protocol().logger(), props)) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(vNode, props, common::nameStr(), protocol().logger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(vNode, props, common::valStr(), protocol().logger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(vNode, props, common::sinceVersionStr(), protocol().logger())) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(vNode, props, common::deprecatedStr(), protocol().logger())) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(vNode, props, common::descriptionStr(), protocol().logger())) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(vNode, props, common::displayNameStr(), protocol().logger())) {
            return false;
        }

        [[maybe_unused]] auto extraAttr = ParseXmlWrap::getExtraAttributes(vNode, PropNames, protocol());
        [[maybe_unused]] auto extraChildren = ParseXmlWrap::getExtraChildren(vNode, PropNames, protocol());

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << ParseXmlWrap::logPrefix(vNode) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto valuesIter = m_state.m_values.find(nameIter->second);
        if (valuesIter != m_state.m_values.end()) {
            logError() << ParseXmlWrap::logPrefix(vNode) << "Value with name \"" << nameIter->second <<
                          "\" has already been defined for enum \"" << name() << "\".";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        std::intmax_t val = 0;
        if (!strToValue(valIter->second, val)) {
            logError() << ParseXmlWrap::logPrefix(vNode) << "Value of \"" << nameIter->second <<
                          "\" (" << valIter->second << ") cannot be recognized.";
            return false;
        }

        auto checkValueInRangeFunc =
            [this, vNode, &nameIter](auto v) -> bool
            {
                if ((v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) ||
                    (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v)) {
                    this->logError() << ParseXmlWrap::logPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of available values within a type.";
                    return false;
                }

                if ((v < static_cast<decltype(v)>(m_state.m_minValue)) ||
                    (static_cast<decltype(v)>(m_state.m_maxValue) < v)) {
                    this->logWarning() << ParseXmlWrap::logPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of correctly serializable values.";
                }

                return true;
            };

        bool checkResult = false;
        if (ParseIntFieldImpl::isBigUnsigned(m_state.m_type)) {
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
                logError() << ParseXmlWrap::logPrefix(vNode) <<
                              "Value \"" << valIter->second << "\" has been already defined "
                              "as \"" << revIter->second << "\".";
                return false;
            }
        }

        ValueInfo info;
        info.m_value = val;
        info.m_sinceVersion = getSinceVersion();
        info.m_deprecatedSince = getDeprecated();

        if (!ParseXmlWrap::getAndCheckVersions(vNode, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
            return false;
        }

        auto descIter = props.find(common::descriptionStr());
        if (descIter != props.end()) {
            info.m_description = descIter->second;
        }

        auto dispNameIter = props.find(common::displayNameStr());
        if ((dispNameIter != props.end()) &&
            (!protocol().strToStringValue(dispNameIter->second, info.m_displayName))) {
            ParseXmlWrap::reportUnexpectedPropertyValue(vNode, nameIter->second, common::displayNameStr(), dispNameIter->second, protocol().logger());
            return false;
        }

        m_state.m_values.emplace(nameIter->second, info);
        m_state.m_revValues.emplace(val, nameIter->second);
    }
    return true;
}

bool ParseEnumFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::defaultValueStr());
    if (valueStr.empty()) {
        return true;
    }

    auto reportErrorFunc =
        [this, &valueStr]()
        {
            logError() << ParseXmlWrap::logPrefix(getNode()) << "The default value of the \"" << name() <<
                          "\" is not within type boundaries (" << valueStr << ").";
        };

    auto reportWarningFunc =
        [this, &valueStr]()
        {
            logWarning() << ParseXmlWrap::logPrefix(getNode()) << "The default value of the \"" << name() <<
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
    if (!strToValue(valueStr, val)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) << "Default value (" << valueStr <<
                      ") cannot be recognized.";
        return false;
    }

    if (ParseIntFieldImpl::isBigUnsigned(m_state.m_type)) {
        return checkValueFunc(static_cast<std::uintmax_t>(val));
    }

    return checkValueFunc(val);
}

bool ParseEnumFieldImpl::updateHexAssign()
{
    if (!validateAndUpdateBoolPropValue(common::hexAssignStr(), m_state.m_hexAssign)) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::hexAssignStr());
    if (valueStr.empty()) {
        return true;
    }    

    if (!ParseIntFieldImpl::isTypeUnsigned(m_state.m_type)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot set \"" << common::hexAssignStr() << "\" property with signed types.";
        return false;
    }

    return true;
}

bool ParseEnumFieldImpl::updateAvailableLengthLimit()
{
    return validateAndUpdateBoolPropValue(common::availableLengthLimitStr(), m_state.m_availableLengthLimit);
}

bool ParseEnumFieldImpl::strToValue(
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
        if (!protocol().strToNumeric(str, false, val, bigUnsigned)) {
            return false;
        }

        if ((!bigUnsigned) && (val < 0) && (ParseIntFieldImpl::isUnsigned(m_state.m_type))) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                "Cannot assign negative value (" << val << " references as " <<
                str << ") to field with positive type.";

            return false;
        }

        if (bigUnsigned && (!ParseIntFieldImpl::isBigUnsigned(m_state.m_type))) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                "Cannot assign such big positive number (" <<
                static_cast<std::uintmax_t>(val) << " referenced as " <<
                str << ").";

            return false;
        }
        return true;
    }


    bool ok = false;
    if (ParseIntFieldImpl::isBigUnsigned(m_state.m_type)) {
        val = static_cast<std::intmax_t>(common::strToUintMax(str, &ok));
    }
    else {
        val = common::strToIntMax(str, &ok);
    }
    return ok;
}


} // namespace parse

} // namespace commsdsl
