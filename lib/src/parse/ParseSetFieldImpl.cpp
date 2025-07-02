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

#include "ParseSetFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "parse_common.h"
#include "ParseProtocolImpl.h"
#include "ParseIntFieldImpl.h"
#include "parse_util.h"

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

ParseSetFieldImpl::ParseSetFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

ParseSetFieldImpl::ParseSetFieldImpl(const ParseSetFieldImpl&) = default;

bool ParseSetFieldImpl::parseIsUnique() const
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
            assert(parseIsNonUniqueAllowed());
            return false;
        }

        prevIdx = b.first;
    }

    return true;
}

ParseFieldImpl::Kind ParseSetFieldImpl::parseKindImpl() const
{
    return Kind::Set;
}

ParseFieldImpl::Ptr ParseSetFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseSetFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseSetFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::parseDefaultValueStr(),
        common::parseEndianStr(),
        common::parseLengthStr(),
        common::parseBitLengthStr(),
        common::parseReservedValueStr(),
        common::parseValidCheckVersionStr(),
        common::parseTypeStr(),
        common::parseNonUniqueAllowedStr(),
        common::parseAvailableLengthLimitStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseSetFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::parseBitStr()
    };

    return List;
}

bool ParseSetFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseSetFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool ParseSetFieldImpl::parseImpl()
{
    return
        parseUpdateEndian() &&
        parseUpdateType() &&
        parseUpdateLength() &&
        parseUpdateNonUniqueAllowed() &&
        parseUpdateValidCheckVersion() &&
        parseUpdateDefaultValue() &&
        parseUpdateReservedValue() &&
        parseUpdateAvailableLengthLimit() &&
        parseUpdateBits();
}

std::size_t ParseSetFieldImpl::parseMinLengthImpl() const
{
    return m_state.m_length;
}

std::size_t ParseSetFieldImpl::parseBitLengthImpl() const
{
    if (parseIsBitfieldMember()) {
        return m_state.m_bitLength;
    }

    return Base::parseBitLengthImpl();
}

bool ParseSetFieldImpl::parseIsComparableToValueImpl(const std::string& val) const
{
    if (common::parseIsValidRefName(val)) {
        bool bigUnsigned = false;
        std::intmax_t valTmp = 0;
        if (!parseProtocol().parseStrToNumeric(val, false, valTmp, bigUnsigned)) {
            return false;
        }

        if ((!bigUnsigned) && (valTmp < 0)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Cannot compare negative value (" << valTmp << " referenced as " <<
            val << ").";
            return false;
        }

        return true;
    }

    bool ok = false;
    auto valTmp = common::parseStrToIntMax(val, &ok);
    if (ok && (valTmp < 0)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot compare to negative number " << valTmp << ".";        
        return false;
    }

    return ok;    
}

bool ParseSetFieldImpl::parseIsComparableToFieldImpl(const ParseFieldImpl& field) const
{
    auto fieldKind = field.parseKind();
    return (fieldKind == Kind::Set);    
}

bool ParseSetFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
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

bool ParseSetFieldImpl::parseStrToBoolImpl(const std::string& ref, bool& val) const
{
    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
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

bool ParseSetFieldImpl::parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const
{
    assert(0U < m_state.m_length);
    auto maxBitLength = m_state.m_length * BitsInByte;
    if (maxBitLength < bitLength) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
                      "Value of property \"" << common::parseBitLengthStr() << "\" exceeds "
                      "maximal length available by the type and/or forced serialisation length.";
        return false;
    }

    return true;
}

ParseSetFieldImpl::FieldRefInfo ParseSetFieldImpl::parseProcessInnerRefImpl(const std::string& refStr) const
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

bool ParseSetFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool ParseSetFieldImpl::parseUpdateEndian()
{
    if (!parseValidateSinglePropInstance(common::parseEndianStr())) {
        return false;
    }

    auto& endianStr = common::parseGetStringProp(parseProps(), common::parseEndianStr());
    if ((endianStr.empty()) && (m_state.m_endian != ParseEndian_NumOfValues)) {
        return true;
    }

    m_state.m_endian = common::parseEndian(endianStr, parseProtocol().parseCurrSchema().parseEndian());
    if (m_state.m_endian == ParseEndian_NumOfValues) {
        parseReportUnexpectedPropertyValue(common::parseEndianStr(), endianStr);
        return false;
    }
    return true;
}

bool ParseSetFieldImpl::parseUpdateType()
{
    if (!parseValidateSinglePropInstance(common::parseTypeStr())) {
        return false;
    }

    auto typeIter = parseProps().find(common::parseTypeStr());
    if (typeIter == parseProps().end()) {
        return true;
    }

    auto typeVal = ParseIntFieldImpl::parseTypeValue(typeIter->second);
    if ((Type::NumOfValues <= typeVal) ||
        (!ParseIntFieldImpl::parseIsTypeUnsigned(typeVal))) {
        parseReportUnexpectedPropertyValue(common::parseTypeStr(), typeIter->second);
        return false;
    }

    if (m_state.m_type == typeVal) {
        return true;
    }

    if (m_state.m_type != Type::NumOfValues) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Type cannot be changed after reuse";
        return false;
    }

    m_state.m_type = typeVal;
    return true;
}

bool ParseSetFieldImpl::parseUpdateLength()
{
    bool mustHaveLength =
            (m_state.m_type == Type::NumOfValues) &&
            (m_state.m_length == 0U) &&
            (!parseIsBitfieldMember());
    if (!parseValidateSinglePropInstance(common::parseLengthStr(), mustHaveLength)) {
        return false;
    }

    std::size_t maxLength = sizeof(std::uint64_t);
    if (m_state.m_type != Type::NumOfValues) {
        maxLength = ParseIntFieldImpl::parseMaxTypeLength(m_state.m_type);
    }

    auto& lengthStr = common::parseGetStringProp(parseProps(), common::parseLengthStr());
    do {
        if (lengthStr.empty()) {
            if ((m_state.m_length == 0U) && (m_state.m_type != Type::NumOfValues)) {
                m_state.m_length = maxLength;
            }
            break;
        }

        bool ok = false;
        auto newLen = static_cast<decltype(m_state.m_length)>(common::parseStrToUintMax(lengthStr, &ok));

        if ((!ok) || (newLen == 0U) || (maxLength < newLen)) {
            parseReportUnexpectedPropertyValue(common::parseLengthStr(), lengthStr);
            return false;
        }

        if (m_state.m_length == newLen) {
            break;
        }

        if (m_state.m_length != 0U) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "Length cannot be changed after reuse";
            return false;
        }

        m_state.m_length = newLen;
    } while (false);

    bool mustHaveBitLength = (parseIsBitfieldMember() && m_state.m_bitLength == 0U && (m_state.m_length == 0U));
    if (!parseValidateSinglePropInstance(common::parseBitLengthStr(), mustHaveBitLength)) {
        return false;
    }

    auto& bitLengthStr = common::parseGetStringProp(parseProps(), common::parseBitLengthStr());
    do {
        if (bitLengthStr.empty()) {
            if (m_state.m_bitLength == 0U) {
                assert(m_state.m_length != 0U);
                m_state.m_bitLength = m_state.m_length * 8U;
            }
            break;
        }

        if (!parseIsBitfieldMember()) {
            parseLogWarning() << ParseXmlWrap::parseLogPrefix((parseGetNode())) <<
                            "The property \"" << common::parseBitLengthStr() << "\" is "
                            "applicable only to the members of \"" << common::parseBitparseFieldStr() << "\"";
            assert(m_state.m_length != 0U);
            m_state.m_bitLength = m_state.m_length * 8U;
            break;
        }

        bool ok = false;
        m_state.m_bitLength = static_cast<decltype(m_state.m_bitLength)>(common::parseStrToUintMax(bitLengthStr, &ok));
        if ((!ok) || (m_state.m_bitLength == 0U)) {
            parseReportUnexpectedPropertyValue(common::parseBitLengthStr(), bitLengthStr);
            return false;
        }

        if ((m_state.m_length != 0) && ((m_state.m_length * 8U) < m_state.m_bitLength)) {
            parseReportUnexpectedPropertyValue(common::parseBitLengthStr(), bitLengthStr);
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

bool ParseSetFieldImpl::parseUpdateNonUniqueAllowed()
{
    bool wasAllowed = m_state.m_nonUniqueAllowed;
    bool newAllowed = false;
    if (!parseValidateAndUpdateBoolPropValue(common::parseNonUniqueAllowedStr(), newAllowed)) {
        return false;
    }

    auto& valueStr = common::parseGetStringProp(parseProps(), common::parseNonUniqueAllowedStr());
    if (valueStr.empty()) {
        return true;
    }
    
    if (wasAllowed && (!newAllowed) && (!parseIsUnique())) {
        parseLogError() << "Cannot clear \"" << common::parseNonUniqueAllowedStr() << "\" property value "
                      "while having multiple names for the same bit(s).";
        return false;
    }

    m_state.m_nonUniqueAllowed = newAllowed;
    return true;
}

bool ParseSetFieldImpl::parseUpdateValidCheckVersion()
{
    return parseValidateAndUpdateBoolPropValue(common::parseValidCheckVersionStr(), m_state.m_validCheckVersion);
}

bool ParseSetFieldImpl::parseUpdateDefaultValue()
{
    auto& propName = common::parseDefaultValueStr();
    auto iter = parseProps().find(propName);
    if (iter == parseProps().end()) {
        return true;
    }

    if (!parseStrToValue(iter->second, m_state.m_defaultBitValue)) {
        parseReportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

bool ParseSetFieldImpl::parseUpdateReservedValue()
{
    auto& propName = common::parseReservedValueStr();
    auto iter = parseProps().find(propName);
    if (iter == parseProps().end()) {
        return true;
    }

    if (!parseStrToValue(iter->second, m_state.m_reservedBitValue)) {
        parseReportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

bool ParseSetFieldImpl::parseUpdateAvailableLengthLimit()
{
    return parseValidateAndUpdateBoolPropValue(common::parseAvailableLengthLimitStr(), m_state.m_availableLengthLimit);
}

bool ParseSetFieldImpl::parseUpdateBits()
{
    auto bits = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseBitStr());

    for (auto* b : bits) {
        static const ParseXmlWrap::NamesList PropNames = {
            common::parseNameStr(),
            common::parseIdxStr(),
            common::parseDefaultValueStr(),
            common::parseReservedValueStr(),
            common::parseReservedStr(),
            common::parseSinceVersionStr(),
            common::parseDeprecatedStr(),
            common::parseDescriptionStr(),
            common::parseDisplayNameStr()
        };

        auto props = ParseXmlWrap::parseNodeProps(b);
        if (!ParseXmlWrap::parseChildrenAsProps(b, PropNames, parseProtocol().parseLogger(), props)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseNameStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseIdxStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseDefaultValueStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseReservedValueStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseReservedStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseSinceVersionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseDeprecatedStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseDescriptionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(b, props, common::parseDisplayNameStr(), parseProtocol().parseLogger())) {
            return false;
        }

        [[maybe_unused]] auto extraAttr = ParseXmlWrap::parseGetExtraAttributes(b, PropNames, parseProtocol());
        [[maybe_unused]] auto extraChildren = ParseXmlWrap::parseGetExtraChildren(b, PropNames, parseProtocol());

        auto nameIter = props.find(common::parseNameStr());
        assert(nameIter != props.end());

        if (!common::parseIsValidName(nameIter->second)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(b) <<
                  "Property \"" << common::parseNameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto bitsIter = m_state.m_bits.find(nameIter->second);
        if (bitsIter != m_state.m_bits.end()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(b) << "Bit with name \"" << nameIter->second <<
                          "\" has already been defined for set \"" << parseName() << "\".";
            return false;
        }

        auto idxIter = props.find(common::parseIdxStr());
        assert(idxIter != props.end());

        bool ok = false;
        unsigned idx = common::parseStrToUnsigned(idxIter->second, &ok);
        if (!ok) {
            ParseXmlWrap::ParseXmlWrap::parseReportUnexpectedPropertyValue(b, nameIter->second, common::parseIdxStr(), idxIter->second, parseProtocol().parseLogger());
            return false;
        }

        if (m_state.m_bitLength <= idx) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(b) <<
                          "Index of the bit (" << idx << ") must be less than number of available bits (" <<
                          m_state.m_bitLength << ").";
            return false;
        }
        if (!m_state.m_nonUniqueAllowed) {
            auto revBitsIter = m_state.m_revBits.find(idx);
            if (revBitsIter != m_state.m_revBits.end()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(b) <<
                      "Bit \"" << revBitsIter->first << "\" has been already defined "
                      "as \"" << revBitsIter->second << "\".";
                return false;
            }
        }

        BitInfo info;
        info.m_idx = idx;
        info.m_defaultValue = m_state.m_defaultBitValue;
        info.m_reservedValue = m_state.m_reservedBitValue;
        info.m_sinceVersion = parseGetSinceVersion();
        info.m_deprecatedSince = parseGetDeprecated();
        do {
            auto& bitDefaultValueStr = common::parseGetStringProp(props, common::parseDefaultValueStr());
            if (bitDefaultValueStr.empty()) {
                break;
            }

            if (!parseStrToValue(bitDefaultValueStr, info.m_defaultValue)) {
                ParseXmlWrap::parseReportUnexpectedPropertyValue(b, nameIter->second, common::parseDefaultValueStr(), bitDefaultValueStr, parseProtocol().parseLogger());
                return false;
            }

        } while (false);

        do{
            auto& bitReservedStr = common::parseGetStringProp(props, common::parseReservedStr());
            if (bitReservedStr.empty()) {
                break;
            }

            if (!parseStrToValue(bitReservedStr, info.m_reserved)) {
                ParseXmlWrap::parseReportUnexpectedPropertyValue(b, nameIter->second, common::parseReservedStr(), bitReservedStr, parseProtocol().parseLogger());
                return false;
            }

        } while (false);


        do {
            auto& bitReservedValueStr = common::parseGetStringProp(props, common::parseReservedValueStr());
            if (bitReservedValueStr.empty()) {
                break;
            }

            if (!parseStrToValue(bitReservedValueStr, info.m_reservedValue)) {
                ParseXmlWrap::parseReportUnexpectedPropertyValue(b, nameIter->second, common::parseReservedValueStr(), bitReservedValueStr, parseProtocol().parseLogger());
                return false;
            }
        } while (false);

        if (!ParseXmlWrap::parseGetAndCheckVersions(b, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
            return false;
        }

        // Make sure that reserved bits dont update version
        do {
            if (!info.m_reserved) {
                break;
            }

            if ((parseGetSinceVersion() < info.m_sinceVersion) ||
                (info.m_deprecatedSince < parseGetDeprecated())) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(b) <<
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
                    parseLogError() << ParseXmlWrap::parseLogPrefix(b) <<
                          "Inconsistent value of \"" << common::parseDefaultValueStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }

                if (info.m_reserved != iter->second.m_reserved) {
                    parseLogError() << ParseXmlWrap::parseLogPrefix(b) <<
                          "Inconsistent value of \"" << common::parseReservedStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }

                if (info.m_reservedValue != iter->second.m_reservedValue) {
                    parseLogError() << ParseXmlWrap::parseLogPrefix(b) <<
                          "Inconsistent value of \"" << common::parseReservedValueStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }
            }
        } while(false);

        auto descIter = props.find(common::parseDescriptionStr());
        if (descIter != props.end()) {
            info.m_description = descIter->second;
        }

        auto dispNameIter = props.find(common::parseDisplayNameStr());
        if ((dispNameIter != props.end()) &&
            (!parseProtocol().parseStrToStringValue(dispNameIter->second, info.m_displayName))) {
            ParseXmlWrap::parseReportUnexpectedPropertyValue(b, nameIter->second, common::parseDisplayNameStr(), dispNameIter->second, parseProtocol().parseLogger());
            return false;
        }

        m_state.m_bits.emplace(nameIter->second, info);
        m_state.m_revBits.emplace(idx, nameIter->second);
    }

    return true;
}

bool ParseSetFieldImpl::parseStrToValue(const std::string& str, bool& val) const
{
    bool ok = false;
    val = common::parseStrToBool(str, &ok);
    if (ok) {
        return true;
    }

    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
        return false;
    }

    return parseProtocol().parseStrToBool(str, true, val);
}

} // namespace parse

} // namespace commsdsl
