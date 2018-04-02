#include "FloatFieldImpl.h"

#include <algorithm>
#include <iterator>
#include <cassert>
#include <limits>
#include <cmath>

#include "common.h"
#include "util.h"
#include "ProtocolImpl.h"

namespace bbmp
{

namespace
{

bool compareLess(double val1, double val2)
{
    static_assert(-std::numeric_limits<double>::infinity() < std::numeric_limits<double>::infinity(), "Invalid assumption");

    if (std::isnan(val1)) {
        return false;
    }

    if (std::isnan(val2)) {
        return true;
    }

    if (std::isinf(val1)) {
        if (val1 < 0) {
            return (!std::isinf(val2)) || (0 < val2);
        }

        return false;
    }

    if (std::isinf(val2)) {
        if (val2 < 0) {
            return false;
        }

        return true;
    }

    return val1 < val2;
}

double minValueForType(FloatFieldImpl::Type value)
{
    static const double Values[] = {
        /* Type::Float */ std::numeric_limits<float>::lowest(),
        /* Type::Double */ std::numeric_limits<double>::lowest(),
    };

    static const std::size_t ValuesSize = std::extent<decltype(Values)>::value;

    static_assert(ValuesSize == util::toUnsigned(FloatFieldImpl::Type::NumOfValues), "Invalid map");

    if (ValuesSize <= util::toUnsigned(value)) {
        assert(!"Mustn't happen");
        value = FloatFieldImpl::Type::Float;
    }

    return Values[util::toUnsigned(value)];
}

double maxValueForType(FloatFieldImpl::Type value)
{
    static const double Values[] = {
        /* Type::Float */ std::numeric_limits<float>::max(),
        /* Type::Double */ std::numeric_limits<double>::max(),
    };

    static const std::size_t ValuesSize = std::extent<decltype(Values)>::value;

    static_assert(ValuesSize == util::toUnsigned(FloatFieldImpl::Type::NumOfValues), "Invalid map");

    if (ValuesSize <= util::toUnsigned(value)) {
        assert(!"Mustn't happen");
        value = FloatFieldImpl::Type::Float;
    }

    return Values[util::toUnsigned(value)];
}

} // namespace

FloatFieldImpl::FloatFieldImpl(xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

FieldImpl::Kind FloatFieldImpl::kindImpl() const
{
    return Kind::Float;
}

FloatFieldImpl::FloatFieldImpl(const FloatFieldImpl&) = default;

FieldImpl::Ptr FloatFieldImpl::cloneImpl() const
{
    return Ptr(new FloatFieldImpl(*this));
}

const XmlWrap::NamesList&FloatFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::typeStr(),
        common::defaultValueStr(),
        common::endianStr(),
        common::validRangeStr(),
        common::validFullRangeStr(),
        common::validValueStr(),
        common::validMinStr(),
        common::validMaxStr()
    };

    return List;
}

const XmlWrap::NamesList&FloatFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::specialStr()
    };

    return List;
}

bool FloatFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateLength() &&
        updateMinMaxValues() &&
        updateDefaultValue() &&
        updateValidRanges() &&
        updateSpecials();
}

std::size_t FloatFieldImpl::lengthImpl() const
{
    return m_length;
}

bool FloatFieldImpl::updateType()
{
    if (!validateSinglePropInstance(common::typeStr(), true)) {
        return false;
    }

    auto propsIter = props().find(common::typeStr());
    assert (propsIter != props().end());

    static const std::string Map[] = {
        /* Type::Float */ "float",
        /* Type::Double */ "double"
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(Type::NumOfValues), "Invalid map");

    auto iter = std::find(std::begin(Map), std::end(Map), propsIter->second);
    if (iter == std::end(Map)) {
        reportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    m_type = static_cast<decltype(m_type)>(std::distance(std::begin(Map), iter));
    return true;
}

bool FloatFieldImpl::updateEndian()
{
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(props(), common::endianStr());
    m_endian = common::parseEndian(endianStr, protocol().schemaImpl().endian());
    if (m_endian == Endian_NumOfValues) {
        reportUnexpectedPropertyValue(common::endianStr(), endianStr);
        return false;
    }

    return true;
}

bool FloatFieldImpl::updateLength()
{
    static const std::size_t Map[] = {
        /* Type::Float */ sizeof(float),
        /* Type::Double */ sizeof(double),
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(Type::NumOfValues), "Invalid map");
    static_assert(sizeof(float) == 4U, "Invalid size assumption");
    static_assert(sizeof(double) == 8U, "Invalid size assumption");

    if (MapSize <= util::toUnsigned(m_type)) {
        assert(!"Mustn't happen");
        return false;
    }

    m_length = Map[util::toUnsigned(m_type)];
    return true;
}

bool FloatFieldImpl::updateMinMaxValues()
{
    m_typeAllowedMinValue = minValueForType(m_type);
    m_typeAllowedMaxValue = maxValueForType(m_type);
    return true;
}

bool FloatFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto& origValueStr = common::getStringProp(props(), common::defaultValueStr());
    if (origValueStr.empty()) {
        return true;
    }

    auto valueStr = common::toLowerCopy(origValueStr);
    bool ok = false;
    m_defaultValue = common::strToDouble(valueStr, &ok);

    if (!ok) {
        reportUnexpectedPropertyValue(common::defaultValueStr(), origValueStr);
        return false;
    }

    bool isSpecial = std::isnan(m_defaultValue) || std::isinf(m_defaultValue);
    if (isSpecial) {
        return true;
    }

    if (m_typeAllowedMaxValue < m_defaultValue) {
        logError() << "Value of property \"" << common::defaultValueStr() <<
                        "\" is greater than the type's maximal value.";
        return false;
    }

    if (m_defaultValue < m_typeAllowedMinValue) {
        logError() << "Value of property \"" << common::defaultValueStr() <<
                        "\" is less than the type's minimal value.";
        return false;
    }

    return true;
}

bool FloatFieldImpl::updateValidRanges()
{
    if (!validateSinglePropInstance(common::validFullRangeStr())) {
        return false;
    }

    auto fullRangeIter = props().find(common::validFullRangeStr());
    if (fullRangeIter != props().end()) {
        bool ok = false;
        bool hasValidFullRange = common::strToBool(fullRangeIter->second, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(common::validFullRangeStr(), fullRangeIter->second);
            return false;
        }

        if (hasValidFullRange) {
            m_validRanges.emplace_back(m_typeAllowedMinValue, m_typeAllowedMaxValue);
        }
    }

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

    assert(std::isinf(-std::numeric_limits<double>::infinity()));
    std::sort(
        m_validRanges.begin(), m_validRanges.end(),
        [](auto& elem1, auto& elem2)
        {
            return compareLess(elem1.first, elem2.first);
        });

    bool intersectingRanges = false;

    std::size_t idx = 0U;
    while ((idx + 2U) <= m_validRanges.size()) {
        auto& thisRange = m_validRanges[idx];
        auto& nextRange = m_validRanges[idx + 1];

        assert(!compareLess(nextRange.first, thisRange.first));
        if (compareLess(thisRange.second, nextRange.first)) {
            ++idx;
            continue;
        }

        if (compareLess(nextRange.first, thisRange.second)) {
            intersectingRanges = true;
        }

        if (compareLess(thisRange.second, nextRange.second)) {
            thisRange.second = nextRange.second;
            assert((!std::isinf(thisRange.second)) || (std::isinf(thisRange.first)));
            assert((!std::isnan(thisRange.second)) || (std::isnan(thisRange.first)));
        }

        m_validRanges.erase(m_validRanges.begin() + idx + 1);
    }

    if (intersectingRanges) {
        logWarning() << XmlWrap::logPrefix(getNode()) << "Some valid values ranges of \"" << name() <<
                        "\" are intersecting.";
    }

    return true;
}

bool FloatFieldImpl::updateSpecials()
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

        auto specialsIter = m_specials.find(nameIter->second);
        if (specialsIter != m_specials.end()) {
            logError() << XmlWrap::logPrefix(s) << "Special with name \"" << nameIter->second <<
                          "\" was already assigned to \"" << name() << "\" element.";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        bool ok = false;
        double val = common::strToDouble(valIter->second, &ok);
        if (!ok) {
            logError() << XmlWrap::logPrefix(s) << "Special value \"" << nameIter->second <<
                          "\" cannot be recognized.";
            return false;
        }

        bool isSpecial = std::isnan(val) || std::isinf(val);
        if ((!isSpecial) &&
            ((val < m_typeAllowedMinValue) || (m_typeAllowedMaxValue < val))) {
            this->logError() << XmlWrap::logPrefix(s) <<
                            "Special value \"" << nameIter->second << "\" is outside the range of available values within a type.";
            return false;
        }

        m_specials.emplace(nameIter->second, val);
    }

    return true;
}

bool FloatFieldImpl::validateValidRangeStr(const std::string& str)
{
    bool ok = false;
    auto range = common::parseRange(str, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validRangeStr(), str);
        return false;
    }

    auto minVal = common::strToDouble(range.first, &ok, false);
    if (!ok) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid min value in valid range (" << str << ").";
        return false;
    }

    auto maxVal = common::strToDouble(range.second, &ok, false);
    if (!ok) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid max value in valid range (" << str << ").";
        return false;
    }

    if (maxVal < minVal) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Min value must be less than max in valid range (" << str << ").";
        return false;
    }

    m_validRanges.emplace_back(minVal, maxVal);
    return true;
}

bool FloatFieldImpl::validateValidValueStr(const std::string& str)
{
    bool ok = false;
    double val = common::strToDouble(str, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validValueStr(), str);
        return false;
    }

    m_validRanges.emplace_back(val, val);
    return true;
}

bool FloatFieldImpl::validateValidMinValueStr(const std::string& str)
{
    bool ok = false;
    double val = common::strToDouble(str, &ok, false);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validMinStr(), str);
        return false;
    }

    if (m_typeAllowedMaxValue < val) {
        logError() << "Value of property \"" << common::validMinStr() <<
                        "\" is greater than the type's maximal value.";
        return false;
    }

    if (val < m_typeAllowedMinValue) {
        logError() << "Value of property \"" << common::validMinStr() <<
                        "\" is less than the type's minimal value.";
        return false;
    }


    m_validRanges.emplace_back(val, m_typeAllowedMaxValue);
    return true;
}

bool FloatFieldImpl::validateValidMaxValueStr(const std::string& str)
{
    bool ok = false;
    double val = common::strToDouble(str, &ok, false);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validMaxStr(), str);
        return false;
    }

    if (m_typeAllowedMaxValue < val) {
        logError() << "Value of property \"" << common::validMaxStr() <<
                        "\" is greater than the type's maximal value.";
        return false;
    }

    if (val < m_typeAllowedMinValue) {
        logError() << "Value of property \"" << common::validMaxStr() <<
                        "\" is less than the type's minimal value.";
        return false;
    }

    m_validRanges.emplace_back(m_typeAllowedMinValue, val);
    return true;
}

} // namespace bbmp
