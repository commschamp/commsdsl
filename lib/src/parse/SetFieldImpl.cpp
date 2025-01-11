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

#include "SetFieldImpl.h"

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

namespace parse
{

namespace
{
const std::size_t BitsInByte =
        std::numeric_limits<std::uint8_t>::digits;
static_assert(BitsInByte == 8U, "Invalid assumption");    

} // namespace 

SetFieldImpl::SetFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

SetFieldImpl::SetFieldImpl(const SetFieldImpl&) = default;

bool SetFieldImpl::isUnique() const
{
    unsigned prevIdx = 0;
    bool firstElem = true;
    for (auto& b : m_state.m_revBits) {
        if (firstElem) {
            prevIdx = b.first;
            firstElem = false;
            continue;
        }

        if (prevIdx == b.first) {
            assert(isNonUniqueAllowed());
            return false;
        }

        prevIdx = b.first;
    }

    return true;
}

FieldImpl::Kind SetFieldImpl::kindImpl() const
{
    return Kind::Set;
}

FieldImpl::Ptr SetFieldImpl::cloneImpl() const
{
    return Ptr(new SetFieldImpl(*this));
}

const XmlWrap::NamesList& SetFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::defaultValueStr(),
        common::endianStr(),
        common::lengthStr(),
        common::bitLengthStr(),
        common::reservedValueStr(),
        common::validCheckVersionStr(),
        common::typeStr(),
        common::nonUniqueAllowedStr(),
        common::availableLengthLimitStr(),
    };

    return List;
}

const XmlWrap::NamesList& SetFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::bitStr()
    };

    return List;
}

bool SetFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const SetFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool SetFieldImpl::parseImpl()
{
    return
        updateEndian() &&
        updateType() &&
        updateLength() &&
        updateNonUniqueAllowed() &&
        updateValidCheckVersion() &&
        updateDefaultValue() &&
        updateReservedValue() &&
        updateAvailableLengthLimit() &&
        updateBits();
}

std::size_t SetFieldImpl::minLengthImpl() const
{
    return m_state.m_length;
}

std::size_t SetFieldImpl::bitLengthImpl() const
{
    if (isBitfieldMember()) {
        return m_state.m_bitLength;
    }

    return Base::bitLengthImpl();
}

bool SetFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    if (common::isValidRefName(val)) {
        bool bigUnsigned = false;
        std::intmax_t valTmp = 0;
        if (!protocol().strToNumeric(val, false, valTmp, bigUnsigned)) {
            return false;
        }

        if ((!bigUnsigned) && (valTmp < 0)) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Cannot compare negative value (" << valTmp << " referenced as " <<
            val << ").";
            return false;
        }

        return true;
    }

    bool ok = false;
    auto valTmp = common::strToIntMax(val, &ok);
    if (ok && (valTmp < 0)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot compare to negative number " << valTmp << ".";        
        return false;
    }

    return ok;    
}

bool SetFieldImpl::isComparableToFieldImpl(const FieldImpl& field) const
{
    auto fieldKind = field.kind();
    return (fieldKind == Kind::Set);    
}

bool SetFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (!protocol().isFieldValueReferenceSupported()) {
        return false;
    }

    isBigUnsigned = false;
    if (ref.empty()) {
        val = static_cast<std::intmax_t>(m_state.m_defaultBitValue);
        return true;
    }

    auto iter = m_state.m_bits.find(ref);
    if (iter == m_state.m_bits.end()) {
        return false;
    }

    val = static_cast<std::intmax_t>(iter->second.m_defaultValue);
    return true;
}

bool SetFieldImpl::strToBoolImpl(const std::string& ref, bool& val) const
{
    if (!protocol().isFieldValueReferenceSupported()) {
        return false;
    }

    if (ref.empty()) {
        val = m_state.m_defaultBitValue;
        return true;
    }

    auto iter = m_state.m_bits.find(ref);
    if (iter == m_state.m_bits.end()) {
        return false;
    }

    val = iter->second.m_defaultValue;
    return true;
}

bool SetFieldImpl::validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const
{
    assert(0U < m_state.m_length);
    auto maxBitLength = m_state.m_length * BitsInByte;
    if (maxBitLength < bitLength) {
        logError() << XmlWrap::logPrefix(node) <<
                      "Value of property \"" << common::bitLengthStr() << "\" exceeds "
                      "maximal length available by the type and/or forced serialisation length.";
        return false;
    }

    return true;
}

SetFieldImpl::FieldRefInfo SetFieldImpl::processInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());

    FieldRefInfo info;
    auto iter = m_state.m_bits.find(refStr);
    if (iter != m_state.m_bits.end()) {
        info.m_field = this;
        info.m_valueName = refStr;
        info.m_refType = FieldRefType_InnerValue;
    }

    return info;    
}

bool SetFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool SetFieldImpl::updateEndian()
{
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(props(), common::endianStr());
    if ((endianStr.empty()) && (m_state.m_endian != Endian_NumOfValues)) {
        return true;
    }

    m_state.m_endian = common::parseEndian(endianStr, protocol().currSchema().endian());
    if (m_state.m_endian == Endian_NumOfValues) {
        reportUnexpectedPropertyValue(common::endianStr(), endianStr);
        return false;
    }
    return true;
}

bool SetFieldImpl::updateType()
{
    if (!validateSinglePropInstance(common::typeStr())) {
        return false;
    }

    auto typeIter = props().find(common::typeStr());
    if (typeIter == props().end()) {
        return true;
    }

    auto typeVal = IntFieldImpl::parseTypeValue(typeIter->second);
    if ((Type::NumOfValues <= typeVal) ||
        (!IntFieldImpl::isTypeUnsigned(typeVal))) {
        reportUnexpectedPropertyValue(common::typeStr(), typeIter->second);
        return false;
    }

    if (m_state.m_type == typeVal) {
        return true;
    }

    if (m_state.m_type != Type::NumOfValues) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Type cannot be changed after reuse";
        return false;
    }

    m_state.m_type = typeVal;
    return true;
}

bool SetFieldImpl::updateLength()
{
    bool mustHaveLength =
            (m_state.m_type == Type::NumOfValues) &&
            (m_state.m_length == 0U) &&
            (!isBitfieldMember());
    if (!validateSinglePropInstance(common::lengthStr(), mustHaveLength)) {
        return false;
    }

    std::size_t maxLength = sizeof(std::uint64_t);
    if (m_state.m_type != Type::NumOfValues) {
        maxLength = IntFieldImpl::maxTypeLength(m_state.m_type);
    }

    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    do {
        if (lengthStr.empty()) {
            if ((m_state.m_length == 0U) && (m_state.m_type != Type::NumOfValues)) {
                m_state.m_length = maxLength;
            }
            break;
        }

        bool ok = false;
        auto newLen = static_cast<decltype(m_state.m_length)>(common::strToUintMax(lengthStr, &ok));

        if ((!ok) || (newLen == 0U) || (maxLength < newLen)) {
            reportUnexpectedPropertyValue(common::lengthStr(), lengthStr);
            return false;
        }

        if (m_state.m_length == newLen) {
            break;
        }

        if (m_state.m_length != 0U) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "Length cannot be changed after reuse";
            return false;
        }

        m_state.m_length = newLen;
    } while (false);

    bool mustHaveBitLength = (isBitfieldMember() && m_state.m_bitLength == 0U && (m_state.m_length == 0U));
    if (!validateSinglePropInstance(common::bitLengthStr(), mustHaveBitLength)) {
        return false;
    }

    auto& bitLengthStr = common::getStringProp(props(), common::bitLengthStr());
    do {
        if (bitLengthStr.empty()) {
            if (m_state.m_bitLength == 0U) {
                assert(m_state.m_length != 0U);
                m_state.m_bitLength = m_state.m_length * 8U;
            }
            break;
        }

        if (!isBitfieldMember()) {
            logWarning() << XmlWrap::logPrefix((getNode())) <<
                            "The property \"" << common::bitLengthStr() << "\" is "
                            "applicable only to the members of \"" << common::bitfieldStr() << "\"";
            assert(m_state.m_length != 0U);
            m_state.m_bitLength = m_state.m_length * 8U;
            break;
        }

        bool ok = false;
        m_state.m_bitLength = static_cast<decltype(m_state.m_bitLength)>(common::strToUintMax(bitLengthStr, &ok));
        if ((!ok) || (m_state.m_bitLength == 0U)) {
            reportUnexpectedPropertyValue(common::bitLengthStr(), bitLengthStr);
            return false;
        }

        if ((m_state.m_length != 0) && ((m_state.m_length * 8U) < m_state.m_bitLength)) {
            reportUnexpectedPropertyValue(common::bitLengthStr(), bitLengthStr);
            return false;
        }

    } while (false);

    if ((m_state.m_length == 0U) && (m_state.m_bitLength != 0U)) {
        m_state.m_length = ((m_state.m_bitLength - 1U) / 8U) + 1U;
    }

    if (m_state.m_type == Type::NumOfValues) {
        assert(m_state.m_length != 0U);

        static const Type Map[] {
            Type::Uint8,
            Type::Uint16,
            Type::Uint32,
            Type::Uint32,
            Type::Uint64,
            Type::Uint64,
            Type::Uint64,
            Type::Uint64,
        };

        static const std::size_t MapSize = std::extent<decltype(Map)>::value;
        static_assert(MapSize == sizeof(std::uint64_t), "Invalid map");

        assert(m_state.m_length <= MapSize);
        m_state.m_type = Map[m_state.m_length - 1];
    }

    assert(m_state.m_type != Type::NumOfValues);
    assert(m_state.m_length != 0U);
    assert(m_state.m_bitLength != 0U);
    return true;
}

bool SetFieldImpl::updateNonUniqueAllowed()
{
    bool wasAllowed = m_state.m_nonUniqueAllowed;
    bool newAllowed = false;
    if (!validateAndUpdateBoolPropValue(common::nonUniqueAllowedStr(), newAllowed)) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::nonUniqueAllowedStr());
    if (valueStr.empty()) {
        return true;
    }
    
    if (wasAllowed && (!newAllowed) && (!isUnique())) {
        logError() << "Cannot clear \"" << common::nonUniqueAllowedStr() << "\" property value "
                      "while having multiple names for the same bit(s).";
        return false;
    }

    m_state.m_nonUniqueAllowed = newAllowed;
    return true;
}

bool SetFieldImpl::updateValidCheckVersion()
{
    return validateAndUpdateBoolPropValue(common::validCheckVersionStr(), m_state.m_validCheckVersion);
}

bool SetFieldImpl::updateDefaultValue()
{
    auto& propName = common::defaultValueStr();
    auto iter = props().find(propName);
    if (iter == props().end()) {
        return true;
    }

    if (!strToValue(iter->second, m_state.m_defaultBitValue)) {
        reportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

bool SetFieldImpl::updateReservedValue()
{
    auto& propName = common::reservedValueStr();
    auto iter = props().find(propName);
    if (iter == props().end()) {
        return true;
    }

    if (!strToValue(iter->second, m_state.m_reservedBitValue)) {
        reportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

bool SetFieldImpl::updateAvailableLengthLimit()
{
    return validateAndUpdateBoolPropValue(common::availableLengthLimitStr(), m_state.m_availableLengthLimit);
}

bool SetFieldImpl::updateBits()
{
    auto bits = XmlWrap::getChildren(getNode(), common::bitStr());

    for (auto* b : bits) {
        static const XmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::idxStr(),
            common::defaultValueStr(),
            common::reservedValueStr(),
            common::reservedStr(),
            common::sinceVersionStr(),
            common::deprecatedStr(),
            common::descriptionStr(),
            common::displayNameStr()
        };

        auto props = XmlWrap::parseNodeProps(b);
        if (!XmlWrap::parseChildrenAsProps(b, PropNames, protocol().logger(), props)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::nameStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::idxStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::defaultValueStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::reservedValueStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::reservedStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::sinceVersionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::deprecatedStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::descriptionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::displayNameStr(), protocol().logger())) {
            return false;
        }

        [[maybe_unused]] auto extraAttr = XmlWrap::getExtraAttributes(b, PropNames, protocol());
        [[maybe_unused]] auto extraChildren = XmlWrap::getExtraChildren(b, PropNames, protocol());

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << XmlWrap::logPrefix(b) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto bitsIter = m_state.m_bits.find(nameIter->second);
        if (bitsIter != m_state.m_bits.end()) {
            logError() << XmlWrap::logPrefix(b) << "Bit with name \"" << nameIter->second <<
                          "\" has already been defined for set \"" << name() << "\".";
            return false;
        }

        auto idxIter = props.find(common::idxStr());
        assert(idxIter != props.end());

        bool ok = false;
        unsigned idx = common::strToUnsigned(idxIter->second, &ok);
        if (!ok) {
            XmlWrap::XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::idxStr(), idxIter->second, protocol().logger());
            return false;
        }

        if (m_state.m_bitLength <= idx) {
            logError() << XmlWrap::logPrefix(b) <<
                          "Index of the bit (" << idx << ") must be less than number of available bits (" <<
                          m_state.m_bitLength << ").";
            return false;
        }
        if (!m_state.m_nonUniqueAllowed) {
            auto revBitsIter = m_state.m_revBits.find(idx);
            if (revBitsIter != m_state.m_revBits.end()) {
                logError() << XmlWrap::logPrefix(b) <<
                      "Bit \"" << revBitsIter->first << "\" has been already defined "
                      "as \"" << revBitsIter->second << "\".";
                return false;
            }
        }

        BitInfo info;
        info.m_idx = idx;
        info.m_defaultValue = m_state.m_defaultBitValue;
        info.m_reservedValue = m_state.m_reservedBitValue;
        info.m_sinceVersion = getSinceVersion();
        info.m_deprecatedSince = getDeprecated();
        do {
            auto& bitDefaultValueStr = common::getStringProp(props, common::defaultValueStr());
            if (bitDefaultValueStr.empty()) {
                break;
            }

            if (!strToValue(bitDefaultValueStr, info.m_defaultValue)) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::defaultValueStr(), bitDefaultValueStr, protocol().logger());
                return false;
            }

        } while (false);

        do{
            auto& bitReservedStr = common::getStringProp(props, common::reservedStr());
            if (bitReservedStr.empty()) {
                break;
            }

            if (!strToValue(bitReservedStr, info.m_reserved)) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::reservedStr(), bitReservedStr, protocol().logger());
                return false;
            }

        } while (false);


        do {
            auto& bitReservedValueStr = common::getStringProp(props, common::reservedValueStr());
            if (bitReservedValueStr.empty()) {
                break;
            }

            if (!strToValue(bitReservedValueStr, info.m_reservedValue)) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::reservedValueStr(), bitReservedValueStr, protocol().logger());
                return false;
            }
        } while (false);

        if (!XmlWrap::getAndCheckVersions(b, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
            return false;
        }

        // Make sure that reserved bits dont update version
        do {
            if (!info.m_reserved) {
                break;
            }

            if ((getSinceVersion() < info.m_sinceVersion) ||
                (info.m_deprecatedSince < getDeprecated())) {
                logError() << XmlWrap::logPrefix(b) <<
                    "Cannot modify version information on explicity reserved bits.";
                return false;
            }
        } while (false);

        // Check consistency with previous definitions
        do {
            if (!m_state.m_nonUniqueAllowed) {
                // The bit hasn't been processed earlier
                assert(m_state.m_revBits.find(idx) == m_state.m_revBits.end());
                break;
            }

            auto revIters = m_state.m_revBits.equal_range(idx);
            if (revIters.first == revIters.second) {
                // The bit hasn't been processed earlier
                break;
            }

            for (auto rIter = revIters.first; rIter != revIters.second; ++rIter) {
                auto iter = m_state.m_bits.find(rIter->second);
                assert(iter != m_state.m_bits.end());

                if (iter->second.m_deprecatedSince <= info.m_sinceVersion) {
                    assert(iter->second.m_sinceVersion < info.m_sinceVersion);
                    continue;
                }

                if (info.m_deprecatedSince <= iter->second.m_sinceVersion) {
                    assert(info.m_sinceVersion < iter->second.m_sinceVersion);
                    continue;
                }

                if (info.m_defaultValue != iter->second.m_defaultValue) {
                    logError() << XmlWrap::logPrefix(b) <<
                          "Inconsistent value of \"" << common::defaultValueStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }

                if (info.m_reserved != iter->second.m_reserved) {
                    logError() << XmlWrap::logPrefix(b) <<
                          "Inconsistent value of \"" << common::reservedStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }

                if (info.m_reservedValue != iter->second.m_reservedValue) {
                    logError() << XmlWrap::logPrefix(b) <<
                          "Inconsistent value of \"" << common::reservedValueStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }
            }
        } while(false);

        auto descIter = props.find(common::descriptionStr());
        if (descIter != props.end()) {
            info.m_description = descIter->second;
        }

        auto dispNameIter = props.find(common::displayNameStr());
        if ((dispNameIter != props.end()) &&
            (!protocol().strToStringValue(dispNameIter->second, info.m_displayName))) {
            XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::displayNameStr(), dispNameIter->second, protocol().logger());
            return false;
        }

        m_state.m_bits.emplace(nameIter->second, info);
        m_state.m_revBits.emplace(idx, nameIter->second);
    }

    return true;
}

bool SetFieldImpl::strToValue(const std::string& str, bool& val) const
{
    bool ok = false;
    val = common::strToBool(str, &ok);
    if (ok) {
        return true;
    }

    if (!protocol().isFieldValueReferenceSupported()) {
        return false;
    }

    return protocol().strToBool(str, true, val);
}

} // namespace parse

} // namespace commsdsl
