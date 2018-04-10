#include "SetFieldImpl.h"

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

SetFieldImpl::SetFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

SetFieldImpl::SetFieldImpl(const SetFieldImpl&) = default;

bool SetFieldImpl::isUnique() const
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
            assert(isNonUniqueAllowed());
            return false;
        }

        prevIdx = b.first;
    }

    return true;
}

FieldImpl::Kind SetFieldImpl::kindImpl() const
{
    return Kind::Set;
}

FieldImpl::Ptr SetFieldImpl::cloneImpl() const
{
    return Ptr(new SetFieldImpl(*this));
}

const XmlWrap::NamesList& SetFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::defaultValueStr(),
        common::endianStr(),
        common::lengthStr(),
        common::bitLengthStr(),
        common::reservedValueStr(),
        common::validCheckVersionStr()
    };

    return List;
}

const XmlWrap::NamesList& SetFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::bitStr()
    };

    return List;
}

bool SetFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const SetFieldImpl&>(other);
    m_state = castedOther.m_state;
    return true;
}

bool SetFieldImpl::parseImpl()
{
    return
        updateEndian() &&
        updateType() &&
        updateLength() &&
        updateNonUniqueAllowed() &&
        updateValidCheckVersion() &&
        updateDefaultValue() &&
        updateReservedValue() &&
        updateBits();
}

std::size_t SetFieldImpl::lengthImpl() const
{
    return m_state.m_length;
}

std::size_t SetFieldImpl::bitLengthImpl() const
{
    return m_state.m_bitLength;
}

bool SetFieldImpl::updateEndian()
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

bool SetFieldImpl::updateType()
{
    if (!validateSinglePropInstance(common::typeStr())) {
        return false;
    }

    auto typeIter = props().find(common::typeStr());
    if (typeIter == props().end()) {
        return true;
    }

    auto typeVal = IntFieldImpl::parseTypeValue(typeIter->second);
    if ((Type::NumOfValues <= typeVal) ||
        (!IntFieldImpl::isTypeUnsigned(typeVal))) {
        reportUnexpectedPropertyValue(common::typeStr(), typeIter->second);
        return false;
    }

    if (m_state.m_type == typeVal) {
        return true;
    }

    if (m_state.m_type != Type::NumOfValues) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Type cannot be changed after reuse";
        return false;
    }

    m_state.m_type = typeVal;
    return true;
}

bool SetFieldImpl::updateLength()
{
    bool mustHaveLength =
            (m_state.m_type == Type::NumOfValues) &&
            (m_state.m_length == 0U) &&
            (!isBitfieldMember());
    if (!validateSinglePropInstance(common::lengthStr(), mustHaveLength)) {
        return false;
    }

    std::size_t maxLength = sizeof(std::uint64_t);
    if (m_state.m_type != Type::NumOfValues) {
        maxLength = IntFieldImpl::maxTypeLength(m_state.m_type);
    }

    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    do {
        if (lengthStr.empty()) {
            if ((m_state.m_length == 0U) && (m_state.m_type != Type::NumOfValues)) {
                m_state.m_length = maxLength;
            }
            break;
        }

        bool ok = false;
        auto newLen = static_cast<decltype(m_state.m_length)>(common::strToUintMax(lengthStr, &ok));

        if ((!ok) || (newLen == 0U) || (maxLength < newLen)) {
            reportUnexpectedPropertyValue(common::lengthStr(), lengthStr);
            return false;
        }

        if (m_state.m_length == newLen) {
            break;
        }

        if (m_state.m_length != 0U) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "Length cannot be changed after reuse";
            return false;
        }

        m_state.m_length = newLen;
    } while (false);

    bool mustHaveBitLength = (isBitfieldMember() && m_state.m_bitLength == 0U && (m_state.m_length == 0U));
    if (!validateSinglePropInstance(common::bitLengthStr(), mustHaveBitLength)) {
        return false;
    }

    auto& bitLengthStr = common::getStringProp(props(), common::bitLengthStr());
    do {
        if (bitLengthStr.empty()) {
            if (m_state.m_bitLength == 0U) {
                assert(m_state.m_length != 0U);
                m_state.m_bitLength = m_state.m_length * 8U;
            }
            break;
        }

        if (!isBitfieldMember()) {
            logWarning() << XmlWrap::logPrefix((getNode())) <<
                            "The property \"" << common::bitLengthStr() << "\" is "
                            "applicable only to the members of \"" << common::bitfieldStr() << "\"";
            assert(m_state.m_length != 0U);
            m_state.m_bitLength = m_state.m_length * 8U;
            break;
        }

        bool ok = false;
        m_state.m_bitLength = static_cast<decltype(m_state.m_bitLength)>(common::strToUintMax(lengthStr, &ok));
        if ((!ok) || (m_state.m_bitLength == 0U)) {
            reportUnexpectedPropertyValue(common::bitLengthStr(), bitLengthStr);
            return false;
        }

        if ((m_state.m_length != 0) && ((m_state.m_length * 8U) < m_state.m_bitLength)) {
            reportUnexpectedPropertyValue(common::bitLengthStr(), bitLengthStr);
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

        assert(m_state.m_length < MapSize);
        m_state.m_type = Map[m_state.m_length - 1];
    }

    assert(m_state.m_type != Type::NumOfValues);
    assert(m_state.m_length != 0U);
    assert(m_state.m_bitLength != 0U);
    return true;
}

bool SetFieldImpl::updateNonUniqueAllowed()
{
    if (!validateSinglePropInstance(common::nonUniqueAllowedStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::nonUniqueAllowedStr());
    if (valueStr.empty()) {
        return true;
    }

    bool wasAllowed = m_state.m_nonUniqueAllowed;
    bool ok = false;
    bool newAllowed = common::strToBool(valueStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::nonUniqueAllowedStr(), valueStr);
        return false;
    }

    if (wasAllowed && (!newAllowed) && (!isUnique())) {
        logError() << "Cannot clear \"" << common::nonUniqueAllowedStr() << "\" property value "
                      "while having multiple names for the same bit(s).";
        return false;
    }

    m_state.m_nonUniqueAllowed = newAllowed;
    return true;
}

bool SetFieldImpl::updateValidCheckVersion()
{
    if (!validateSinglePropInstance(common::validCheckVersionStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::validCheckVersionStr());
    if (valueStr.empty()) {
        return true;
    }

    bool ok = false;
    m_state.m_validCheckVersion = common::strToBool(valueStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::validCheckVersionStr(), valueStr);
        return false;
    }

    return true;
}

bool SetFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), common::defaultValueStr());
    if (valueStr.empty()) {
        return true;
    }

    bool ok = false;
    m_state.m_defaultBitValue = common::strToBool(valueStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::defaultValueStr(), valueStr);
        return false;
    }

    return true;
}

bool SetFieldImpl::updateReservedValue()
{
    if (!validateSinglePropInstance(common::reservedValueStr())) {
        return false;
    }

    auto valueStr = common::getStringProp(props(), common::reservedValueStr());
    if (valueStr.empty()) {
        return true;
    }

    bool ok = false;
    m_state.m_reservedBitValue = common::strToBool(valueStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::reservedValueStr(), valueStr);
        return false;
    }

    return true;
}

bool SetFieldImpl::updateBits()
{
    auto bits = XmlWrap::getChildren(getNode(), common::bitStr());
    if (bits.empty()) {
        if (!m_state.m_bits.empty()) {
            assert(!m_state.m_revBits.empty());
            return true; // already has values
        }

        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The set \"" << name() << "\" doesn't list any bits.";
        return false;
    }

    for (auto* b : bits) {
        static const XmlWrap::NamesList PropNames = {
            common::nameStr(),
            common::idxStr(),
            common::defaultValueStr(),
            common::reservedValueStr(),
            common::reservedStr(),
            common::sinceVersionStr(),
            common::deprecatedStr()
        };

        auto props = XmlWrap::parseNodeProps(b);
        if (!XmlWrap::parseChildrenAsProps(b, PropNames, protocol().logger(), props)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::nameStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::idxStr(), protocol().logger(), true)) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::defaultValueStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::reservedValueStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::reservedStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::sinceVersionStr(), protocol().logger())) {
            return false;
        }

        if (!XmlWrap::validateSinglePropInstance(b, props, common::deprecatedStr(), protocol().logger())) {
            return false;
        }

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << XmlWrap::logPrefix(b) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto bitsIter = m_state.m_bits.find(nameIter->second);
        if (bitsIter != m_state.m_bits.end()) {
            logError() << XmlWrap::logPrefix(b) << "Bit with name \"" << nameIter->second <<
                          "\" has already been defined for set \"" << name() << "\".";
            return false;
        }

        auto idxIter = props.find(common::idxStr());
        assert(idxIter != props.end());

        bool ok = false;
        unsigned idx = common::strToUnsigned(idxIter->second, &ok);
        if (!ok) {
            XmlWrap::XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::idxStr(), idxIter->second, protocol().logger());
            return false;
        }

        if (m_state.m_bitLength <= idx) {
            logError() << XmlWrap::logPrefix(b) <<
                          "Index of the bit (" << idx << ") must be less than number of available bits (" <<
                          m_state.m_bitLength << ").";
            return false;
        }
        if (!m_state.m_nonUniqueAllowed) {
            auto revBitsIter = m_state.m_revBits.find(idx);
            if (revBitsIter != m_state.m_revBits.end()) {
                logError() << XmlWrap::logPrefix(b) <<
                      "Bit \"" << revBitsIter->first << "\" has been already defined "
                      "as \"" << revBitsIter->second << "\".";
                return false;
            }
        }

        BitInfo info;
        info.m_idx = idx;
        info.m_defaultValue = m_state.m_defaultBitValue;
        info.m_reservedValue = m_state.m_reservedBitValue;
        info.m_sinceVersion = getSinceVersion();
        info.m_deprecatedSince = getDeprecated();
        do {
            auto& bitDefaultValueStr = common::getStringProp(props, common::defaultValueStr());
            if (bitDefaultValueStr.empty()) {
                break;
            }

            ok = false;
            info.m_defaultValue = common::strToBool(bitDefaultValueStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::defaultValueStr(), bitDefaultValueStr, protocol().logger());
                return false;
            }

        } while (false);

        do{
            auto& bitReservedStr = common::getStringProp(props, common::reservedStr());
            if (bitReservedStr.empty()) {
                break;
            }

            ok = false;
            info.m_reserved = common::strToBool(bitReservedStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::reservedStr(), bitReservedStr, protocol().logger());
                return false;
            }

        } while (false);


        do {
            if (!info.m_reserved) {
                info.m_reservedValue = false;
                break;
            }

            auto& bitReservedValueStr = common::getStringProp(props, common::reservedValueStr());
            if (bitReservedValueStr.empty()) {
                break;
            }

            ok = false;
            info.m_reservedValue = common::strToBool(bitReservedValueStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::reservedValueStr(), bitReservedValueStr, protocol().logger());
                return false;
            }
        } while (false);

        if (!XmlWrap::getAndCheckVersions(b, nameIter->second, props, info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
            return false;
        }

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
                    logError() << XmlWrap::logPrefix(b) <<
                          "Inconsistent value of \"" << common::defaultValueStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }

                if (info.m_reserved != iter->second.m_reserved) {
                    logError() << XmlWrap::logPrefix(b) <<
                          "Inconsistent value of \"" << common::reservedStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }

                if (info.m_reservedValue != iter->second.m_reservedValue) {
                    logError() << XmlWrap::logPrefix(b) <<
                          "Inconsistent value of \"" << common::reservedValueStr() << "\" property "
                          "for bit " << idx << ".";
                    return false;
                }
            }
        } while(false);

        m_state.m_bits.emplace(nameIter->second, info);
        m_state.m_revBits.emplace(idx, nameIter->second);
    }

    return true;
}

} // namespace bbmp
