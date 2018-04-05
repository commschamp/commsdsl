#include "EnumFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "util.h"

namespace bbmp
{

namespace
{

} // namespace

EnumFieldImpl::EnumFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

bool EnumFieldImpl::isUnique() const
{
    std::intmax_t prevKey = 0;
    bool firstElem = true;
    for (auto& v : m_state.m_revValues) {
        if (firstElem) {
            prevKey = v.first;
            firstElem = false;
            continue;
        }

        if (prevKey == v.first) {
            assert(isNonUniqueAllowed());
            return false;
        }

        prevKey = v.first;
    }

    return true;
}

FieldImpl::Kind EnumFieldImpl::kindImpl() const
{
    return Kind::Enum;
}

EnumFieldImpl::EnumFieldImpl(const EnumFieldImpl&) = default;

FieldImpl::Ptr EnumFieldImpl::cloneImpl() const
{
    return Ptr(new EnumFieldImpl(*this));
}

const XmlWrap::NamesList& EnumFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::typeStr(),
        common::defaultValueStr(),
        common::endianStr(),
        common::lengthStr(),
        common::bitLengthStr(),
    };

    return List;
}

const XmlWrap::NamesList& EnumFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::validValueStr()
    };

    return List;
}

bool EnumFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const EnumFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool EnumFieldImpl::parseImpl()
{
    return
        updateType() &&
        updateEndian() &&
        updateLength() &&
        updateBitLength() &&
        updateNonUniqueAllowed() &&
        updateMinMaxValues() &&
        updateValues() &&
        updateDefaultValue();
}

std::size_t EnumFieldImpl::lengthImpl() const
{
    return m_state.m_length;
}

std::size_t EnumFieldImpl::bitLengthImpl() const
{
    return m_state.m_bitLength;
}

bool EnumFieldImpl::updateType()
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

    auto newType = IntFieldImpl::parseTypeValue(propsIter->second);
    if (newType == Type::NumOfValues) {
        reportUnexpectedPropertyValue(common::typeStr(), propsIter->second);
        return false;
    }

    if (mustHave) {
        m_state.m_type = newType;
        return true;
    }

    if (m_state.m_type == newType) {
        return true;
    }

    logError() << XmlWrap::logPrefix(getNode()) <<
                  "Type cannot be changed after reuse";
    return false;
}

bool EnumFieldImpl::updateEndian()
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

bool EnumFieldImpl::updateLength()
{
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto maxLength = IntFieldImpl::maxTypeLength(m_state.m_type);
    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    if (lengthStr.empty()) {
        if (m_state.m_length == 0) {
            m_state.m_length = maxLength;
            return true;
        }

        assert(m_state.m_length <= IntFieldImpl::maxTypeLength(m_state.m_type));
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
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Length cannot be changed after reuse";
        return false;
    }

    m_state.m_length = newLength;

    assert(0U < maxLength);
    if (maxLength < m_state.m_length) {
        logError() << XmlWrap::logPrefix(getNode()) << "Length of the \"" << name() << "\" element (" << lengthStr << ") cannot exceed "
                      "max length allowed by the type (" << maxLength << ").";
        return false;
    }

    assert (m_state.m_length != 0U);
    return true;
}

bool EnumFieldImpl::updateBitLength()
{
    if (!validateSinglePropInstance(common::bitLengthStr())) {
        return false;
    }

    static const std::size_t BitsInByte =
         std::numeric_limits<std::uint8_t>::digits;
    static_assert(BitsInByte == 8U, "Invalid assumption");

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

bool EnumFieldImpl::updateNonUniqueAllowed()
{
    if (!validateSinglePropInstance(common::nonUniqueAllowedStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::nonUniqueAllowedStr());
    if (valueStr.empty()) {
        return true;
    }

    bool ok = false;
    m_state.m_nonUniqueAllowed = common::strToBool(valueStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::nonUniqueAllowedStr(), valueStr);
        return false;
    }

    return true;
}

bool EnumFieldImpl::updateMinMaxValues()
{
    m_state.m_typeAllowedMinValue = IntFieldImpl::minTypeValue(m_state.m_type);
    m_state.m_typeAllowedMaxValue = IntFieldImpl::maxTypeValue(m_state.m_type);

    m_state.m_minValue = IntFieldImpl::calcMinValue(m_state.m_type, m_state.m_bitLength);
    m_state.m_maxValue = IntFieldImpl::calcMaxValue(m_state.m_type, m_state.m_bitLength);

    return true;
}

bool EnumFieldImpl::updateValues()
{
    auto validValues = XmlWrap::getChildren(getNode(), common::validValueStr());
    if (validValues.empty()) {
        if (!m_state.m_values.empty()) {
            assert(!m_state.m_revValues.empty());
            return true; // already has values
        }

        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The enum \"" << name() << "\" doesn't list any valid value.";
        return false;
    }

    for (auto* vNode : validValues) {
        static const XmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::valStr()
        };

        auto props = XmlWrap::parseNodeProps(vNode);
        if (!XmlWrap::parseChildrenAsProps(vNode, PropNames, protocol().logger(), props)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::nameStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::valStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::sinceVersionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(vNode, props, common::deprecatedStr(), protocol().logger())) {
            return false;
        }

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << XmlWrap::logPrefix(vNode) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto valuesIter = m_state.m_values.find(nameIter->second);
        if (valuesIter != m_state.m_values.end()) {
            logError() << XmlWrap::logPrefix(vNode) << "Value with name \"" << nameIter->second <<
                          "\" has already been defined for enum \"" << name() << "\".";
            return false;
        }

        auto valIter = props.find(common::valStr());
        assert(valIter != props.end());

        std::intmax_t val = 0;
        if (!strToNumeric(valIter->second, val)) {
            logError() << XmlWrap::logPrefix(vNode) << "Value of \"" << nameIter->second <<
                          "\" cannot be recognized.";
            return false;
        }

        auto checkValueInRangeFunc =
            [this, vNode, &nameIter](auto v) -> bool
            {
                if ((v < static_cast<decltype(v)>(m_state.m_typeAllowedMinValue)) ||
                    (static_cast<decltype(v)>(m_state.m_typeAllowedMaxValue) < v)) {
                    this->logError() << XmlWrap::logPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of available values within a type.";
                    return false;
                }

                if ((v < static_cast<decltype(v)>(m_state.m_minValue)) ||
                    (static_cast<decltype(v)>(m_state.m_maxValue) < v)) {
                    this->logWarning() << XmlWrap::logPrefix(vNode) <<
                                    "Valid value \"" << nameIter->second << "\" is outside the range of correctly serializable values.";
                }

                return true;
            };

        bool checkResult = false;
        if (IntFieldImpl::isBigUnsigned(m_state.m_type)) {
            checkResult = checkValueInRangeFunc(static_cast<std::uintmax_t>(val));
        }
        else {
            checkResult = checkValueInRangeFunc(val);
        }

        if (!checkResult) {
            return false;
        }

        if (!m_state.m_nonUniqueAllowed) {
            auto revIter = m_state.m_revValues.find(val);
            if (revIter != m_state.m_revValues.end()) {
                logError() << XmlWrap::logPrefix(vNode) <<
                              "Value \"" << valIter->second << "\" has been already defined "
                              "as \"" << revIter->second << "\".";
                return false;
            }
        }

        ValueInfo info;
        info.m_value = val;

        auto sinceVerIter = props.find(common::sinceVersionStr());
        do {
            if (sinceVerIter == props.end()) {
                info.m_sinceVersion = getMaxSinceVersion();
                assert(info.m_sinceVersion <= protocol().schemaImpl().version());
                break;
            }

            auto& sinceVerStr = sinceVerIter->second;
            bool ok = false;
            info.m_sinceVersion = common::strToUnsigned(sinceVerStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(vNode, nameIter->second, common::sinceVersionStr(), sinceVerStr, protocol().logger());
                return false;
            }

            if (protocol().schemaImpl().version() < info.m_sinceVersion) {
                logError() << XmlWrap::logPrefix(vNode) <<
                              "The value of \"" << common::sinceVersionStr() << "\" property cannot "
                              "be greater than value of \"" << common::versionStr() << "\" property of the schema.";
                return false;
            }

        } while (false);

        auto deprecatedIter = props.find(common::deprecatedStr());
        do {
            if (deprecatedIter == props.end()) {
                info.m_deprecatedSince = getDeprecated();
                assert(info.m_sinceVersion < info.m_deprecatedSince);
                break;
            }

            auto& deprecatedStr = deprecatedIter->second;
            bool ok = false;
            info.m_deprecatedSince = common::strToUnsigned(deprecatedStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(vNode, nameIter->second, common::deprecatedStr(), deprecatedStr, protocol().logger());
                return false;
            }

            if (info.m_deprecatedSince <= info.m_sinceVersion) {
                logError() << XmlWrap::logPrefix(vNode) <<
                              "The value of \"" << common::deprecatedStr() << "\" property must "
                              "be greater than value of \"" << common::sinceVersionStr() << "\" property of the value.";
                return false;
            }

        } while (false);

        m_state.m_values.emplace(nameIter->second, info);
        m_state.m_revValues.emplace(val, nameIter->second);
    }
    return true;
}

bool EnumFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::defaultValueStr());
    if (valueStr.empty()) {
        return true;
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
        logError() << XmlWrap::logPrefix(getNode()) << "Default value (" << valueStr <<
                      ") cannot be recognized.";
        return false;
    }

    if (IntFieldImpl::isBigUnsigned(m_state.m_type)) {
        return checkValueFunc(static_cast<std::uintmax_t>(val));
    }

    return checkValueFunc(val);
}

bool EnumFieldImpl::strToNumeric(const std::string& str, std::intmax_t& val)
{
    if (common::isValidName(str)) {
        // Check among specials
        auto iter = m_state.m_values.find(str);
        if (iter == m_state.m_values.end()) {
            return false;
        }

        val = iter->second.m_value;
        return true;
    }

    if (common::isValidRefName(str)) {
        return protocol().strToEnumValue(str, val, false);
    }

    bool ok = false;
    if (IntFieldImpl::isBigUnsigned(m_state.m_type)) {
        val = static_cast<std::intmax_t>(common::strToUintMax(str, &ok));
    }
    else {
        val = common::strToIntMax(str, &ok);
    }
    return ok;
}


} // namespace bbmp
