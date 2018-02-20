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
        /* Type_uint64 */ minValueForType<std::intmax_t>(), // should not be used
        /* Type_intvar */ maxValueForType<std::int64_t>(),
        /* Type_uintvar */ minValueForType<std::intmax_t>(), // should not be used
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == IntFieldImpl::Type_numOfValues, "Invalid map");

    if (MapSize <= t) {
        assert(!"Mustn't happen");
        return minValueForType<std::intmax_t>();
    }

    return Map[t];
}

std::uintmax_t maxUnsignedTypeValue(IntFieldImpl::Type t)
{
    if ((t == IntFieldImpl::Type_uint64) || (t == IntFieldImpl::Type_uintvar)) {
        return maxValueForBigUnsignedType<std::uint64_t>();
    }

    return static_cast<std::uintmax_t>(maxTypeValue(t));
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
        common::endianStr()
    };

    return List;
}

bool IntFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateDefaultValue();
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

bool IntFieldImpl::updateDefaultValue()
{
    auto& valueStr = common::getStringProp(props(), common::defaultValueStr());

    static const Type UnsignedTypes[] = {
        Type_uint8,
        Type_uint16,
        Type_uint32,
        Type_uint64,
        Type_uintvar
    };

    auto reportErrorFunc =
        [this, &valueStr]()
        {
            logError() << XmlWrap::logPrefix(getNode()) << "The default value of the \"" << name() <<
                          "\" is not within type boundaries (" << valueStr << ").";
        };

    assert(m_type != Type_numOfValues);
    auto iter = std::find(std::begin(UnsignedTypes), std::end(UnsignedTypes), m_type);
    if (iter != std::end(UnsignedTypes)) {
        bool ok = false;
        m_defaultValue = common::strToUintMax(valueStr, &ok);
        if (!ok) {
            reportErrorFunc();
            return false;
        }

        auto minValue = static_cast<decltype(m_defaultValue)>(minTypeValue(m_type));
        if (m_defaultValue < minValue) {
            reportErrorFunc();
            return false;
        }

        auto maxValue = maxUnsignedTypeValue(m_type);
        if (maxValue < m_defaultValue) {
            reportErrorFunc();
            return false;
        }

        return true;
    }

    // signed type
    bool ok = false;
    std::intmax_t defaultValue = common::strToIntMax(valueStr, &ok);

    if (!ok) {
        reportErrorFunc();
        return false;
    }

    auto minValue = minTypeValue(m_type);
    if (defaultValue < minValue) {
        reportErrorFunc();
        return false;
    }

    auto maxValue = maxTypeValue(m_type);
    if (maxValue < defaultValue) {
        reportErrorFunc();
        return false;
    }

    m_defaultValue = *(reinterpret_cast<std::uintmax_t*>(&defaultValue));
    return true;
}

} // namespace bbmp
