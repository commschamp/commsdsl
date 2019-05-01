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

#include "EnumFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "util.h"

namespace commsdsl
{

namespace
{

} // namespace

EnumFieldImpl::EnumFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

bool EnumFieldImpl::isUnique() const
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

FieldImpl::Kind EnumFieldImpl::kindImpl() const
{
    return Kind::Enum;
}

EnumFieldImpl::EnumFieldImpl(const EnumFieldImpl&) = default;

FieldImpl::Ptr EnumFieldImpl::cloneImpl() const
{
    return Ptr(new EnumFieldImpl(*this));
}

const XmlWrap::NamesList& EnumFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::typeStr(),
        common::defaultValueStr(),
        common::endianStr(),
        common::lengthStr(),
        common::bitLengthStr(),
        common::nonUniqueAllowedStr(),
        common::validCheckVersionStr(),
        common::hexAssignStr()
    };

    return List;
}

const XmlWrap::NamesList& EnumFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::validValueStr()
    };

    return List;
}

bool EnumFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const EnumFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool EnumFieldImpl::parseImpl()
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
        updateHexAssign();
}

std::size_t EnumFieldImpl::minLengthImpl() const
{
    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        return 1U;
    }

    return m_state.m_length;
}

std::size_t EnumFieldImpl::maxLengthImpl() const
{
    return m_state.m_length;
}

std::size_t EnumFieldImpl::bitLengthImpl() const
{
    if (isBitfieldMember()) {
        return m_state.m_bitLength;
    }
    return Base::bitLengthImpl();
}

bool EnumFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    std::intmax_t value = 0;
    return strToNumeric(val, value);
}

bool EnumFieldImpl::isComparableToFieldImpl(const FieldImpl& field) const
{
    auto fieldKind = field.kind();
    return ((fieldKind == Kind::Int) || (fieldKind == Kind::Enum));
}

bool EnumFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    auto iter = m_state.m_values.find(ref);
    if (iter == m_state.m_values.end()) {
        return false;
    }

    static const std::uintmax_t BigUnsignedThreshold =
         static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());
    val = iter->second.m_value;
    isBigUnsigned =
        IntFieldImpl::isBigUnsigned(m_state.m_type) &&
        (BigUnsignedThreshold < static_cast<std::uintmax_t>(val));
    return true;
}

bool EnumFieldImpl::updateType()
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

    auto newType = IntFieldImpl::parseTypeValue(propsIter->second);
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

    logError() << XmlWrap::logPrefix(getNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool EnumFieldImpl::updateEndian()
{
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(props(), common::endianStr());
    if ((endianStr.empty()) && (m_state.m_endian != Endian_NumOfValues)) {
        return true;
    }

    m_state.m_endian = common::parseEndian(endianStr, protocol().schemaImpl().endian());
    if (m_state.m_endian == Endian_NumOfValues) {
        reportUnexpectedPropertyValue(common::endianStr(), endianStr);
        return false;
    }
    return true;
}

bool EnumFieldImpl::updateLength()
{
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto maxLength = IntFieldImpl::maxTypeLength(m_state.m_type);
    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    if (lengthStr.empty()) {
        if (m_state.m_length == 0) {
            m_state.m_length = maxLength;
            return true;
        }

        assert(m_state.m_length <= IntFieldImpl::maxTypeLength(m_state.m_type));
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
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Length cannot be changed after reuse";
        return false;
    }

    m_state.m_length = newLength;

    assert(0U < maxLength);
    if (maxLength < m_state.m_length) {
        logError() << XmlWrap::logPrefix(getNode()) << "Length of the \"" << name() << "\" element (" << lengthStr << ") cannot exceed "
                      "max length allowed by the type (" << maxLength << ").";
        return false;
    }

    assert (m_state.m_length != 0U);
    return true;
}

bool EnumFieldImpl::updateBitLength()
{
    if (!validateSinglePropInstance(common::bitLengthStr())) {
        return false;
    }

    static const std::size_t BitsInByte =
         std::numeric_limits<std::uint8_t>::digits;
    static_assert(BitsInByte == 8U, "Invalid assumption");

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
        logWarning() << XmlWrap::logPrefix((getNode())) <<
                        "The property \"" << common::bitLengthStr() << "\" is "
                        "applicable only to the members of \"" << common::bitfieldStr() << "\"";
        m_state.m_bitLength = maxBitLength;
        return true;
    }

    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        logError() << XmlWrap::logPrefix((getNode())) <<
                      "Bitfield member cannot have variable length type.";
        return false;
    }

    bool ok = false;
    m_state.m_bitLength = common::strToUnsigned(valStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::bitLengthStr(), valStr);
        return false;
    }

    if (maxBitLength < m_state.m_bitLength) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Value of property \"" << common::bitLengthStr() << "\" exceeds "
                      "maximal length available by the type and/or forced serialisation length.";
        return false;
    }

    return true;
}

bool EnumFieldImpl::updateNonUniqueAllowed()
{
    return validateAndUpdateBoolPropValue(common::nonUniqueAllowedStr(), m_state.m_nonUniqueAllowed);
}

bool EnumFieldImpl::updateValidCheckVersion()
{
    return validateAndUpdateBoolPropValue(common::validCheckVersionStr(), m_state.m_validCheckVersion);
}

bool EnumFieldImpl::updateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = IntFieldImpl::minTypeValue(m_state.m_type);
    m_state.m_typeAllowedMaxValue = IntFieldImpl::maxTypeValue(m_state.m_type);

    m_state.m_minValue = IntFieldImpl::calcMinValue(m_state.m_type, m_state.m_bitLength);
    m_state.m_maxValue = IntFieldImpl::calcMaxValue(m_state.m_type, m_state.m_bitLength);

    return true;
}

bool EnumFieldImpl::updateValues()
{
    auto validValues = XmlWrap::getChildren(getNode(), common::validValueStr());
    if (validValues.empty()) {
        if (!m_state.m_values.empty()) {
            assert(!m_state.m_revValues.empty());
            return true; // already has values
        }

        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The enum \"" << name() << "\" doesn't list any valid value.";
        return false;
    }

    for (auto* vNode : validValues) {
        static const XmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::valStr(),
            common::sinceVersionStr(),
            common::deprecatedStr(),
            common::descriptionStr(),
            common::displayNameStr()
        };

        auto props = XmlWrap::parseNodeProps(vNode);
        if (!XmlWrap::parseChildrenAsProps(vNode, PropNames, protocol().logger(), props)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::nameStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::valStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::sinceVersionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::deprecatedStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::descriptionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::displayNameStr(), protocol().logger())) {
            return false;
        }

        auto extraAttr = XmlWrap::getExtraAttributes(vNode, PropNames, protocol());
        static_cast<void>(extraAttr);
        auto extraChildren = XmlWrap::getExtraChildren(vNode, PropNames, protocol());
        static_cast<void>(extraChildren);

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << XmlWrap::logPrefix(vNode) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto valuesIter = m_state.m_values.find(nameIter->second);
        if (valuesIter != m_state.m_values.end()) {
            logError() << XmlWrap::logPrefix(vNode) << "Value with name \"" << nameIter->second <<
                          "\" has already been defined for enum \"" << name() << "\".";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        std::intmax_t val = 0;
        if (!strToNumeric(valIter->second, val)) {
            logError() << XmlWrap::logPrefix(vNode) << "Value of \"" << nameIter->second <<
                          "\" cannot be recognized.";
            return false;
        }

        auto checkValueInRangeFunc =
            [this, vNode, &nameIter](auto v) -> bool
            {
                if ((v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) ||
                    (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v)) {
                    this->logError() << XmlWrap::logPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of available values within a type.";
                    return false;
                }

                if ((v < static_cast<decltype(v)>(m_state.m_minValue)) ||
                    (static_cast<decltype(v)>(m_state.m_maxValue) < v)) {
                    this->logWarning() << XmlWrap::logPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of correctly serializable values.";
                }

                return true;
            };

        bool checkResult = false;
        if (IntFieldImpl::isBigUnsigned(m_state.m_type)) {
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
                logError() << XmlWrap::logPrefix(vNode) <<
                              "Value \"" << valIter->second << "\" has been already defined "
                              "as \"" << revIter->second << "\".";
                return false;
            }
        }

        ValueInfo info;
        info.m_value = val;
        info.m_sinceVersion = getSinceVersion();
        info.m_deprecatedSince = getDeprecated();

        if (!XmlWrap::getAndCheckVersions(vNode, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
            return false;
        }

        auto descIter = props.find(common::descriptionStr());
        if (descIter != props.end()) {
            info.m_description = descIter->second;
        }

        auto dispNameIter = props.find(common::displayNameStr());
        if (dispNameIter != props.end()) {
            info.m_displayName = dispNameIter->second;
        }

        m_state.m_values.emplace(nameIter->second, info);
        m_state.m_revValues.emplace(val, nameIter->second);
    }
    return true;
}

bool EnumFieldImpl::updateDefaultValue()
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
            logError() << XmlWrap::logPrefix(getNode()) << "The default value of the \"" << name() <<
                          "\" is not within type boundaries (" << valueStr << ").";
        };

    auto reportWarningFunc =
        [this, &valueStr]()
        {
            logWarning() << XmlWrap::logPrefix(getNode()) << "The default value of the \"" << name() <<
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
    if (!strToNumeric(valueStr, val)) {
        logError() << XmlWrap::logPrefix(getNode()) << "Default value (" << valueStr <<
                      ") cannot be recognized.";
        return false;
    }

    if (IntFieldImpl::isBigUnsigned(m_state.m_type)) {
        return checkValueFunc(static_cast<std::uintmax_t>(val));
    }

    return checkValueFunc(val);
}

bool EnumFieldImpl::updateHexAssign()
{
    if (!validateAndUpdateBoolPropValue(common::hexAssignStr(), m_state.m_hexAssign)) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::hexAssignStr());
    if (valueStr.empty()) {
        return true;
    }    

    if (!IntFieldImpl::isTypeUnsigned(m_state.m_type)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot set \"" << common::hexAssignStr() << "\" property with signed types.";
        return false;
    }

    return true;
}

bool EnumFieldImpl::strToNumeric(
    const std::string& str,
    std::intmax_t& val) const
{
    if (common::isValidName(str)) {
        // Check among specials
        auto iter = m_state.m_values.find(str);
        if (iter == m_state.m_values.end()) {
            return false;
        }

        val = iter->second.m_value;
        return true;
    }

   if (common::isValidRefName(str)) {

        bool bigUnsigned = false;
        if (!protocol().strToNumeric(str, false, val, bigUnsigned)) {
            return false;
        }

        if ((!bigUnsigned) && (val < 0) && (IntFieldImpl::isUnsigned(m_state.m_type))) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Cannot assign negative value to enum with positive type";
            return false;
        }

        if (bigUnsigned && (!IntFieldImpl::isBigUnsigned(m_state.m_type))) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Cannot assign such big positive number.";
            return false;

        }
        return true;
    }

    bool ok = false;
    if (IntFieldImpl::isBigUnsigned(m_state.m_type)) {
        val = static_cast<std::intmax_t>(common::strToUintMax(str, &ok));
    }
    else {
        val = common::strToIntMax(str, &ok);
    }
    return ok;
}


} // namespace commsdsl
