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
        common::serOffsetStr(),
        common::validRangeStr(),
        common::validValueStr()
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
        updateDefaultValue() &&
        updateScaling() &&
        updateValidRanges();
}

std::size_t IntFieldImpl::lengthImpl() const
{
    return m_length;
}

bool IntFieldImpl::updateType()
{
    if (!validateSinglePropInstance(common::typeStr(), true)) {
        return false;
    }

    auto propsIter = props().find(common::typeStr());
    assert (propsIter != props().end());

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
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

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
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

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
    if (!validateSinglePropInstance(common::serOffsetStr())) {
        return false;
    }

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
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

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

bool IntFieldImpl::updateScaling()
{
    if (!validateSinglePropInstance(common::scalingStr())) {
        return false;
    }

    std::intmax_t num = 1;
    std::intmax_t denom = 1;


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

    m_scaling = std::make_pair(num, denom);

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

                if (nextRange.first < thisRange.first) {

                }
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

    if (isBigUnsigned(m_type)) {
        using UnsignedValidRange = std::pair<std::uintmax_t, std::uintmax_t>;
        using UnsignedValidRangesList = std::vector<UnsignedValidRange>;

        UnsignedValidRangesList unsignedRanges;
        unsignedRanges.reserve(m_validRanges.size());
        std::transform(
            m_validRanges.begin(), m_validRanges.end(), std::back_inserter(unsignedRanges),
            [](auto& r)
            {
                return UnsignedValidRange(
                            static_cast<std::uintmax_t>(r.first),
                            static_cast<std::uintmax_t>(r.second));
            });

        sortFunc(unsignedRanges);
        unifyFunc(unsignedRanges);

        m_validRanges.clear();

        std::transform(
            unsignedRanges.begin(), unsignedRanges.end(), std::back_inserter(m_validRanges),
            [](auto& r)
            {
                return ValidRange(
                            static_cast<std::intmax_t>(r.first),
                            static_cast<std::intmax_t>(r.second));
            });
    }
    else {
        sortFunc(m_validRanges);
        unifyFunc(m_validRanges);
    }

    if (intersectingRanges) {
        logWarning() << XmlWrap::logPrefix(getNode()) << "Some valid values ranges of \"" << name() <<
                        "\" are intersecting.";
    }

    logError() << "Ranges:";
    for (auto& r : m_validRanges) {
        logError() << '[' << r.first << ", " << r.second << "]";
    }
    return true;
}

bool IntFieldImpl::validateValidRangeStr(const std::string& str)
{
    static const char Beg = '[';
    static const char End = ']';
    static const char Sep = ',';

    do {
        if (str.size() <= 3U) {
            break;
        }

        static const std::string WhiteChars(" \t\r\n");
        auto firstNonWhitePos = str.find_first_not_of(WhiteChars);
        if (firstNonWhitePos == std::string::npos) {
            break;
        }

        auto lastNonWhitePos = str.find_last_not_of(WhiteChars);
        assert(lastNonWhitePos != std::string::npos);
        if (lastNonWhitePos <= (firstNonWhitePos + 2)) {
            break;
        }

        auto begPos = str.find(Beg, firstNonWhitePos);
        if (begPos != firstNonWhitePos) {
            break;
        }

        auto sepPos = str.find(Sep, begPos + 1);
        if (sepPos == std::string::npos) {
            break;
        }

        if (str.find(Sep, sepPos + 1) != std::string::npos) {
            break;
        }

        auto endPos = str.find(End, sepPos + 1);
        if ((endPos == std::string::npos) ||
            (endPos != lastNonWhitePos)) {
            break;
        }


        assert(begPos < str.size());
        assert(sepPos < str.size());
        assert(endPos < str.size());
        std::string minStr(str.begin() + begPos + 1, str.begin() + sepPos);
        std::string maxStr(str.begin() + sepPos + 1, str.begin() + endPos);

        if (isBigUnsigned(m_type)) {
            bool ok = false;
            std::uintmax_t minVal = common::strToUintMax(minStr, &ok);
            if (!ok) {
                break;
            }

            std::uintmax_t maxVal = common::strToUintMax(maxStr, &ok);
            if (!ok) {
                break;
            }

            if (maxVal < minVal) {
                logError() << XmlWrap::logPrefix(getNode()) << "The min value is expected to be less or euqal to max in valid range of \"" << name() <<
                              "\" (" << str << ").";
                break;
            }

            m_validRanges.emplace_back(static_cast<std::intmax_t>(minVal), static_cast<std::intmax_t>(maxVal));
        }
        else {
            bool ok = false;
            std::intmax_t minVal = common::strToIntMax(minStr, &ok);
            if (!ok) {
                break;
            }

            std::intmax_t maxVal = common::strToIntMax(maxStr, &ok);
            if (!ok) {
                break;
            }

            if (maxVal < minVal) {
                logError() << XmlWrap::logPrefix(getNode()) << "The min value is expected to be less or euqal to max in valid range of \"" << name() <<
                              "\" (" << str << ").";
                break;
            }

            m_validRanges.emplace_back(minVal, maxVal);
        }

        return true;

    } while (false);

    logError() << XmlWrap::logPrefix(getNode()) << "The valid range of the \"" << name() <<
                  "\" is not of expected format (" << str << ").";
    return false;
}

bool IntFieldImpl::validateValidValueStr(const std::string& str)
{
    bool ok = false;
    std::intmax_t val = 0;
    if (isBigUnsigned(m_type)) {
        val = static_cast<decltype(val)>(common::strToUintMax(str, &ok));
    }
    else {
        val = common::strToIntMax(str, &ok);
    }

    if (!ok) {
        logError() << XmlWrap::logPrefix(getNode()) << "The valid value of the \"" << name() <<
                      "\" is not of expected format (" << str << ").";
        return false;
    }

    m_validRanges.emplace_back(val, val);
    return true;
}


} // namespace bbmp
