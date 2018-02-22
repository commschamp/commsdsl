#include "IntFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"

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

std::intmax_t minTypeValue(IntFieldImpl::Type t)
{
    static const std::intmax_t Map[] = {
        /* Type_int8 */ minValueForType<std::int8_t>(),
        /* Type_uint8 */ minValueForType<std::uint8_t>(),
        /* Type_int16 */ minValueForType<std::int16_t>(),
        /* Type_uint16 */ minValueForType<std::uint16_t>(),
        /* Type_int32 */ minValueForType<std::int32_t>(),
        /* Type_uint32 */ minValueForType<std::uint32_t>(),
        /* Type_int64 */ minValueForType<std::int64_t>(),
        /* Type_uint64 */ minValueForType<std::uint64_t>(),
        /* Type_intvar */ minValueForType<std::int64_t>(),
        /* Type_uintvar */ minValueForType<std::uint64_t>()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == IntFieldImpl::Type_numOfValues, "Invalid map");

    if (MapSize <= t) {
        assert(!"Mustn't happen");
        return maxValueForType<std::intmax_t>();
    }

    return Map[t];
}

std::intmax_t maxTypeValue(IntFieldImpl::Type t)
{
    static const std::intmax_t Map[] = {
        /* Type_int8 */ maxValueForType<std::int8_t>(),
        /* Type_uint8 */ maxValueForType<std::uint8_t>(),
        /* Type_int16 */ maxValueForType<std::int16_t>(),
        /* Type_uint16 */ maxValueForType<std::uint16_t>(),
        /* Type_int32 */ maxValueForType<std::int32_t>(),
        /* Type_uint32 */ maxValueForType<std::uint32_t>(),
        /* Type_int64 */ maxValueForType<std::int64_t>(),
        /* Type_uint64 */ maxValueForType<std::uint64_t>(),
        /* Type_intvar */ maxValueForType<std::intmax_t>(),
        /* Type_uintvar */ maxValueForType<std::uintmax_t>(),
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == IntFieldImpl::Type_numOfValues, "Invalid map");

    if (MapSize <= t) {
        assert(!"Mustn't happen");
        return minValueForType<std::intmax_t>();
    }

    return Map[t];
}

std::size_t maxTypeLength(IntFieldImpl::Type t)
{
    static const std::size_t Map[] = {
        /* Type_int8 */ sizeof(std::int8_t),
        /* Type_uint8 */ sizeof(std::uint8_t),
        /* Type_int16 */ sizeof(std::int16_t),
        /* Type_uint16 */ sizeof(std::uint16_t),
        /* Type_int32 */ sizeof(std::int32_t),
        /* Type_uint32 */ sizeof(std::uint32_t),
        /* Type_int64 */ sizeof(std::int64_t),
        /* Type_uint64 */ sizeof(std::uint64_t),
        /* Type_intvar */ (((sizeof(std::intmax_t) * 8) - 1) / 7) + 1,
        /* Type_uintvar */(((sizeof(std::uintmax_t) * 8) - 1) / 7) + 1
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == IntFieldImpl::Type_numOfValues, "Invalid map");

    if (MapSize <= t) {
        assert(!"Mustn't happen");
        return 0U;
    }

    return Map[t];
}

std::intmax_t calcMaxFixedSignedValue(std::size_t len) {
    assert(0U < len);
    std::uintmax_t result = 0x7f;
    while (1U < len) {
        result = (result << 8U) | 0xff;
        --len;
    }
    return static_cast<std::intmax_t>(result);
}

std::intmax_t calcMinFixedSignedValue(std::size_t len) {
    assert(0U < len);
    std::uintmax_t result = ~(static_cast<std::uintmax_t>(calcMaxFixedSignedValue(len)));
    return *(reinterpret_cast<std::intmax_t*>(&result));
}

std::uintmax_t calcMaxFixedUnsignedValue(std::size_t len) {
    assert(0U < len);
    std::uintmax_t result = 0xff;
    while (1U < len) {
        result = (result << 8U) | 0xff;
        --len;
    }
    return result;
}

std::uintmax_t calcMaxVarUnsignedValue(std::size_t len) {
    assert(0U < len);
    auto totalValueBits = std::min(len * 7U, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    return (static_cast<std::uint64_t>(1U) << totalValueBits) - 1U;
}

std::intmax_t calcMaxVarSignedValue(std::size_t len) {
    assert(0U < len);
    auto totalValueBits = std::min(len * 7U, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::int64_t>::max();
    }

    auto result = calcMaxVarUnsignedValue(len);
    auto mask = (result + 1) >> 1U;
    return static_cast<std::intmax_t>(result ^ mask);
}

std::intmax_t calcMinVarSignedValue(std::size_t len) {
    assert(0U < len);
    auto totalValueBits = std::min(len * 7U, std::size_t(64U));
    if (totalValueBits == 64U) {
        return std::numeric_limits<std::int64_t>::min();
    }

    std::uintmax_t result = ~(calcMaxVarSignedValue(len));
    return *(reinterpret_cast<std::intmax_t*>(&result));
}

bool isTypeUnsigned(IntFieldImpl::Type t)
{
    static const IntFieldImpl::Type UnsignedTypes[] = {
        IntFieldImpl::Type_uint8,
        IntFieldImpl::Type_uint16,
        IntFieldImpl::Type_uint32,
        IntFieldImpl::Type_uint64,
        IntFieldImpl::Type_uintvar
    };

    auto iter = std::find(std::begin(UnsignedTypes), std::end(UnsignedTypes), t);
    return (iter != std::end(UnsignedTypes));
}

bool isBigUnsigned(IntFieldImpl::Type t)
{
    return (t == IntFieldImpl::Type_uint64) || (t == IntFieldImpl::Type_uintvar);
}

std::intmax_t calcMinValue(IntFieldImpl::Type t, std::size_t len)
{
    if (isTypeUnsigned(t)) {
        return 0;
    }

    if (t == IntFieldImpl::Type_intvar) {
        return calcMinVarSignedValue(len);
    }

    return calcMinFixedSignedValue(len);
}

std::uintmax_t calcMaxUnsignedValue(IntFieldImpl::Type t, std::size_t len)
{
    if (t == IntFieldImpl::Type_uintvar) {
        return calcMaxVarUnsignedValue(len);
    }

    return calcMaxFixedUnsignedValue(len);
}

std::intmax_t calcMaxValue(IntFieldImpl::Type t, std::size_t len)
{
    if (isBigUnsigned(t)) {
        return static_cast<std::intmax_t>(calcMaxUnsignedValue(t, len));
    }

    if (t == IntFieldImpl::Type_intvar) {
        return calcMaxVarSignedValue(len);
    }

    if (isTypeUnsigned(t)) {
        return static_cast<std::intmax_t>(calcMaxUnsignedValue(t, len));
    }

    return calcMaxFixedSignedValue(len);
}


} // namespace

IntFieldImpl::IntFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
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
        common::serOffsetStr()
    };

    return List;
}

bool IntFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateLength() &&
        updateSerOffset() &&
        updateMinMaxValues() &&
        updateDefaultValue();
}

std::size_t IntFieldImpl::lengthImpl() const
{
    return m_length;
}

bool IntFieldImpl::updateType()
{
    auto propsIter = props().find(common::typeStr());
    if (propsIter == props().end()) {
        logError() << XmlWrap::logPrefix(getNode()) << "Type of the \"" << name() << "\" element hasn't been specified.";
        return false;
    }

    static const std::string Map[] = {
        /* Type_int8 */ "int8",
        /* Type_uint8 */ "uint8",
        /* Type_int16 */ "int16",
        /* Type_uint16 */ "uint16",
        /* Type_int32 */ "int32",
        /* Type_uint32 */ "uint32",
        /* Type_int64 */ "int64",
        /* Type_uint64 */ "uint64",
        /* Type_intvar */ "intvar",
        /* Type_uintvar */ "uintvar",
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == Type_numOfValues, "Invalid map");

    auto iter = std::find(std::begin(Map), std::end(Map), propsIter->second);
    if (iter == std::end(Map)) {
        logError() << XmlWrap::logPrefix(getNode()) << "Type of the \"" << name() << "\" element has unknown value (\"" << propsIter->second << "\")";
        return false;
    }

    m_type = static_cast<decltype(m_type)>(std::distance(std::begin(Map), iter));
    return true;
}

bool IntFieldImpl::updateEndian()
{
    auto& endianStr = common::getStringProp(props(), common::endianStr());
    m_endian = common::parseEndian(endianStr, protocol().schemaImpl().endian());
    if (m_endian == Endian_NumOfValues) {
        logError() << XmlWrap::logPrefix(getNode()) << "Endian of the \"" << name() << "\" element has unknown value (\"" << endianStr << "\")";
        return false;
    }
    return true;
}

bool IntFieldImpl::updateLength()
{
    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    if (lengthStr.empty()) {
        m_length = maxTypeLength(m_type);
        return true;
    }

    bool ok = false;
    m_length = static_cast<decltype(m_length)>(common::strToUintMax(lengthStr, &ok));

    if (!ok) {
        logError() << XmlWrap::logPrefix(getNode()) << "Length of the \"" << name() << "\" element has unexpected value (\"" << lengthStr << "\")";
        return false;
    }

    auto maxLength = maxTypeLength(m_type);
    assert(0U < maxLength);

    if (maxLength < m_length) {
        logError() << XmlWrap::logPrefix(getNode()) << "Length of the \"" << name() << "\" element (" << lengthStr << ") cannot execeed "
                      "max length allowed by the type (" << maxLength << ").";
        return false;
    }

    if (m_length == 0) {
        m_length = maxLength;
    }

    return true;
}

bool IntFieldImpl::updateSerOffset()
{
    auto& valueStr = common::getStringProp(props(), common::serOffsetStr());

    if (valueStr.empty()) {
        assert(m_serOffset == 0);
        return true;
    }

    bool ok = false;
    m_serOffset = common::strToIntMax(valueStr, &ok);

    if (!ok) {
        logError() << XmlWrap::logPrefix(getNode()) << "The serialisation offset value of the \"" <<
                      name() << "\" element has unexpected value.";
        return false;
    }

    return true;
}

bool IntFieldImpl::updateMinMaxValues()
{
    m_minValue = calcMinValue(m_type, m_length) - m_serOffset;
    if (isTypeUnsigned(m_type)) {
        m_maxValue = static_cast<decltype(m_maxValue)>(calcMaxUnsignedValue(m_type, m_length)) - m_serOffset;
    }
    else {
        m_maxValue = calcMaxValue(m_type, m_length) - m_serOffset;
    }

    return true;
}

bool IntFieldImpl::updateDefaultValue()
{
    auto& valueStr = common::getStringProp(props(), common::defaultValueStr());

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

    auto minValueOfType = minTypeValue(m_type);
    auto maxValueOfType = maxTypeValue(m_type);

    if (isBigUnsigned(m_type)) {
        bool ok = false;
        auto unsignedDefaultValue = common::strToUintMax(valueStr, &ok);

        if (!ok) {
            reportErrorFunc();
            return false;
        }

        auto unsignedMinValueOfType = static_cast<std::uintmax_t>(minValueOfType);
        auto unsignedMaxValueOfType = static_cast<std::uintmax_t>(maxValueOfType);
        assert(unsignedMinValueOfType == 0U);

        if (unsignedDefaultValue < unsignedMinValueOfType) {
            reportErrorFunc();
            return false;
        }

        if (unsignedMaxValueOfType < unsignedDefaultValue) {
            reportErrorFunc();
            return false;
        }

        auto unsignedMinValue = static_cast<std::uintmax_t>(m_minValue);
        auto unsignedMaxValue = static_cast<std::uintmax_t>(m_maxValue);
        if ((unsignedDefaultValue < unsignedMinValue) ||
            (unsignedMaxValue < unsignedDefaultValue)) {
            reportWarningFunc();
        }

        m_defaultValue = static_cast<decltype(m_defaultValue)>(unsignedDefaultValue);
        return true;
    }

    bool ok = false;
    m_defaultValue = common::strToIntMax(valueStr, &ok);

    if (!ok) {
        reportErrorFunc();
        return false;
    }

    if (m_defaultValue < minValueOfType) {
        reportErrorFunc();
        return false;
    }


    if (maxValueOfType < m_defaultValue) {
        reportErrorFunc();
        return false;
    }

    if ((m_defaultValue < m_minValue) ||
        (m_maxValue < m_defaultValue)) {
        reportWarningFunc();
    }

    return true;
}


} // namespace bbmp
