//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "FloatFieldImpl.h"

#include <algorithm>
#include <iterator>
#include <cassert>
#include <limits>
#include <cmath>
#include <map>

#include "common.h"
#include "util.h"
#include "ProtocolImpl.h"

namespace commsdsl
{

namespace parse
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
        /* Type::Float */ double(std::numeric_limits<float>::lowest()),
        /* Type::Double */ std::numeric_limits<double>::lowest(),
    };

    static const std::size_t ValuesSize = std::extent<decltype(Values)>::value;

    static_assert(ValuesSize == util::toUnsigned(FloatFieldImpl::Type::NumOfValues), "Invalid map");

    if (ValuesSize <= util::toUnsigned(value)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        value = FloatFieldImpl::Type::Float;
    }

    return Values[util::toUnsigned(value)];
}

double maxValueForType(FloatFieldImpl::Type value)
{
    static const double Values[] = {
        /* Type::Float */ double(std::numeric_limits<float>::max()),
        /* Type::Double */ std::numeric_limits<double>::max(),
    };

    static const std::size_t ValuesSize = std::extent<decltype(Values)>::value;

    static_assert(ValuesSize == util::toUnsigned(FloatFieldImpl::Type::NumOfValues), "Invalid map");

    if (ValuesSize <= util::toUnsigned(value)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        value = FloatFieldImpl::Type::Float;
    }

    return Values[util::toUnsigned(value)];
}

} // namespace

FloatFieldImpl::FloatFieldImpl(xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
    m_state.m_nonUniqueSpecialsAllowed = !protocol.isNonUniqueSpecialsAllowedSupported();
}

bool FloatFieldImpl::hasNonUniqueSpecials() const
{
    if (!m_state.m_nonUniqueSpecialsAllowed) {
        return false;
    }


    std::vector<double> specValues;
    specValues.reserve(m_state.m_specials.size());

    for (auto& s : m_state.m_specials) {
        if (std::isnan(s.second.m_value)) {
            continue;
        }

        specValues.push_back(s.second.m_value);
    }

    if ((specValues.size() + 1U) < m_state.m_specials.size()) {
        // More than one NaN inside
        return true;
    }

    std::sort(specValues.begin(), specValues.end());

    bool firstValue = true;
    double prevValue = 0.0;
    for (auto s : specValues) {
        if ((!firstValue) && (prevValue == s)) {
            return true;
        }
        firstValue = false;
        prevValue = s;
    }
    return false;
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
        common::validMaxStr(),
        common::validCheckVersionStr(),
        common::unitsStr(),
        common::displayDesimalsStr(),
        common::nonUniqueSpecialsAllowedStr(),
        common::displaySpecialsStr(),
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

bool FloatFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const FloatFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool FloatFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateLength() &&
        updateMinMaxValues() &&
        updateNonUniqueSpecialsAllowed() &&
        updateSpecials() &&
        updateDefaultValue() &&
        updateValidCheckVersion() &&
        updateValidRanges() &&
        updateUnits() &&
        updateDisplayDecimals() &&
        updateDisplaySpecials();
}

std::size_t FloatFieldImpl::minLengthImpl() const
{
    return m_state.m_length;
}

bool FloatFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    double value = 0.0;
    return strToValue(val, value, true);
}

bool FloatFieldImpl::strToFpImpl(const std::string& ref, double& val) const
{
    if (!protocol().isFieldValueReferenceSupported()) {
        return false;
    }

    if (ref.empty()) {
        val = m_state.m_defaultValue;
        return true;
    }

    auto iter = m_state.m_specials.find(ref);
    if (iter == m_state.m_specials.end()) {
        return false;
    }

    val = iter->second.m_value;
    return true;
}

FloatFieldImpl::FieldRefInfo FloatFieldImpl::processInnerRefImpl(const std::string& refStr) const
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

bool FloatFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_InnerValue);
}

bool FloatFieldImpl::updateType()
{
    bool mustHave = (m_state.m_type == Type::NumOfValues);
    if (!validateSinglePropInstance(common::typeStr(), mustHave)) {
        return false;
    }

    auto propsIter = props().find(common::typeStr());
    if (propsIter == props().end()) {
        assert(!mustHave);
        return true;
    }

    static const std::string Map[] = {
        /* Type::Float */ "float",
        /* Type::Double */ "double"
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;

    static_assert(MapSize == util::toUnsigned(Type::NumOfValues), "Invalid map");

    auto typeStr = common::toLowerCopy(propsIter->second);
    auto iter = std::find(std::begin(Map), std::end(Map), typeStr);
    if (iter == std::end(Map)) {
        reportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    auto newType = static_cast<decltype(m_state.m_type)>(std::distance(std::begin(Map), iter));
    if (m_state.m_type == Type::NumOfValues) {
        m_state.m_type = newType;
        return true;
    }

    if (newType == m_state.m_type) {
        return true;
    }

    logError() << XmlWrap::logPrefix(getNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool FloatFieldImpl::updateEndian()
{
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(props(), common::endianStr());
    m_state.m_endian = common::parseEndian(endianStr, protocol().currSchema().endian());
    if (m_state.m_endian == Endian_NumOfValues) {
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

    if (MapSize <= util::toUnsigned(m_state.m_type)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return false;
    }

    m_state.m_length = Map[util::toUnsigned(m_state.m_type)];
    return true;
}

bool FloatFieldImpl::updateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = minValueForType(m_state.m_type);
    m_state.m_typeAllowedMaxValue = maxValueForType(m_state.m_type);
    return true;
}

bool FloatFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::defaultValueStr());
    if (valueStr.empty()) {
        return true;
    }

    if (!strToValue(valueStr, m_state.m_defaultValue)) {
        reportUnexpectedPropertyValue(common::defaultValueStr(), valueStr);
        return false;
    }

    bool isSpecial = std::isnan(m_state.m_defaultValue) || std::isinf(m_state.m_defaultValue);
    if (isSpecial) {
        return true;
    }

    if (m_state.m_typeAllowedMaxValue < m_state.m_defaultValue) {
        logError() << "Value of property \"" << common::defaultValueStr() <<
                        "\" is greater than the type's maximal value.";
        return false;
    }

    if (m_state.m_defaultValue < m_state.m_typeAllowedMinValue) {
        logError() << "Value of property \"" << common::defaultValueStr() <<
                        "\" is less than the type's minimal value.";
        return false;
    }

    return true;
}

bool FloatFieldImpl::updateValidCheckVersion()
{
    return validateAndUpdateBoolPropValue(common::validCheckVersionStr(), m_state.m_validCheckVersion);
}

bool FloatFieldImpl::updateValidRanges()
{
    auto attrs = XmlWrap::parseNodeProps(getNode());
    bool result =
        checkFullRangeProps(attrs) &&
        checkValidRangeProps(attrs) &&
        checkValidValueProps(attrs) &&
        checkValidMinProps(attrs) &&
        checkValidMaxProps(attrs);
    if (!result) {
        return false;
    }

    // sort by version
    assert(std::isinf(-std::numeric_limits<double>::infinity()));
    std::sort(
        m_state.m_validRanges.begin(), m_state.m_validRanges.end(),
        [](auto& elem1, auto& elem2)
        {
            assert(elem1.m_deprecatedSince != 0U);
            assert(elem2.m_deprecatedSince != 0U);
            if (elem1.m_sinceVersion != elem2.m_sinceVersion) {
                return elem1.m_sinceVersion < elem2.m_sinceVersion;
            }

            if (elem1.m_deprecatedSince != elem2.m_deprecatedSince) {
                return elem1.m_deprecatedSince < elem2.m_deprecatedSince;
            }

            if (compareLess(elem1.m_min, elem2.m_min)) {
                return true;
            }

            if (compareLess(elem2.m_min, elem1.m_min)) {
                return false;
            }

            return compareLess(elem1.m_max, elem2.m_max);
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

            auto isSpecial =
                    [](double val)
                    {
                        return std::isinf(val) || std::isnan(val);
                    };

            if ((iter->m_sinceVersion != nextIter->m_sinceVersion) ||
                (iter->m_deprecatedSince != nextIter->m_deprecatedSince) ||
                (isSpecial(iter->m_max)) ||
                (isSpecial(nextIter->m_min)) ||
                (compareLess(iter->m_max, nextIter->m_min))) {
                break;
            }

            assert(!isSpecial(nextIter->m_max));
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
        [](auto& elem1, auto& elem2)
        {
            assert(elem1.m_deprecatedSince != 0U);
            assert(elem2.m_deprecatedSince != 0U);
            if (compareLess(elem1.m_min, elem2.m_min)) {
                return true;
            }

            if (compareLess(elem2.m_min, elem1.m_min)) {
                return false;
            }

            if (compareLess(elem1.m_max, elem2.m_max)) {
                return true;
            }

            if (compareLess(elem2.m_max, elem1.m_max)) {
                return false;
            }

            if (elem1.m_sinceVersion != elem2.m_sinceVersion) {
                return elem1.m_sinceVersion < elem2.m_sinceVersion;
            }

            assert(elem1.m_deprecatedSince != elem2.m_deprecatedSince);
            return elem1.m_deprecatedSince < elem2.m_deprecatedSince;
        });

    return true;
}

bool FloatFieldImpl::updateNonUniqueSpecialsAllowed()
{
    return validateAndUpdateBoolPropValue(common::nonUniqueSpecialsAllowedStr(), m_state.m_nonUniqueSpecialsAllowed);
}

bool FloatFieldImpl::updateSpecials()
{
    auto specials = XmlWrap::getChildren(getNode(), common::specialStr());

    using RecSpecials = std::multimap<double, std::string>;
    RecSpecials recSpecials;
    std::string prevNanSpecial;

    for (auto* s : specials) {
        static const XmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::valStr(),
            common::sinceVersionStr(),
            common::deprecatedStr(),
            common::descriptionStr(),
            common::displayNameStr()
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

        if (!XmlWrap::validateSinglePropInstance(s, props, common::sinceVersionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(s, props, common::deprecatedStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(s, props, common::descriptionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(s, props, common::displayNameStr(), protocol().logger())) {
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

        double val = 0.0;
        if (!strToValue(valIter->second, val)) {
            logError() << XmlWrap::logPrefix(s) <<
                "Value of \"" << nameIter->second << "\" (" << valIter->second << ") cannot be recognized.";
            return false;
        }

        if (!m_state.m_nonUniqueSpecialsAllowed) {
            bool reportError = false;
            const std::string* prevDefName = nullptr;
            do {
                if (std::isnan(val)) {
                    reportError = !prevNanSpecial.empty();
                    prevNanSpecial = nameIter->second;
                    prevDefName = &prevNanSpecial;
                    break;
                }

                auto recIter = recSpecials.find(val);
                if (recIter != recSpecials.end()) {
                    reportError = true;
                    prevDefName = &(recIter->second);
                }
                else {
                    recSpecials.insert(std::make_pair(val, nameIter->second));
                }
            } while (false);

            if (reportError) {
                assert(prevDefName != nullptr);
                logError() << XmlWrap::logPrefix(s) <<
                    "Value of special \"" << nameIter->second <<
                    "\" (" << valIter->second << ") has already been defined as \"" <<
                    *prevDefName << "\".";
                return false;
            }
        }

        bool isSpecial = std::isnan(val) || std::isinf(val);
        if ((!isSpecial) &&
            ((val < m_state.m_typeAllowedMinValue) || (m_state.m_typeAllowedMaxValue < val))) {
            this->logError() << XmlWrap::logPrefix(s) <<
                "Special value \"" << nameIter->second << "\" (" <<
                valIter->second << ") is outside the range of available values within a type.";
            return false;
        }

        SpecialValueInfo info;
        info.m_value = val;
        info.m_sinceVersion = getSinceVersion();
        info.m_deprecatedSince = getDeprecated();

        if (!XmlWrap::getAndCheckVersions(s, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
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

bool FloatFieldImpl::updateUnits()
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

bool FloatFieldImpl::updateDisplayDecimals()
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

bool FloatFieldImpl::updateDisplaySpecials()
{
    checkAndReportDeprecatedPropertyValue(common::displaySpecialsStr());
    return true;
}

bool FloatFieldImpl::checkFullRangeAsAttr(const FieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validFullRangeStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    bool ok = false;
    bool fullRange = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validFullRangeStr(), iter->second);
        return false;
    }

    if (!fullRange) {
        return true;
    }

    ValidRangeInfo info;
    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();
    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkFullRangeAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!XmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    bool ok = false;
    bool fullRange = common::strToBool(str, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validFullRangeStr(), str);
        return false;
    }

    if (!fullRange) {
        return true;
    }

    ValidRangeInfo info;
    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!XmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkFullRangeProps(const FieldImpl::PropsMap& xmlAttrs)
{
    if (!validateSinglePropInstance(common::validFullRangeStr())) {
        return false;
    }

    if (!checkFullRangeAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = XmlWrap::getChildren(getNode(), common::validFullRangeStr());
    for (auto* c : children) {
        if (!checkFullRangeAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool FloatFieldImpl::checkValidRangeAsAttr(const FieldImpl::PropsMap& xmlAttrs)
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

bool FloatFieldImpl::checkValidRangeAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!XmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!validateValidRangeStr(str, info.m_min, info.m_max)) {
        return false;
    }

    if (!XmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkValidRangeProps(const FieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidRangeAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = XmlWrap::getChildren(getNode(), common::validRangeStr());
    for (auto* c : children) {
        if (!checkValidRangeAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool FloatFieldImpl::checkValidValueAsAttr(const FieldImpl::PropsMap& xmlAttrs)
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

bool FloatFieldImpl::checkValidValueAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!XmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!validateValidValueStr(str, common::validValueStr(), info.m_min)) {
        return false;
    }

    info.m_max = info.m_min;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!XmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkValidValueProps(const FieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidValueAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = XmlWrap::getChildren(getNode(), common::validValueStr());
    for (auto* c : children) {
        if (!checkValidValueAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool FloatFieldImpl::checkValidMinAsAttr(const FieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validMinStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!validateValidValueStr(iter->second, common::validMinStr(), info.m_min, false)) {
        return false;
    }

    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkValidMinAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!XmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!validateValidValueStr(str, common::validMinStr(), info.m_min, false)) {
        return false;
    }

    info.m_max = m_state.m_typeAllowedMaxValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!XmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkValidMinProps(const FieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidMinAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = XmlWrap::getChildren(getNode(), common::validMinStr());
    for (auto* c : children) {
        if (!checkValidMinAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool FloatFieldImpl::checkValidMaxAsAttr(const FieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validMaxStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    ValidRangeInfo info;
    if (!validateValidValueStr(iter->second, common::validMaxStr(), info.m_max, false)) {
        return false;
    }

    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkValidMaxAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!XmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    ValidRangeInfo info;

    if (!validateValidValueStr(str, common::validMaxStr(), info.m_max, false)) {
        return false;
    }

    info.m_min = m_state.m_typeAllowedMinValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!XmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validRanges.push_back(info);
    return true;
}

bool FloatFieldImpl::checkValidMaxProps(const FieldImpl::PropsMap& xmlAttrs)
{
    if (!checkValidMaxAsAttr(xmlAttrs)) {
        return false;
    }

    auto children = XmlWrap::getChildren(getNode(), common::validMaxStr());
    for (auto* c : children) {
        if (!checkValidMaxAsChild(c)) {
            return false;
        }
    }

    return true;
}

bool FloatFieldImpl::validateValidRangeStr(const std::string& str, double& min, double& max)
{
    bool ok = false;
    auto range = common::parseRange(str, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validRangeStr(), str);
        return false;
    }

    if (!strToValue(range.first, min, false)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid min value in valid range (" << str << ").";
        return false;
    }

    if (!strToValue(range.second, max, false)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid max value in valid range (" << str << ").";
        return false;
    }

    if (max < min) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Min value must be less than max in valid range (" << str << ").";
        return false;
    }

    return true;
}

bool FloatFieldImpl::validateValidValueStr(
    const std::string& str,
    const std::string& type,
    double& val,
    bool allowSpecials)
{
    if (!strToValue(str, val, allowSpecials)) {
        reportUnexpectedPropertyValue(type, str);
        return false;
    }

    if (std::isnan(val) || std::isinf(val)) {
        return true;
    }

    if (m_state.m_typeAllowedMaxValue < val) {
        logError() << "Value of property \"" << type <<
                        "\" is greater than the type's maximal value.";
        return false;
    }

    if (val < m_state.m_typeAllowedMinValue) {
        logError() << "Value of property \"" << type <<
                        "\" is less than the type's minimal value.";
        return false;
    }

    return true;
}

bool FloatFieldImpl::strToValue(
    const std::string& str,
    double& val,
    bool allowSpecials) const
{
    auto strCpy = common::toLowerCopy(str);
    do {
        if (allowSpecials && common::isFpSpecial(strCpy)) {
            break;
        }

        if (common::isValidName(str)) {
            auto iter = m_state.m_specials.find(str);
            if (iter != m_state.m_specials.end()) {
                val = iter->second.m_value;
                return true;
            }
        }

        if (common::isValidRefName(str)) {

             if (!protocol().strToFp(str, false, val)) {
                 return false;
             }

             return true;
         }

    } while (false);

    bool ok = false;
    val = common::strToDouble(strCpy, &ok, allowSpecials);
    return ok;
}

} // namespace parse

} // namespace commsdsl
