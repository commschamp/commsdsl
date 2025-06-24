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

#include "ParseIntFieldImpl.h"

#include "parse_common.h"
#include "ParseProtocolImpl.h"
#include "ParseRefFieldImpl.h"
#include "parse_util.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <map>
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

static_assert(
    static_cast<std::intmax_t>(std::numeric_limits<std::uintmax_t>::max()) == -1,
    "The code expects 2's compliment negative integers representation.");

static_assert(
    static_cast<std::uintmax_t>(std::intmax_t(-1)) == std::numeric_limits<std::uintmax_t>::max(),
    "The code expects 2's compliment negative integers representation.");


template <typename T>
constexpr std::intmax_t parseMinValueForType()
{
    return static_cast<std::intmax_t>(std::numeric_limits<T>::min());
}

template <typename T>
constexpr std::intmax_t parseMaxValueForType()
{
    return static_cast<std::intmax_t>(std::numeric_limits<T>::max());
}

template <typename T>
constexpr std::uintmax_t parseMaxValueForBigUnsignedType()
{
    return static_cast<std::uintmax_t>(std::numeric_limits<T>::max());
}

std::intmax_t parseCalcMaxFixedSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    if (64U <= bitsLen) {
        return std::numeric_limits<std::int64_t>::max();
    }

    auto result = (std::uintmax_t(1) << (bitsLen - 1)) - 1;
    return static_cast<std::intmax_t>(result);
}

std::intmax_t parseCalcMinFixedSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    std::uintmax_t result = ~(static_cast<std::uintmax_t>(parseCalcMaxFixedSignedValue(bitsLen)));
    return static_cast<std::intmax_t>(result);
}

std::uintmax_t parseCalcMaxFixedUnsignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    if (64U <= bitsLen) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    auto result = (std::uintmax_t(1) << bitsLen) - 1;
    return result;
}

std::uintmax_t parseCalcMaxVarUnsignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    auto totalValueBits = std::min((bitsLen / 8) * 7, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    return (static_cast<std::uint64_t>(1U) << totalValueBits) - 1U;
}

std::intmax_t parseCalcMaxVarSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    auto totalValueBits = std::min((bitsLen / 8U) * 7U, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::int64_t>::max();
    }

    auto result = parseCalcMaxVarUnsignedValue(bitsLen);
    auto mask = (result + 1) >> 1U;
    return static_cast<std::intmax_t>(result ^ mask);
}

std::intmax_t parseCalcMinVarSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    auto totalValueBits = std::min((bitsLen / 8U) * 7U, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::int64_t>::min();
    }

    std::uintmax_t result = ~(parseCalcMaxVarSignedValue(bitsLen));
    return static_cast<std::intmax_t>(result);
}

std::uintmax_t parseCalcMaxUnsignedValue(ParseIntFieldImpl::Type t, std::size_t bitsLen)
{
    if (t == ParseIntFieldImpl::Type::Uintvar) {
        return parseCalcMaxVarUnsignedValue(bitsLen);
    }

    return parseCalcMaxFixedUnsignedValue(bitsLen);
}

} // namespace

ParseIntFieldImpl::ParseIntFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
    m_state.m_nonUniqueSpecialsAllowed = !protocol.parseIsNonUniqueSpecialsAllowedSupported();
}

ParseIntFieldImpl::Type ParseIntFieldImpl::parseTypeValue(const std::string& value)
{
    static const std::map<std::string, Type> Map = {
        std::make_pair("int8", Type::Int8),
        std::make_pair("uint8", Type::Uint8),
        std::make_pair("int16", Type::Int16),
        std::make_pair("uint16", Type::Uint16),
        std::make_pair("int32", Type::Int32),
        std::make_pair("uint32", Type::Uint32),
        std::make_pair("int64", Type::Int64),
        std::make_pair("uint64", Type::Uint64),
        std::make_pair("intvar", Type::Intvar),
        std::make_pair("uintvar", Type::Uintvar)
    };

    auto iter = Map.find(common::toLowerCopy(value));
    if (iter == Map.end()) {
        return Type::NumOfValues;
    }

    return iter->second;
}

std::size_t ParseIntFieldImpl::parseMaxTypeLength(Type t)
{
    static const std::size_t Map[] = {
        /* Type::Int8 */ sizeof(std::int8_t),
        /* Type::Uint8 */ sizeof(std::uint8_t),
        /* Type::Int16 */ sizeof(std::int16_t),
        /* Type::Uint16 */ sizeof(std::uint16_t),
        /* Type::Int32 */ sizeof(std::int32_t),
        /* Type::Uint32 */ sizeof(std::uint32_t),
        /* Type::Int64 */ sizeof(std::int64_t),
        /* Type::Uint64 */ sizeof(std::uint64_t),
        /* Type::Intvar */ (((sizeof(std::intmax_t) * 8) - 1) / 7) + 1,
        /* Type::Uintvar */(((sizeof(std::uintmax_t) * 8) - 1) / 7) + 1
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(ParseIntFieldImpl::Type::NumOfValues),
                  "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return 0U;
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t ParseIntFieldImpl::parseMinTypeValue(Type t)
{
    static const std::intmax_t Map[] = {
        /* Type::Int8 */ parseMinValueForType<std::int8_t>(),
        /* Type::Uint8 */ parseMinValueForType<std::uint8_t>(),
        /* Type::Int16 */ parseMinValueForType<std::int16_t>(),
        /* Type::Uint16 */ parseMinValueForType<std::uint16_t>(),
        /* Type::Int32 */ parseMinValueForType<std::int32_t>(),
        /* Type::Uint32 */ parseMinValueForType<std::uint32_t>(),
        /* Type::Int64 */ parseMinValueForType<std::int64_t>(),
        /* Type::Uint64 */ parseMinValueForType<std::uint64_t>(),
        /* Type::Intvar */ parseMinValueForType<std::int64_t>(),
        /* Type::Uintvar */ parseMinValueForType<std::uint64_t>()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(ParseIntFieldImpl::Type::NumOfValues), "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return parseMaxValueForType<std::intmax_t>();
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t ParseIntFieldImpl::parseMaxTypeValue(Type t)
{
    static const std::intmax_t Map[] = {
        /* Type::Int8 */ parseMaxValueForType<std::int8_t>(),
        /* Type::Uint8 */ parseMaxValueForType<std::uint8_t>(),
        /* Type::Int16 */ parseMaxValueForType<std::int16_t>(),
        /* Type::Uint16 */ parseMaxValueForType<std::uint16_t>(),
        /* Type::Int32 */ parseMaxValueForType<std::int32_t>(),
        /* Type::Uint32 */ parseMaxValueForType<std::uint32_t>(),
        /* Type::Int64 */ parseMaxValueForType<std::int64_t>(),
        /* Type::Uint64 */ parseMaxValueForType<std::uint64_t>(),
        /* Type::Intvar */ parseMaxValueForType<std::intmax_t>(),
        /* Type::Uintvar */ parseMaxValueForType<std::uintmax_t>(),
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(ParseIntFieldImpl::Type::NumOfValues),
                  "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return parseMinValueForType<std::intmax_t>();
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t ParseIntFieldImpl::parseCalcMinValue(Type t, std::size_t bitsLen)
{
    if (parseIsTypeUnsigned(t)) {
        return 0;
    }

    if (t == ParseIntFieldImpl::Type::Intvar) {
        return parseCalcMinVarSignedValue(bitsLen);
    }

    return parseCalcMinFixedSignedValue(bitsLen);
}

std::intmax_t ParseIntFieldImpl::parseCalcMaxValue(Type t, std::size_t bitsLen)
{
    if (parseIsBigUnsigned(t)) {
        return static_cast<std::intmax_t>(parseCalcMaxUnsignedValue(t, bitsLen));
    }

    if (t == ParseIntFieldImpl::Type::Intvar) {
        return parseCalcMaxVarSignedValue(bitsLen);
    }

    if (parseIsTypeUnsigned(t)) {
        return static_cast<std::intmax_t>(parseCalcMaxUnsignedValue(t, bitsLen));
    }

    return parseCalcMaxFixedSignedValue(bitsLen);
}

bool ParseIntFieldImpl::parseIsUnsigned(Type t)
{
    static const Type Map[] = {
        Type::Uint8,
        Type::Uint16,
        Type::Uint32,
        Type::Uint64,
        Type::Uintvar,
    };

    auto iter = std::find(std::begin(Map), std::end(Map), t);
    return iter != std::end(Map);
}

bool ParseIntFieldImpl::parseIsTypeUnsigned(Type t)
{
    static const ParseIntFieldImpl::Type UnsignedTypes[] = {
        ParseIntFieldImpl::Type::Uint8,
        ParseIntFieldImpl::Type::Uint16,
        ParseIntFieldImpl::Type::Uint32,
        ParseIntFieldImpl::Type::Uint64,
        ParseIntFieldImpl::Type::Uintvar
    };

    auto iter = std::find(std::begin(UnsignedTypes), std::end(UnsignedTypes), t);
    return (iter != std::end(UnsignedTypes));
}

ParseFieldImpl::Kind ParseIntFieldImpl::parseKindImpl() const
{
    return Kind::Int;
}


ParseFieldImpl::Ptr ParseIntFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseIntFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseIntFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::typeStr(),
        common::defaultValueStr(),
        common::unitsStr(),
        common::scalingStr(),
        common::endianStr(),
        common::lengthStr(),
        common::bitLengthStr(),
        common::serOffsetStr(),
        common::validRangeStr(),
        common::validValueStr(),
        common::validMinStr(),
        common::validMaxStr(),
        common::validCheckVersionStr(),
        common::nonUniqueSpecialsAllowedStr(),
        common::displayDesimalsStr(),
        common::displayOffsetStr(),
        common::signExtStr(),
        common::displaySpecialsStr(),
        common::defaultValidValueStr(),
        common::availableLengthLimitStr()
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseIntFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::specialStr()
    };

    return List;
}

bool ParseIntFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseIntFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool ParseIntFieldImpl::parseImpl()
{
    return
        parseUpdateType() &&
        parseUpdateEndian() &&
        parseUpdateLength() &&
        parseUpdateBitLength() &&
        parseUpdateScaling() &&
        parseUpdateSerOffset() &&
        parseUpdateMinMaxValues() &&
        parseUpdateNonUniqueSpecialsAllowed() &&
        parseUpdateSpecials() &&
        parseUpdateDefaultValue() &&
        parseUpdateDefaultValidValue() &&
        parseUpdateValidCheckVersion() &&
        parseUpdateValidRanges() &&
        parseUpdateUnits() &&
        parseUpdateDisplayDecimals() &&
        parseUpdateDisplayOffset() &&
        parseUpdateSignExt() &&
        parseUpdateDisplaySpecials() &&
        parseUpdateAvailableLengthLimit();
}

std::size_t ParseIntFieldImpl::parseMinLengthImpl() const
{
    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        return 1U;
    }

    return m_state.m_length;
}

std::size_t ParseIntFieldImpl::parseMaxLengthImpl() const
{
    return m_state.m_length;
}

std::size_t ParseIntFieldImpl::parseBitLengthImpl() const
{
    if (parseIsBitfieldMember()) {
        return m_state.m_bitLength;
    }
    return Base::parseBitLengthImpl();
}

bool ParseIntFieldImpl::parseIsComparableToValueImpl(const std::string& val) const
{
    std::intmax_t value = 0;
    return parseStrToValue(val, value);
}

bool ParseIntFieldImpl::parseIsComparableToFieldImpl(const ParseFieldImpl& field) const
{
    auto fieldKind = field.parseKind();
    return ((fieldKind == Kind::Int) || (fieldKind == Kind::Enum));
}

bool ParseIntFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
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

    auto iter = m_state.m_specials.find(ref);
    if (iter == m_state.m_specials.end()) {
        return false;
    }

    val = iter->second.m_value;
    updateIsBigUnsignedFunc();
    return true;
}

bool ParseIntFieldImpl::parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const
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

bool ParseIntFieldImpl::parseVerifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    if (type == SemanticType::Version) {
        return true;
    }

    if ((type == SemanticType::Length) &&
        (parseProtocol().parseIsSemanticTypeLengthSupported())) {
        return true;
    }

    return false;
}

ParseIntFieldImpl::FieldRefInfo ParseIntFieldImpl::parseProcessInnerRefImpl(const std::string& refStr) const
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

bool ParseIntFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool ParseIntFieldImpl::parseUpdateType()
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

    auto type = parseTypeValue(propsIter->second);
    if (type == Type::NumOfValues) {
        parseReportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    if (mustHave) {
        m_state.m_type = type;
        return true;
    }

    if (type == m_state.m_type) {
        return true;
    }

    parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool ParseIntFieldImpl::parseUpdateEndian()
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

bool ParseIntFieldImpl::parseUpdateLength()
{
    if (!parseValidateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto maxLength = parseMaxTypeLength(m_state.m_type);
    auto& lengthStr = common::getStringProp(parseProps(), common::lengthStr());
    if (lengthStr.empty()) {
        if (m_state.m_length == 0) {
            m_state.m_length = maxLength;
            return true;
        }

        assert(m_state.m_length <= parseMaxTypeLength(m_state.m_type));
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

bool ParseIntFieldImpl::parseUpdateBitLength()
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
    m_state.m_bitLength = common::strToUnsigned(valStr, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::bitLengthStr(), valStr);
        return false;
    }

    if (!parseValidateBitLengthValue(m_state.m_bitLength)) {
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::parseUpdateSerOffset()
{
    if (!parseValidateSinglePropInstance(common::serOffsetStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(parseProps(), common::serOffsetStr());

    if (valueStr.empty()) {
        return true;
    }

    bool ok = false;
    m_state.m_serOffset = common::strToIntMax(valueStr, &ok);

    if (!ok) {
        parseReportUnexpectedPropertyValue(common::serOffsetStr(), valueStr);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::parseUpdateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = parseMinTypeValue(m_state.m_type);
    m_state.m_typeAllowedMaxValue = parseMaxTypeValue(m_state.m_type);

    m_state.m_minValue = parseCalcMinValue(m_state.m_type, m_state.m_bitLength);
    m_state.m_maxValue = parseCalcMaxValue(m_state.m_type, m_state.m_bitLength);

    if (m_state.m_serOffset == 0) {
        return true;
    }

    do {
        if ((parseIsBigUnsigned(m_state.m_type)) ||
            (m_state.m_typeAllowedMaxValue == std::numeric_limits<std::intmax_t>::max())) {
            break;
        }

        auto diff = (m_state.m_typeAllowedMaxValue - m_state.m_typeAllowedMinValue);
        assert(diff <= static_cast<decltype(diff)>(std::numeric_limits<std::uint32_t>::max()));
        if (std::abs(m_state.m_serOffset) < diff) {
            break;
        }

        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "The serialisation offset value is too big or too small for selected type.";
        return false;
    } while (false);

    auto updateMinValueFunc =
        [this](auto& val)
        {
            using ValType = std::decay_t<decltype(val)>;
            if (m_state.m_serOffset < 0) {
                val -= m_state.m_serOffset;
                return;
            }

            assert(0 < m_state.m_serOffset);
            auto limit = std::numeric_limits<ValType>::min() + m_state.m_serOffset;
            if (static_cast<ValType>(limit) < val) {
                val -= m_state.m_serOffset;
            }
        };

    auto updateMaxValueFunc =
        [this](auto& val)
        {
            using ValType = std::decay_t<decltype(val)>;
            if (m_state.m_serOffset < 0) {
                auto limit = std::numeric_limits<ValType>::max() + m_state.m_serOffset;
                if (val < static_cast<ValType>(limit)) {
                    val -= m_state.m_serOffset;
                }
                return;
            }

            assert(0 < m_state.m_serOffset);
            val -= m_state.m_serOffset;
        };

    if (!parseIsTypeUnsigned(m_state.m_type)) {
        updateMinValueFunc(m_state.m_minValue);
        updateMaxValueFunc(m_state.m_maxValue);
        return true;
    }

    auto minValueTmp = static_cast<std::uint64_t>(m_state.m_minValue);
    auto maxValueTmp = static_cast<std::uint64_t>(m_state.m_maxValue);

    updateMinValueFunc(minValueTmp);
    updateMaxValueFunc(maxValueTmp);

    m_state.m_minValue = static_cast<std::uint64_t>(minValueTmp);
    m_state.m_maxValue = static_cast<std::uint64_t>(maxValueTmp);

    return true;
}

bool ParseIntFieldImpl::parseUpdateDefaultValue()
{
    auto& prop = common::defaultValueStr();
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto& valueStr = common::getStringProp(parseProps(), prop);
    if (valueStr.empty()) {
        return true;
    }

    return parseUpdateDefaultValueInternal(valueStr);
}

bool ParseIntFieldImpl::parseUpdateDefaultValidValue()
{
    auto& prop = common::defaultValidValueStr();
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = parseProps().find(prop);
    if (iter == parseProps().end()) {
        return true;
    }

    if (!parseProtocol().parseIsPropertySupported(prop)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << 
            "Property \"" << prop << "\" is not supported for DSL version " << parseProtocol().parseCurrSchema().parseDslVersion() << ", ignoring...";        
        return true;
    }

    auto& valueStr = common::getStringProp(parseProps(), prop);
    if (!parseUpdateDefaultValueInternal(valueStr)) {
        return false;
    }

    ValidRangeInfo info;
    info.m_min = m_state.m_defaultValue;
    info.m_max = m_state.m_defaultValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();    
    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::parseUpdateScaling()
{
    if (!parseValidateSinglePropInstance(common::scalingStr())) {
        return false;
    }

    std::intmax_t num = m_state.m_scaling.first;
    if (num == 0) {
        num = 1;
    }
    std::intmax_t denom = m_state.m_scaling.second;
    if (denom == 0) {
        denom = 1;
    }

    do {
        auto& valueStr = common::getStringProp(parseProps(), common::scalingStr());
        if (valueStr.empty()) {
            break;
        }

        auto reportErrorFunc =
            [this, &valueStr]()
            {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << "The scaling ratio value of the \"" << parseName() <<
                              "\" is not of expected format (" << valueStr << ").";
            };

        static const char Sep = '/';
        auto sepPos = valueStr.find(Sep);

        if (sepPos == std::string::npos) {
            denom = 1;

            bool ok = false;
            num = common::strToIntMax(valueStr, &ok);
            if (!ok) {
                reportErrorFunc();
                return false;
            }
            break;
        }

        if (valueStr.find(Sep, sepPos + 1) != std::string::npos) {
            reportErrorFunc();
            return false;
        }

        std::string numStr(valueStr.begin(), valueStr.begin() + sepPos);
        std::string denomStr(valueStr.begin() + sepPos + 1, valueStr.end());

        if (numStr.empty() || denomStr.empty()) {
            reportErrorFunc();
            return false;
        }

        bool ok = false;
        num = common::strToIntMax(numStr, &ok);
        if (!ok) {
            reportErrorFunc();
            return false;
        }

        denom = common::strToIntMax(denomStr, &ok);
        if (!ok) {
            reportErrorFunc();
            return false;
        }

    } while (false);

    if ((num == 0) || (denom == 0)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Neither part of scaling fraction is allowed to be 0.";
        return false;
    }

    m_state.m_scaling = std::make_pair(num, denom);

    return true;
}

bool ParseIntFieldImpl::parseUpdateValidCheckVersion()
{
    return parseValidateAndUpdateBoolPropValue(common::validCheckVersionStr(), m_state.m_validCheckVersion);
}

bool ParseIntFieldImpl::parseUpdateValidRanges()
{
    auto attrs = ParseXmlWrap::parseNodeProps(parseGetNode());
    bool result =
        parseCheckValidRangeProps(attrs) &&
        parseCheckValidValueProps(attrs) &&
        parseCheckValidMinProps(attrs) &&
        parseCheckValidMaxProps(attrs);
    if (!result) {
        return false;
    }

    // Sort by version first
    bool bigUnsigned = parseIsBigUnsigned(m_state.m_type);
    std::sort(
        m_state.m_validRanges.begin(), m_state.m_validRanges.end(),
        [bigUnsigned](auto& elem1, auto& elem2)
        {
            assert(elem1.m_deprecatedSince != 0U);
            assert(elem2.m_deprecatedSince != 0U);
            if (elem1.m_sinceVersion != elem2.m_sinceVersion) {
                return elem1.m_sinceVersion < elem2.m_sinceVersion;
            }

            if (elem1.m_deprecatedSince != elem2.m_deprecatedSince) {
                return elem1.m_deprecatedSince < elem2.m_deprecatedSince;
            }

            if (bigUnsigned) {
                if (elem1.m_min != elem2.m_min) {
                    return static_cast<std::uintmax_t>(elem1.m_min) < static_cast<std::uintmax_t>(elem2.m_min);
                }

                return static_cast<std::uintmax_t>(elem1.m_max) < static_cast<std::uintmax_t>(elem2.m_max);
            }

            if (elem1.m_min != elem2.m_min) {
                return elem1.m_min < elem2.m_min;
            }

            return elem1.m_max < elem2.m_max;
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

            if ((iter->m_sinceVersion != nextIter->m_sinceVersion) ||
                (iter->m_deprecatedSince != nextIter->m_deprecatedSince) ||
                ((iter->m_max + 1) < nextIter->m_min)) {
                break;
            }

            assert(iter->m_min <= nextIter->m_min);
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
        [bigUnsigned](auto& elem1, auto& elem2)
        {
            assert(elem1.m_deprecatedSince != 0U);
            assert(elem2.m_deprecatedSince != 0U);
            if (elem1.m_min != elem2.m_min) {
                if (bigUnsigned) {
                    return static_cast<std::uintmax_t>(elem1.m_min) < static_cast<std::uintmax_t>(elem2.m_min);
                }
                else {
                    return elem1.m_min < elem2.m_min;
                }
            }

            if (elem1.m_max != elem2.m_max) {
                if (bigUnsigned) {
                    return static_cast<std::uintmax_t>(elem1.m_max) < static_cast<std::uintmax_t>(elem2.m_max);
                }
                else {
                    return elem1.m_max < elem2.m_max;
                }
            }

            if (elem1.m_sinceVersion != elem2.m_sinceVersion) {
                return elem1.m_sinceVersion < elem2.m_sinceVersion;
            }

            assert(elem1.m_deprecatedSince != elem2.m_deprecatedSince);
            return elem1.m_deprecatedSince < elem2.m_deprecatedSince;
        });

    return true;
}

bool ParseIntFieldImpl::parseUpdateNonUniqueSpecialsAllowed()
{
    return parseValidateAndUpdateBoolPropValue(common::nonUniqueSpecialsAllowedStr(), m_state.m_nonUniqueSpecialsAllowed);
}

bool ParseIntFieldImpl::parseUpdateSpecials()
{
    bool bigUnsignedType = parseIsBigUnsigned(m_state.m_type);
    auto specials = ParseXmlWrap::parseGetChildren(parseGetNode(), common::specialStr());

    using RecSpecials = std::multimap<std::intmax_t, std::string>;
    RecSpecials recSpecials;

    for (auto* s : specials) {
        static const ParseXmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::valStr(),
            common::sinceVersionStr(),
            common::deprecatedStr(),
            common::descriptionStr(),
            common::displayNameStr(),
        };

        auto props = ParseXmlWrap::parseNodeProps(s);
        if (!ParseXmlWrap::parseChildrenAsProps(s, PropNames, parseProtocol().parseLogger(), props)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::nameStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::valStr(), parseProtocol().parseLogger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::sinceVersionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::deprecatedStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::descriptionStr(), parseProtocol().parseLogger())) {
            return false;
        }

        if (!ParseXmlWrap::parseValidateSinglePropInstance(s, props, common::displayNameStr(), parseProtocol().parseLogger())) {
            return false;
        }

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto specialsIter = m_state.m_specials.find(nameIter->second);
        if (specialsIter != m_state.m_specials.end()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(s) << "Special with name \"" << nameIter->second <<
                          "\" was already assigned to \"" << parseName() << "\" element.";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        std::intmax_t val = 0;
        if (!parseStrToValue(valIter->second, val)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                "Value of special \"" << nameIter->second <<
                "\" (" << valIter->second << ") cannot be recognized.";
            return false;
        }

        if (!m_state.m_nonUniqueSpecialsAllowed) {
            auto recIter = recSpecials.find(val);
            if (recIter != recSpecials.end()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                    "Value of special \"" << nameIter->second <<
                    "\" (" << valIter->second << ") has already been defined as \"" <<
                    recIter->second << "\".";
                return false;
            }

            recSpecials.insert(std::make_pair(val, nameIter->second));
        }

        auto checkSpecialInRangeFunc =
            [this, s, &nameIter](auto v) -> bool
            {
                if ((v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) ||
                    (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v)) {
                    this->parseLogError() << ParseXmlWrap::parseLogPrefix(s) <<
                                    "Special value \"" << nameIter->second << "\" is outside the range of available values within a type.";
                    return false;
                }

                if ((v < static_cast<decltype(v)>(m_state.m_minValue)) ||
                    (static_cast<decltype(v)>(m_state.m_maxValue) < v)) {
                    this->parseLogWarning() << ParseXmlWrap::parseLogPrefix(s) <<
                                    "Special value \"" << nameIter->second << "\" is outside the range of correctly serializable values.";
                }

                return true;
            };

        bool checkResult = false;
        if (bigUnsignedType) {
            checkResult = checkSpecialInRangeFunc(static_cast<std::uintmax_t>(val));
        }
        else {
            checkResult = checkSpecialInRangeFunc(val);
        }

        if (!checkResult) {
            return false;
        }

        SpecialValueInfo info;
        info.m_value = val;
        info.m_sinceVersion = parseGetSinceVersion();
        info.m_deprecatedSince = parseGetDeprecated();
        if (!ParseXmlWrap::parseGetAndCheckVersions(s, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
            return false;
        }

        auto descIter = props.find(common::descriptionStr());
        if (descIter != props.end()) {
            info.m_description = descIter->second;
        }

        auto displayNameIter = props.find(common::displayNameStr());
        if (displayNameIter != props.end()) {
            info.m_displayName = displayNameIter->second;
        }

        m_state.m_specials.emplace(nameIter->second, info);
    }

    return true;
}

bool ParseIntFieldImpl::parseUpdateUnits()
{
    if (!parseValidateSinglePropInstance(common::unitsStr())) {
        return false;
    }

    auto iter = parseProps().find(common::unitsStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_units = common::strToUnits(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::unitsStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::parseUpdateDisplayDecimals()
{
    if (!parseValidateSinglePropInstance(common::displayDesimalsStr())) {
        return false;
    }

    auto iter = parseProps().find(common::displayDesimalsStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_displayDecimals = common::strToUnsigned(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::displayDesimalsStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::parseUpdateDisplayOffset()
{
    if (!parseValidateSinglePropInstance(common::displayOffsetStr())) {
        return false;
    }

    auto iter = parseProps().find(common::displayOffsetStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_displayOffset = common::strToIntMax(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::displayOffsetStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::parseUpdateSignExt()
{
    if (!parseValidateAndUpdateBoolPropValue(common::signExtStr(), m_state.m_signExt)) {
        return false;
    }

    auto& valueStr = common::getStringProp(parseProps(), common::signExtStr());
    if (valueStr.empty()) {
        return true;
    }

    do {
        if (parseIsUnsigned(m_state.m_type)) {
            break;
        }

        auto bitLen = parseBitLength();
        if (bitLen == 0U) {
            bitLen = parseMinLength() * 8U;
        }

        if ((parseMaxTypeLength(m_state.m_type) * 8U) <= bitLen) {
            break;
        }

        return true;
    } while (false);

    parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
        "Property \"" << common::signExtStr() << "\" is relevant only to signed types with "
        "length limitation.";

    return true;
}

bool ParseIntFieldImpl::parseUpdateDisplaySpecials()
{
    parseCheckAndReportDeprecatedPropertyValue(common::displaySpecialsStr());
    return true;
}

bool ParseIntFieldImpl::parseUpdateAvailableLengthLimit()
{
    return parseValidateAndUpdateBoolPropValue(common::availableLengthLimitStr(), m_state.m_availableLengthLimit);
}

bool ParseIntFieldImpl::parseCheckValidRangeAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validRangeStr());
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

bool ParseIntFieldImpl::parseCheckValidRangeAsChild(::xmlNodePtr child)
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

bool ParseIntFieldImpl::parseCheckValidRangeProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidRangeAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::validRangeStr());
    for (auto* c : children) {
        if (!parseCheckValidRangeAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::parseCheckValidValueAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validValueStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!parseValidateValidValueStr(iter->second, common::validValueStr(), info.m_min)) {
        return false;
    }

    info.m_max = info.m_min;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::parseCheckValidValueAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!parseValidateValidValueStr(str, common::validValueStr(), info.m_min)) {
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

bool ParseIntFieldImpl::parseCheckValidValueProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidValueAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::validValueStr());
    for (auto* c : children) {
        if (!parseCheckValidValueAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::parseCheckValidMinAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validMinStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!parseValidateValidValueStr(iter->second, common::validMinStr(), info.m_min)) {
        return false;
    }

    info.m_max = m_state.m_maxValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::parseCheckValidMinAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!parseValidateValidValueStr(str, common::validMinStr(), info.m_min)) {
        return false;
    }

    info.m_max = m_state.m_maxValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::parseCheckValidMinProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidMinAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::validMinStr());
    for (auto* c : children) {
        if (!parseCheckValidMinAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::parseCheckValidMaxAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validMaxStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!parseValidateValidValueStr(iter->second, common::validMaxStr(), info.m_max)) {
        return false;
    }

    info.m_min = m_state.m_minValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::parseCheckValidMaxAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!parseValidateValidValueStr(str, common::validMaxStr(), info.m_max)) {
        return false;
    }

    info.m_min = m_state.m_minValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::parseCheckValidMaxProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!parseCheckValidMaxAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::validMaxStr());
    for (auto* c : children) {
        if (!parseCheckValidMaxAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::parseValidateValidRangeStr(const std::string& str, std::intmax_t& minVal, std::intmax_t& maxVal)
{
    bool ok = false;
    auto range = common::parseRange(str, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::validRangeStr(), str);
        return false;
    }

    minVal = 0;
    if (!parseStrToValue(range.first, minVal)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Invalid min value in valid range (" << str << ").";
        return false;
    }

    maxVal = 0;
    if (!parseStrToValue(range.second, maxVal)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Invalid max value in valid range (" << str << ").";
        return false;
    }

    bool validComparison = (minVal <= maxVal);
    if (parseIsBigUnsigned(m_state.m_type)) {
        validComparison = (static_cast<std::uintmax_t>(minVal) <= static_cast<std::uintmax_t>(maxVal));
    }

    if (!validComparison) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Min value must be less than max in valid range (" << str << ").";
        return false;
    }

    auto validateFunc =
        [this](auto v, const std::string& vType)
        {
             if (v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) {
                 parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                        "Range's " << vType << " value (" << v << ") "
                        "is below the type's minimal value.";
             }

             if (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v) {
                 parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                        "Range's " << vType << " value (" << v << ") "
                        "is above the type's maximal value.";
             }
        };

    static const std::string MinStr("min");
    static const std::string MaxStr("max");
    if (parseIsBigUnsigned(m_state.m_type)) {
        validateFunc(static_cast<std::uintmax_t>(minVal), MinStr);
        validateFunc(static_cast<std::uintmax_t>(maxVal), MaxStr);
    }
    else {
        validateFunc(minVal, MinStr);
        validateFunc(maxVal, MaxStr);

    }

    return true;
}

bool ParseIntFieldImpl::parseValidateValidValueStr(
    const std::string& str,
    const std::string& type,
    std::intmax_t& val)
{
    val = 0;
    if (!parseStrToValue(str, val)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Property value \"" << type << "\" of int element \"" <<
                      parseName() << "\" cannot be properly parsed.";
        return false;
    }

    auto validateFunc =
        [this, &type](auto v)
        {
             if (v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) {
                 parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                                 "Property value \"" << type <<
                                 "\" is below the type's minimal value.";
             }

             if (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v) {
                 parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                                 "Property value \"" << type <<
                                 "\" is above the type's maximal value.";
             }
        };

    if (parseIsBigUnsigned(m_state.m_type)) {
        validateFunc(static_cast<std::uintmax_t>(val));
    }
    else {
        validateFunc(val);
    }

    return true;
}

bool ParseIntFieldImpl::parseStrToValue(
    const std::string& str,
    std::intmax_t& val) const
{
    if (common::isValidName(str)) {
        // Check among specials
        auto iter = m_state.m_specials.find(str);
        if (iter != m_state.m_specials.end()) {
            val = iter->second.m_value;
            return true;
        }
    }

    if (common::isValidRefName(str)) {
        bool bigUnsigned = false;
        if (!parseProtocol().parseStrToNumeric(str, false, val, bigUnsigned)) {
            return false;
        }

        if ((!bigUnsigned) && (val < 0) && (parseIsUnsigned(m_state.m_type))) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Cannot assign negative value (" << val << " references as " <<
            str << ") to field with positive type.";
            return false;
        }

        if (bigUnsigned && (!parseIsBigUnsigned(m_state.m_type))) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot assign such big positive number (" <<
            static_cast<std::uintmax_t>(val) << " referenced as " <<
            str << ").";
            return false;

        }
        return true;
    }

    bool ok = false;
    if (parseIsBigUnsigned(m_state.m_type)) {
        val = static_cast<std::intmax_t>(common::strToUintMax(str, &ok));
    }
    else {
        val = common::strToIntMax(str, &ok);
    }
    return ok;
}

bool ParseIntFieldImpl::parseUpdateDefaultValueInternal(const std::string& valueStr)
{
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
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << "The default value of the \"" << parseName() <<
                      "\" cannot be recongized (" << valueStr << ").";
        return false;
    }

    if (parseIsBigUnsigned(m_state.m_type)) {
        return checkValueFunc(static_cast<std::uint64_t>(val));
    }

    return checkValueFunc(val);
}


} // namespace parse

} // namespace commsdsl
