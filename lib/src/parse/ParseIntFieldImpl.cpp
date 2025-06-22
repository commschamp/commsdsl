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
constexpr std::intmax_t minValueForType()
{
    return static_cast<std::intmax_t>(std::numeric_limits<T>::min());
}

template <typename T>
constexpr std::intmax_t maxValueForType()
{
    return static_cast<std::intmax_t>(std::numeric_limits<T>::max());
}

template <typename T>
constexpr std::uintmax_t maxValueForBigUnsignedType()
{
    return static_cast<std::uintmax_t>(std::numeric_limits<T>::max());
}

std::intmax_t calcMaxFixedSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    if (64U <= bitsLen) {
        return std::numeric_limits<std::int64_t>::max();
    }

    auto result = (std::uintmax_t(1) << (bitsLen - 1)) - 1;
    return static_cast<std::intmax_t>(result);
}

std::intmax_t calcMinFixedSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    std::uintmax_t result = ~(static_cast<std::uintmax_t>(calcMaxFixedSignedValue(bitsLen)));
    return static_cast<std::intmax_t>(result);
}

std::uintmax_t calcMaxFixedUnsignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    if (64U <= bitsLen) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    auto result = (std::uintmax_t(1) << bitsLen) - 1;
    return result;
}

std::uintmax_t calcMaxVarUnsignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    auto totalValueBits = std::min((bitsLen / 8) * 7, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    return (static_cast<std::uint64_t>(1U) << totalValueBits) - 1U;
}

std::intmax_t calcMaxVarSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    auto totalValueBits = std::min((bitsLen / 8U) * 7U, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::int64_t>::max();
    }

    auto result = calcMaxVarUnsignedValue(bitsLen);
    auto mask = (result + 1) >> 1U;
    return static_cast<std::intmax_t>(result ^ mask);
}

std::intmax_t calcMinVarSignedValue(std::size_t bitsLen) {
    assert(0U < bitsLen);
    auto totalValueBits = std::min((bitsLen / 8U) * 7U, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::int64_t>::min();
    }

    std::uintmax_t result = ~(calcMaxVarSignedValue(bitsLen));
    return static_cast<std::intmax_t>(result);
}

std::uintmax_t calcMaxUnsignedValue(ParseIntFieldImpl::Type t, std::size_t bitsLen)
{
    if (t == ParseIntFieldImpl::Type::Uintvar) {
        return calcMaxVarUnsignedValue(bitsLen);
    }

    return calcMaxFixedUnsignedValue(bitsLen);
}

} // namespace

ParseIntFieldImpl::ParseIntFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
    m_state.m_nonUniqueSpecialsAllowed = !protocol.isNonUniqueSpecialsAllowedSupported();
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

std::size_t ParseIntFieldImpl::maxTypeLength(Type t)
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

std::intmax_t ParseIntFieldImpl::minTypeValue(Type t)
{
    static const std::intmax_t Map[] = {
        /* Type::Int8 */ minValueForType<std::int8_t>(),
        /* Type::Uint8 */ minValueForType<std::uint8_t>(),
        /* Type::Int16 */ minValueForType<std::int16_t>(),
        /* Type::Uint16 */ minValueForType<std::uint16_t>(),
        /* Type::Int32 */ minValueForType<std::int32_t>(),
        /* Type::Uint32 */ minValueForType<std::uint32_t>(),
        /* Type::Int64 */ minValueForType<std::int64_t>(),
        /* Type::Uint64 */ minValueForType<std::uint64_t>(),
        /* Type::Intvar */ minValueForType<std::int64_t>(),
        /* Type::Uintvar */ minValueForType<std::uint64_t>()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(ParseIntFieldImpl::Type::NumOfValues), "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return maxValueForType<std::intmax_t>();
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t ParseIntFieldImpl::maxTypeValue(Type t)
{
    static const std::intmax_t Map[] = {
        /* Type::Int8 */ maxValueForType<std::int8_t>(),
        /* Type::Uint8 */ maxValueForType<std::uint8_t>(),
        /* Type::Int16 */ maxValueForType<std::int16_t>(),
        /* Type::Uint16 */ maxValueForType<std::uint16_t>(),
        /* Type::Int32 */ maxValueForType<std::int32_t>(),
        /* Type::Uint32 */ maxValueForType<std::uint32_t>(),
        /* Type::Int64 */ maxValueForType<std::int64_t>(),
        /* Type::Uint64 */ maxValueForType<std::uint64_t>(),
        /* Type::Intvar */ maxValueForType<std::intmax_t>(),
        /* Type::Uintvar */ maxValueForType<std::uintmax_t>(),
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(ParseIntFieldImpl::Type::NumOfValues),
                  "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return minValueForType<std::intmax_t>();
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t ParseIntFieldImpl::calcMinValue(Type t, std::size_t bitsLen)
{
    if (isTypeUnsigned(t)) {
        return 0;
    }

    if (t == ParseIntFieldImpl::Type::Intvar) {
        return calcMinVarSignedValue(bitsLen);
    }

    return calcMinFixedSignedValue(bitsLen);
}

std::intmax_t ParseIntFieldImpl::calcMaxValue(Type t, std::size_t bitsLen)
{
    if (isBigUnsigned(t)) {
        return static_cast<std::intmax_t>(calcMaxUnsignedValue(t, bitsLen));
    }

    if (t == ParseIntFieldImpl::Type::Intvar) {
        return calcMaxVarSignedValue(bitsLen);
    }

    if (isTypeUnsigned(t)) {
        return static_cast<std::intmax_t>(calcMaxUnsignedValue(t, bitsLen));
    }

    return calcMaxFixedSignedValue(bitsLen);
}

bool ParseIntFieldImpl::isUnsigned(Type t)
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

bool ParseIntFieldImpl::isTypeUnsigned(Type t)
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

ParseFieldImpl::Kind ParseIntFieldImpl::kindImpl() const
{
    return Kind::Int;
}


ParseFieldImpl::Ptr ParseIntFieldImpl::cloneImpl() const
{
    return Ptr(new ParseIntFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseIntFieldImpl::extraPropsNamesImpl() const
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

const ParseXmlWrap::NamesList& ParseIntFieldImpl::extraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::specialStr()
    };

    return List;
}

bool ParseIntFieldImpl::reuseImpl(const ParseFieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ParseIntFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool ParseIntFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateLength() &&
        updateBitLength() &&
        updateScaling() &&
        updateSerOffset() &&
        updateMinMaxValues() &&
        updateNonUniqueSpecialsAllowed() &&
        updateSpecials() &&
        updateDefaultValue() &&
        updateDefaultValidValue() &&
        updateValidCheckVersion() &&
        updateValidRanges() &&
        updateUnits() &&
        updateDisplayDecimals() &&
        updateDisplayOffset() &&
        updateSignExt() &&
        updateDisplaySpecials() &&
        updateAvailableLengthLimit();
}

std::size_t ParseIntFieldImpl::minLengthImpl() const
{
    if ((m_state.m_type == Type::Intvar) || (m_state.m_type == Type::Uintvar)) {
        return 1U;
    }

    return m_state.m_length;
}

std::size_t ParseIntFieldImpl::maxLengthImpl() const
{
    return m_state.m_length;
}

std::size_t ParseIntFieldImpl::bitLengthImpl() const
{
    if (isBitfieldMember()) {
        return m_state.m_bitLength;
    }
    return Base::bitLengthImpl();
}

bool ParseIntFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    std::intmax_t value = 0;
    return strToValue(val, value);
}

bool ParseIntFieldImpl::isComparableToFieldImpl(const ParseFieldImpl& field) const
{
    auto fieldKind = field.kind();
    return ((fieldKind == Kind::Int) || (fieldKind == Kind::Enum));
}

bool ParseIntFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (!protocol().isFieldValueReferenceSupported()) {
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

    auto iter = m_state.m_specials.find(ref);
    if (iter == m_state.m_specials.end()) {
        return false;
    }

    val = iter->second.m_value;
    updateIsBigUnsignedFunc();
    return true;
}

bool ParseIntFieldImpl::validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const
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

bool ParseIntFieldImpl::verifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    if (type == SemanticType::Version) {
        return true;
    }

    if ((type == SemanticType::Length) &&
        (protocol().isSemanticTypeLengthSupported())) {
        return true;
    }

    return false;
}

ParseIntFieldImpl::FieldRefInfo ParseIntFieldImpl::processInnerRefImpl(const std::string& refStr) const
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

bool ParseIntFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool ParseIntFieldImpl::updateType()
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

    auto type = parseTypeValue(propsIter->second);
    if (type == Type::NumOfValues) {
        reportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    if (mustHave) {
        m_state.m_type = type;
        return true;
    }

    if (type == m_state.m_type) {
        return true;
    }

    logError() << ParseXmlWrap::logPrefix(getNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool ParseIntFieldImpl::updateEndian()
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

bool ParseIntFieldImpl::updateLength()
{
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto maxLength = maxTypeLength(m_state.m_type);
    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    if (lengthStr.empty()) {
        if (m_state.m_length == 0) {
            m_state.m_length = maxLength;
            return true;
        }

        assert(m_state.m_length <= maxTypeLength(m_state.m_type));
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

bool ParseIntFieldImpl::updateBitLength()
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

bool ParseIntFieldImpl::updateSerOffset()
{
    if (!validateSinglePropInstance(common::serOffsetStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::serOffsetStr());

    if (valueStr.empty()) {
        return true;
    }

    bool ok = false;
    m_state.m_serOffset = common::strToIntMax(valueStr, &ok);

    if (!ok) {
        reportUnexpectedPropertyValue(common::serOffsetStr(), valueStr);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::updateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = minTypeValue(m_state.m_type);
    m_state.m_typeAllowedMaxValue = maxTypeValue(m_state.m_type);

    m_state.m_minValue = calcMinValue(m_state.m_type, m_state.m_bitLength);
    m_state.m_maxValue = calcMaxValue(m_state.m_type, m_state.m_bitLength);

    if (m_state.m_serOffset == 0) {
        return true;
    }

    do {
        if ((isBigUnsigned(m_state.m_type)) ||
            (m_state.m_typeAllowedMaxValue == std::numeric_limits<std::intmax_t>::max())) {
            break;
        }

        auto diff = (m_state.m_typeAllowedMaxValue - m_state.m_typeAllowedMinValue);
        assert(diff <= static_cast<decltype(diff)>(std::numeric_limits<std::uint32_t>::max()));
        if (std::abs(m_state.m_serOffset) < diff) {
            break;
        }

        logError() << ParseXmlWrap::logPrefix(getNode()) <<
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

    if (!isTypeUnsigned(m_state.m_type)) {
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

bool ParseIntFieldImpl::updateDefaultValue()
{
    auto& prop = common::defaultValueStr();
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), prop);
    if (valueStr.empty()) {
        return true;
    }

    return updateDefaultValueInternal(valueStr);
}

bool ParseIntFieldImpl::updateDefaultValidValue()
{
    auto& prop = common::defaultValidValueStr();
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = props().find(prop);
    if (iter == props().end()) {
        return true;
    }

    if (!protocol().isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) << 
            "Property \"" << prop << "\" is not supported for DSL version " << protocol().currSchema().dslVersion() << ", ignoring...";        
        return true;
    }

    auto& valueStr = common::getStringProp(props(), prop);
    if (!updateDefaultValueInternal(valueStr)) {
        return false;
    }

    ValidRangeInfo info;
    info.m_min = m_state.m_defaultValue;
    info.m_max = m_state.m_defaultValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();    
    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::updateScaling()
{
    if (!validateSinglePropInstance(common::scalingStr())) {
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
        auto& valueStr = common::getStringProp(props(), common::scalingStr());
        if (valueStr.empty()) {
            break;
        }

        auto reportErrorFunc =
            [this, &valueStr]()
            {
                logError() << ParseXmlWrap::logPrefix(getNode()) << "The scaling ratio value of the \"" << name() <<
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
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Neither part of scaling fraction is allowed to be 0.";
        return false;
    }

    m_state.m_scaling = std::make_pair(num, denom);

    return true;
}

bool ParseIntFieldImpl::updateValidCheckVersion()
{
    return validateAndUpdateBoolPropValue(common::validCheckVersionStr(), m_state.m_validCheckVersion);
}

bool ParseIntFieldImpl::updateValidRanges()
{
    auto attrs = ParseXmlWrap::parseNodeProps(getNode());
    bool result =
        checkValidRangeProps(attrs) &&
        checkValidValueProps(attrs) &&
        checkValidMinProps(attrs) &&
        checkValidMaxProps(attrs);
    if (!result) {
        return false;
    }

    // Sort by version first
    bool bigUnsigned = isBigUnsigned(m_state.m_type);
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

bool ParseIntFieldImpl::updateNonUniqueSpecialsAllowed()
{
    return validateAndUpdateBoolPropValue(common::nonUniqueSpecialsAllowedStr(), m_state.m_nonUniqueSpecialsAllowed);
}

bool ParseIntFieldImpl::updateSpecials()
{
    bool bigUnsignedType = isBigUnsigned(m_state.m_type);
    auto specials = ParseXmlWrap::getChildren(getNode(), common::specialStr());

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
        if (!ParseXmlWrap::parseChildrenAsProps(s, PropNames, protocol().logger(), props)) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(s, props, common::nameStr(), protocol().logger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(s, props, common::valStr(), protocol().logger(), true)) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(s, props, common::sinceVersionStr(), protocol().logger())) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(s, props, common::deprecatedStr(), protocol().logger())) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(s, props, common::descriptionStr(), protocol().logger())) {
            return false;
        }

        if (!ParseXmlWrap::validateSinglePropInstance(s, props, common::displayNameStr(), protocol().logger())) {
            return false;
        }

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << ParseXmlWrap::logPrefix(s) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto specialsIter = m_state.m_specials.find(nameIter->second);
        if (specialsIter != m_state.m_specials.end()) {
            logError() << ParseXmlWrap::logPrefix(s) << "Special with name \"" << nameIter->second <<
                          "\" was already assigned to \"" << name() << "\" element.";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        std::intmax_t val = 0;
        if (!strToValue(valIter->second, val)) {
            logError() << ParseXmlWrap::logPrefix(s) <<
                "Value of special \"" << nameIter->second <<
                "\" (" << valIter->second << ") cannot be recognized.";
            return false;
        }

        if (!m_state.m_nonUniqueSpecialsAllowed) {
            auto recIter = recSpecials.find(val);
            if (recIter != recSpecials.end()) {
                logError() << ParseXmlWrap::logPrefix(s) <<
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
                    this->logError() << ParseXmlWrap::logPrefix(s) <<
                                    "Special value \"" << nameIter->second << "\" is outside the range of available values within a type.";
                    return false;
                }

                if ((v < static_cast<decltype(v)>(m_state.m_minValue)) ||
                    (static_cast<decltype(v)>(m_state.m_maxValue) < v)) {
                    this->logWarning() << ParseXmlWrap::logPrefix(s) <<
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
        info.m_sinceVersion = getSinceVersion();
        info.m_deprecatedSince = getDeprecated();
        if (!ParseXmlWrap::getAndCheckVersions(s, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
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

bool ParseIntFieldImpl::updateUnits()
{
    if (!validateSinglePropInstance(common::unitsStr())) {
        return false;
    }

    auto iter = props().find(common::unitsStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_units = common::strToUnits(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::unitsStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::updateDisplayDecimals()
{
    if (!validateSinglePropInstance(common::displayDesimalsStr())) {
        return false;
    }

    auto iter = props().find(common::displayDesimalsStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_displayDecimals = common::strToUnsigned(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::displayDesimalsStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::updateDisplayOffset()
{
    if (!validateSinglePropInstance(common::displayOffsetStr())) {
        return false;
    }

    auto iter = props().find(common::displayOffsetStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    m_state.m_displayOffset = common::strToIntMax(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::displayOffsetStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseIntFieldImpl::updateSignExt()
{
    if (!validateAndUpdateBoolPropValue(common::signExtStr(), m_state.m_signExt)) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::signExtStr());
    if (valueStr.empty()) {
        return true;
    }

    do {
        if (isUnsigned(m_state.m_type)) {
            break;
        }

        auto bitLen = bitLength();
        if (bitLen == 0U) {
            bitLen = minLength() * 8U;
        }

        if ((maxTypeLength(m_state.m_type) * 8U) <= bitLen) {
            break;
        }

        return true;
    } while (false);

    logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
        "Property \"" << common::signExtStr() << "\" is relevant only to signed types with "
        "length limitation.";

    return true;
}

bool ParseIntFieldImpl::updateDisplaySpecials()
{
    checkAndReportDeprecatedPropertyValue(common::displaySpecialsStr());
    return true;
}

bool ParseIntFieldImpl::updateAvailableLengthLimit()
{
    return validateAndUpdateBoolPropValue(common::availableLengthLimitStr(), m_state.m_availableLengthLimit);
}

bool ParseIntFieldImpl::checkValidRangeAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validRangeStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;

    if (!validateValidRangeStr(iter->second, info.m_min, info.m_max)) {
        return false;
    }

    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();
    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidRangeAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!validateValidRangeStr(str, info.m_min, info.m_max)) {
        return false;
    }

    if (!ParseXmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidRangeProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidRangeAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::getChildren(getNode(), common::validRangeStr());
    for (auto* c : children) {
        if (!checkValidRangeAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::checkValidValueAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validValueStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!validateValidValueStr(iter->second, common::validValueStr(), info.m_min)) {
        return false;
    }

    info.m_max = info.m_min;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidValueAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!validateValidValueStr(str, common::validValueStr(), info.m_min)) {
        return false;
    }

    info.m_max = info.m_min;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!ParseXmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidValueProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidValueAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::getChildren(getNode(), common::validValueStr());
    for (auto* c : children) {
        if (!checkValidValueAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::checkValidMinAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validMinStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!validateValidValueStr(iter->second, common::validMinStr(), info.m_min)) {
        return false;
    }

    info.m_max = m_state.m_maxValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidMinAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!validateValidValueStr(str, common::validMinStr(), info.m_min)) {
        return false;
    }

    info.m_max = m_state.m_maxValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!ParseXmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidMinProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidMinAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::getChildren(getNode(), common::validMinStr());
    for (auto* c : children) {
        if (!checkValidMinAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::checkValidMaxAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validMaxStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!validateValidValueStr(iter->second, common::validMaxStr(), info.m_max)) {
        return false;
    }

    info.m_min = m_state.m_minValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidMaxAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!validateValidValueStr(str, common::validMaxStr(), info.m_max)) {
        return false;
    }

    info.m_min = m_state.m_minValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!ParseXmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool ParseIntFieldImpl::checkValidMaxProps(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidMaxAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = ParseXmlWrap::getChildren(getNode(), common::validMaxStr());
    for (auto* c : children) {
        if (!checkValidMaxAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool ParseIntFieldImpl::validateValidRangeStr(const std::string& str, std::intmax_t& minVal, std::intmax_t& maxVal)
{
    bool ok = false;
    auto range = common::parseRange(str, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validRangeStr(), str);
        return false;
    }

    minVal = 0;
    if (!strToValue(range.first, minVal)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Invalid min value in valid range (" << str << ").";
        return false;
    }

    maxVal = 0;
    if (!strToValue(range.second, maxVal)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Invalid max value in valid range (" << str << ").";
        return false;
    }

    bool validComparison = (minVal <= maxVal);
    if (isBigUnsigned(m_state.m_type)) {
        validComparison = (static_cast<std::uintmax_t>(minVal) <= static_cast<std::uintmax_t>(maxVal));
    }

    if (!validComparison) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Min value must be less than max in valid range (" << str << ").";
        return false;
    }

    auto validateFunc =
        [this](auto v, const std::string& vType)
        {
             if (v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) {
                 logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
                        "Range's " << vType << " value (" << v << ") "
                        "is below the type's minimal value.";
             }

             if (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v) {
                 logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
                        "Range's " << vType << " value (" << v << ") "
                        "is above the type's maximal value.";
             }
        };

    static const std::string MinStr("min");
    static const std::string MaxStr("max");
    if (isBigUnsigned(m_state.m_type)) {
        validateFunc(static_cast<std::uintmax_t>(minVal), MinStr);
        validateFunc(static_cast<std::uintmax_t>(maxVal), MaxStr);
    }
    else {
        validateFunc(minVal, MinStr);
        validateFunc(maxVal, MaxStr);

    }

    return true;
}

bool ParseIntFieldImpl::validateValidValueStr(
    const std::string& str,
    const std::string& type,
    std::intmax_t& val)
{
    val = 0;
    if (!strToValue(str, val)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Property value \"" << type << "\" of int element \"" <<
                      name() << "\" cannot be properly parsed.";
        return false;
    }

    auto validateFunc =
        [this, &type](auto v)
        {
             if (v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) {
                 logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
                                 "Property value \"" << type <<
                                 "\" is below the type's minimal value.";
             }

             if (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v) {
                 logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
                                 "Property value \"" << type <<
                                 "\" is above the type's maximal value.";
             }
        };

    if (isBigUnsigned(m_state.m_type)) {
        validateFunc(static_cast<std::uintmax_t>(val));
    }
    else {
        validateFunc(val);
    }

    return true;
}

bool ParseIntFieldImpl::strToValue(
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
        if (!protocol().strToNumeric(str, false, val, bigUnsigned)) {
            return false;
        }

        if ((!bigUnsigned) && (val < 0) && (isUnsigned(m_state.m_type))) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                "Cannot assign negative value (" << val << " references as " <<
            str << ") to field with positive type.";
            return false;
        }

        if (bigUnsigned && (!isBigUnsigned(m_state.m_type))) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot assign such big positive number (" <<
            static_cast<std::uintmax_t>(val) << " referenced as " <<
            str << ").";
            return false;

        }
        return true;
    }

    bool ok = false;
    if (isBigUnsigned(m_state.m_type)) {
        val = static_cast<std::intmax_t>(common::strToUintMax(str, &ok));
    }
    else {
        val = common::strToIntMax(str, &ok);
    }
    return ok;
}

bool ParseIntFieldImpl::updateDefaultValueInternal(const std::string& valueStr)
{
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
        logError() << ParseXmlWrap::logPrefix(getNode()) << "The default value of the \"" << name() <<
                      "\" cannot be recongized (" << valueStr << ").";
        return false;
    }

    if (isBigUnsigned(m_state.m_type)) {
        return checkValueFunc(static_cast<std::uint64_t>(val));
    }

    return checkValueFunc(val);
}


} // namespace parse

} // namespace commsdsl
