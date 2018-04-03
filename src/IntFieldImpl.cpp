#include "IntFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"
#include "util.h"

namespace bbmp
{

namespace
{

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

bool isTypeUnsigned(IntFieldImpl::Type t)
{
    static const IntFieldImpl::Type UnsignedTypes[] = {
        IntFieldImpl::Type::Uint8,
        IntFieldImpl::Type::Uint16,
        IntFieldImpl::Type::Uint32,
        IntFieldImpl::Type::Uint64,
        IntFieldImpl::Type::Uintvar
    };

    auto iter = std::find(std::begin(UnsignedTypes), std::end(UnsignedTypes), t);
    return (iter != std::end(UnsignedTypes));
}

std::uintmax_t calcMaxUnsignedValue(IntFieldImpl::Type t, std::size_t bitsLen)
{
    if (t == IntFieldImpl::Type::Uintvar) {
        return calcMaxVarUnsignedValue(bitsLen);
    }

    return calcMaxFixedUnsignedValue(bitsLen);
}

} // namespace

IntFieldImpl::IntFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

IntFieldImpl::Type IntFieldImpl::parseTypeValue(const std::string& value)
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

std::size_t IntFieldImpl::maxTypeLength(Type t)
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

    static_assert(MapSize == util::toUnsigned(IntFieldImpl::Type::NumOfValues),
                  "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        assert(!"Mustn't happen");
        return 0U;
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t IntFieldImpl::minTypeValue(Type t)
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

    static_assert(MapSize == util::toUnsigned(IntFieldImpl::Type::NumOfValues), "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        assert(!"Mustn't happen");
        return maxValueForType<std::intmax_t>();
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t IntFieldImpl::maxTypeValue(Type t)
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

    static_assert(MapSize == util::toUnsigned(IntFieldImpl::Type::NumOfValues),
                  "Invalid map");

    if (MapSize <= util::toUnsigned(t)) {
        assert(!"Mustn't happen");
        return minValueForType<std::intmax_t>();
    }

    return Map[util::toUnsigned(t)];
}

std::intmax_t IntFieldImpl::calcMinValue(Type t, std::size_t bitsLen)
{
    if (isTypeUnsigned(t)) {
        return 0;
    }

    if (t == IntFieldImpl::Type::Intvar) {
        return calcMinVarSignedValue(bitsLen);
    }

    return calcMinFixedSignedValue(bitsLen);
}

std::intmax_t IntFieldImpl::calcMaxValue(Type t, std::size_t bitsLen)
{
    if (isBigUnsigned(t)) {
        return static_cast<std::intmax_t>(calcMaxUnsignedValue(t, bitsLen));
    }

    if (t == IntFieldImpl::Type::Intvar) {
        return calcMaxVarSignedValue(bitsLen);
    }

    if (isTypeUnsigned(t)) {
        return static_cast<std::intmax_t>(calcMaxUnsignedValue(t, bitsLen));
    }

    return calcMaxFixedSignedValue(bitsLen);
}


FieldImpl::Kind IntFieldImpl::kindImpl() const
{
    return Kind::Int;
}


FieldImpl::Ptr IntFieldImpl::cloneImpl() const
{
    return Ptr(new IntFieldImpl(*this));
}

const XmlWrap::NamesList& IntFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
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
        common::validMaxStr()
    };

    return List;
}

const XmlWrap::NamesList& IntFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::specialStr()
    };

    return List;
}

bool IntFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const IntFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool IntFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateLength() &&
        updateBitLength() &&
        updateScaling() &&
        updateSerOffset() &&
        updateMinMaxValues() &&
        updateSpecials() &&
        updateDefaultValue() &&
        updateValidRanges();
}

std::size_t IntFieldImpl::lengthImpl() const
{
    return m_state.m_length;
}

std::size_t IntFieldImpl::bitLengthImpl() const
{
    return m_state.m_bitLength;
}

bool IntFieldImpl::updateType()
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

    m_state.m_type = parseTypeValue(propsIter->second);
    if (m_state.m_type == Type::NumOfValues) {
        reportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    return true;
}

bool IntFieldImpl::updateEndian()
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

bool IntFieldImpl::updateLength()
{
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    do {
        if (!lengthStr.empty()) {
            break;
        }

        if (m_state.m_length == 0) {
            m_state.m_length = maxTypeLength(m_state.m_type);
        }

        return true;
    } while (false);

    bool ok = false;
    m_state.m_length = static_cast<decltype(m_state.m_length)>(common::strToUintMax(lengthStr, &ok));

    if (!ok) {
        logError() << XmlWrap::logPrefix(getNode()) << "Length of the \"" << name() << "\" element has unexpected value (\"" << lengthStr << "\")";
        return false;
    }

    auto maxLength = maxTypeLength(m_state.m_type);
    assert(0U < maxLength);

    if (maxLength < m_state.m_length) {
        logError() << XmlWrap::logPrefix(getNode()) << "Length of the \"" << name() << "\" element (" << lengthStr << ") cannot execeed "
                      "max length allowed by the type (" << maxLength << ").";
        return false;
    }

    if (m_state.m_length == 0) {
        m_state.m_length = maxLength;
    }

    return true;
}

bool IntFieldImpl::updateBitLength()
{
    if (!validateSinglePropInstance(common::bitLengthStr())) {
        return false;
    }

    static const std::size_t BitsInByte =
         std::numeric_limits<std::uint8_t>::digits;
    static_assert(BitsInByte == 8U, "Invalid assumption");

    auto maxBitLength = m_state.m_length * BitsInByte;
    auto& valStr = common::getStringProp(props(), common::bitLengthStr());
    if (valStr.empty()) {
        assert(0 < m_state.m_length);
        if ((m_state.m_bitLength != 0) && (m_state.m_bitLength < maxBitLength)) {
            return true;
        }

        m_state.m_bitLength = maxBitLength;
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

bool IntFieldImpl::updateSerOffset()
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
        logError() << XmlWrap::logPrefix(getNode()) << "The serialisation offset value of the \"" <<
                      name() << "\" element has unexpected value.";
        return false;
    }

    return true;
}

bool IntFieldImpl::updateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = minTypeValue(m_state.m_type);
    m_state.m_typeAllowedMaxValue = maxTypeValue(m_state.m_type);

    m_state.m_minValue = calcMinValue(m_state.m_type, m_state.m_bitLength);
    m_state.m_maxValue = calcMaxValue(m_state.m_type, m_state.m_bitLength);

    if (m_state.m_serOffset == 0) {
        return true;
    }

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

bool IntFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto valueStr = common::getStringProp(props(), common::defaultValueStr());
    if (valueStr.empty()) {
        valueStr = std::to_string(m_state.m_defaultValue);
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
        logError() << XmlWrap::logPrefix(getNode()) << "The default value of the \"" << name() <<
                      "\" cannot be recongized (" << valueStr << ").";
        return false;
    }

    if (isBigUnsigned(m_state.m_type)) {
        return checkValueFunc(static_cast<std::uint64_t>(val));
    }

    return checkValueFunc(val);
}

bool IntFieldImpl::updateScaling()
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
                logError() << XmlWrap::logPrefix(getNode()) << "The scaling ratio value of the \"" << name() <<
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
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Neither part of scaling fraction is allowed to be 0.";
        return false;
    }

    m_state.m_scaling = std::make_pair(num, denom);

    return true;
}

bool IntFieldImpl::updateValidRanges()
{
    auto validRangersIters = props().equal_range(common::validRangeStr());
    for (auto iter = validRangersIters.first; iter != validRangersIters.second; ++iter) {
        if (!validateValidRangeStr(iter->second)) {
            return false;
        }
    }

    auto validValuesIters = props().equal_range(common::validValueStr());
    for (auto iter = validValuesIters.first; iter != validValuesIters.second; ++iter) {
        if (!validateValidValueStr(iter->second)) {
            return false;
        }
    }

    auto validMinValuesIters = props().equal_range(common::validMinStr());
    for (auto iter = validMinValuesIters.first; iter != validMinValuesIters.second; ++iter) {
        if (!validateValidMinValueStr(iter->second)) {
            return false;
        }
    }

    auto validMaxValuesIters = props().equal_range(common::validMaxStr());
    for (auto iter = validMaxValuesIters.first; iter != validMaxValuesIters.second; ++iter) {
        if (!validateValidMaxValueStr(iter->second)) {
            return false;
        }
    }

    auto sortFunc =
        [](auto& ranges)
        {
            std::sort(
                ranges.begin(), ranges.end(),
                [](auto& elem1, auto& elem2)
                {
                    return elem1.first < elem2.first;
                });
        };

    bool intersectingRanges = false;
    auto unifyFunc =
        [this, &intersectingRanges](auto& ranges)
        {
            if (ranges.size() < 2U) {
                return;
            }

            std::size_t idx = 0U;
            while ((idx + 2U) <= ranges.size()) {
                auto& thisRange = ranges[idx];
                auto& nextRange = ranges[idx + 1];

                assert(thisRange.first <= nextRange.first);
                if ((thisRange.second + 1) < nextRange.first) {
                    ++idx;
                    continue;
                }

                if (nextRange.first <= thisRange.second) {
                    intersectingRanges = true;
                }

                if (thisRange.second < nextRange.second) {
                    thisRange.second = nextRange.second;
                }

                ranges.erase(ranges.begin() + idx + 1);
            }
        };

    if (isBigUnsigned(m_state.m_type)) {
        using UnsignedValidRange = std::pair<std::uintmax_t, std::uintmax_t>;
        using UnsignedValidRangesList = std::vector<UnsignedValidRange>;

        UnsignedValidRangesList unsignedRanges;
        unsignedRanges.reserve(m_state.m_validRanges.size());
        std::transform(
            m_state.m_validRanges.begin(), m_state.m_validRanges.end(), std::back_inserter(unsignedRanges),
            [](auto& r)
            {
                return UnsignedValidRange(
                            static_cast<std::uintmax_t>(r.first),
                            static_cast<std::uintmax_t>(r.second));
            });

        sortFunc(unsignedRanges);
        unifyFunc(unsignedRanges);

        m_state.m_validRanges.clear();

        std::transform(
            unsignedRanges.begin(), unsignedRanges.end(), std::back_inserter(m_state.m_validRanges),
            [](auto& r)
            {
                return ValidRange(
                            static_cast<std::intmax_t>(r.first),
                            static_cast<std::intmax_t>(r.second));
            });
    }
    else {
        sortFunc(m_state.m_validRanges);
        unifyFunc(m_state.m_validRanges);
    }

    if (intersectingRanges) {
        logWarning() << XmlWrap::logPrefix(getNode()) << "Some valid values ranges of \"" << name() <<
                        "\" are intersecting.";
    }

    return true;
}

bool IntFieldImpl::updateSpecials()
{
    auto specials = XmlWrap::getChildren(getNode(), common::specialStr());
    for (auto* s : specials) {
        static const XmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::valStr()
        };

        auto props = XmlWrap::parseNodeProps(s);
        if (!XmlWrap::parseChildrenAsProps(s, PropNames, protocol().logger(), props)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(s, props, common::nameStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(s, props, common::valStr(), protocol().logger(), true)) {
            return false;
        }

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << XmlWrap::logPrefix(s) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto specialsIter = m_state.m_specials.find(nameIter->second);
        if (specialsIter != m_state.m_specials.end()) {
            logError() << XmlWrap::logPrefix(s) << "Special with name \"" << nameIter->second <<
                          "\" was already assigned to \"" << name() << "\" element.";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        std::intmax_t val = 0;
        if (!strToNumeric(valIter->second, val)) {
            logError() << XmlWrap::logPrefix(s) << "Special value \"" << nameIter->second <<
                          "\" cannot be recognized.";
            return false;
        }

        auto checkSpecialInRangeFunc =
            [this, s, &nameIter](auto v) -> bool
            {
                if ((v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) ||
                    (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v)) {
                    this->logError() << XmlWrap::logPrefix(s) <<
                                    "Special value \"" << nameIter->second << "\" is outside the range of available values within a type.";
                    return false;
                }

                if ((v < static_cast<decltype(v)>(m_state.m_minValue)) ||
                    (static_cast<decltype(v)>(m_state.m_maxValue) < v)) {
                    this->logWarning() << XmlWrap::logPrefix(s) <<
                                    "Special value \"" << nameIter->second << "\" is outside the range of correctly serializable values.";
                }

                return true;
            };

        bool checkResult = false;
        if (isBigUnsigned(m_state.m_type)) {
            checkResult = checkSpecialInRangeFunc(static_cast<std::uintmax_t>(val));
        }
        else {
            checkResult = checkSpecialInRangeFunc(val);
        }

        if (!checkResult) {
            return false;
        }

        m_state.m_specials.emplace(nameIter->second, val);
    }

    return true;
}

bool IntFieldImpl::validateValidRangeStr(const std::string& str)
{
    bool ok = false;
    auto range = common::parseRange(str, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validRangeStr(), str);
        return false;
    }

    std::intmax_t minVal = 0;
    if (!strToNumeric(range.first, minVal)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid min value in valid range (" << str << ").";
        return false;
    }

    std::intmax_t maxVal = 0;
    if (!strToNumeric(range.second, maxVal)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid max value in valid range (" << str << ").";
        return false;
    }

    bool validComparison = (minVal <= maxVal);
    if (isBigUnsigned(m_state.m_type)) {
        validComparison = (static_cast<std::uintmax_t>(minVal) <= static_cast<std::uintmax_t>(maxVal));
    }

    if (!validComparison) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Min value must be less than max in valid range (" << str << ").";
        return false;
    }

    m_state.m_validRanges.emplace_back(minVal, maxVal);
    return true;
}

bool IntFieldImpl::validateValidValueStr(const std::string& str)
{
    std::intmax_t val = 0;
    if (!strToNumeric(str, val)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Property value \"" << common::validValueStr() << "\" of int element \"" <<
                      name() << "\" cannot be properly parsed.";
        return false;
    }

    auto validateFunc =
        [this](auto v)
        {
             if (v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) {
                 logWarning() << "Property value \"" << common::validValueStr() <<
                                 "\" is below the type's minimal value.";
             }

             if (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v) {
                 logWarning() << "Property value \"" << common::validValueStr() <<
                                 "\" is above the type's maximal value.";
             }
        };

    if (isBigUnsigned(m_state.m_type)) {
        validateFunc(static_cast<std::uintmax_t>(val));
    }
    else {
        validateFunc(val);
    }

    m_state.m_validRanges.emplace_back(val, val);
    return true;
}

bool IntFieldImpl::validateValidMinValueStr(const std::string& str)
{
    std::intmax_t val = 0;
    if (!strToNumeric(str, val)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Property value \"" << common::validMinStr() << "\" of int element \"" <<
                      name() << "\" cannot be properly parsed.";
        return false;
    }

    auto validateFunc =
        [this](auto v) -> bool
        {
             if (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v) {
                 this->logError() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMinStr() <<
                        "\" (" << v << ") is greater than the type's maximal "
                        "value (" << m_state.m_typeAllowedMaxValue << ").";
                 return false;
             }

             if (v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) {
                 this->logError() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMinStr() <<
                        "\" (" << v << ") is less than the type's minimal "
                        "value (" << m_state.m_typeAllowedMinValue << ").";
                 return false;
             }

             if (static_cast<decltype(v)>(m_state.m_maxValue) < v) {
                 this->logWarning() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMinStr() <<
                        "\" (" << v << ") is greater than the correctly "
                        "serialisable maximal value (" << m_state.m_maxValue << ").";
             }

             if (v < static_cast<decltype(v)>(m_state.m_minValue)) {
                 this->logWarning() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMinStr() <<
                        "\" (" << v << ") is less than the correctly "
                        "serialisable minimal value (" << m_state.m_minValue << ").";
             }

             return true;
        };

    bool validateResult = false;
    if (isBigUnsigned(m_state.m_type)) {
        validateResult = validateFunc(static_cast<std::uintmax_t>(val));
    }
    else {
        validateResult = validateFunc(val);
    }

    if (!validateResult) {
        return false;
    }

    m_state.m_validRanges.emplace_back(val, m_state.m_typeAllowedMaxValue);
    return true;
}

bool IntFieldImpl::validateValidMaxValueStr(const std::string& str)
{
    std::intmax_t val = 0;
    if (!strToNumeric(str, val)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Property value \"" << common::validMaxStr() << "\" of int element \"" <<
                      name() << "\" cannot be properly parsed.";
        return false;
    }

    auto validateFunc =
        [this](auto v) -> bool
        {
             if (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v) {
                 this->logError() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMaxStr() <<
                        "\" (" << v << ") is greater than the type's maximal "
                        "value (" << m_state.m_typeAllowedMaxValue << ").";
                 return false;
             }

             if (v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) {
                 this->logError() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMaxStr() <<
                        "\" (" << v << ") is less than the type's minimal "
                        "value (" << m_state.m_typeAllowedMinValue << ").";
                 return false;
             }

             if (static_cast<decltype(v)>(m_state.m_maxValue) < v) {
                 this->logWarning() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMaxStr() <<
                        "\" (" << v << ") is greater than the correctly "
                        "serialisable maximal value (" << m_state.m_maxValue << ").";
             }

             if (v < static_cast<decltype(v)>(m_state.m_minValue)) {
                 this->logWarning() << XmlWrap::logPrefix(this->getNode()) <<
                        "Value of property \"" << common::validMaxStr() <<
                        "\" (" << v << ") is less than the correctly "
                        "serialisable minimal value (" << m_state.m_minValue << ").";
             }

             return true;
        };

    bool validateResult = false;
    if (isBigUnsigned(m_state.m_type)) {
        validateResult = validateFunc(static_cast<std::uintmax_t>(val));
    }
    else {
        validateResult = validateFunc(val);
    }

    if (!validateResult) {
        return false;
    }

    m_state.m_validRanges.emplace_back(m_state.m_typeAllowedMinValue, val);
    return true;
}

bool IntFieldImpl::strToNumeric(const std::string& str, std::intmax_t& val)
{
    if (common::isValidName(str)) {
        // Check among specials
        auto iter = m_state.m_specials.find(str);
        if (iter == m_state.m_specials.end()) {
            return false;
        }

        val = iter->second;
        return true;
    }

    if (common::isValidRefName(str)) {
        return protocol().strToEnumValue(str, val, false);
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


} // namespace bbmp
