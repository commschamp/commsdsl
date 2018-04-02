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

namespace
{

} // namespace

SetFieldImpl::SetFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

SetFieldImpl::SetFieldImpl(const SetFieldImpl&) = default;

bool SetFieldImpl::isUnique() const
{
    unsigned prevIdx = 0;
    bool firstElem = true;
    for (auto& b : m_revBits) {
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
        common::reservedValueStr()
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

bool SetFieldImpl::parseImpl()
{
    return
        updateEndian() &&
        updateLength() &&
        updateNonUniqueAllowed() &&
        updateDefaultValue() &&
        updateReservedValue() &&
        updateBits();
}

std::size_t SetFieldImpl::lengthImpl() const
{
    return m_length;
}

std::size_t SetFieldImpl::bitLengthImpl() const
{
    return m_bitLength;
}

bool SetFieldImpl::updateEndian()
{
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(props(), common::endianStr());
    if ((endianStr.empty()) && (m_endian != Endian_NumOfValues)) {
        return true;
    }

    m_endian = common::parseEndian(endianStr, protocol().schemaImpl().endian());
    if (m_endian == Endian_NumOfValues) {
        reportUnexpectedPropertyValue(common::endianStr(), endianStr);
        return false;
    }
    return true;
}

bool SetFieldImpl::updateLength()
{
    bool mustHaveLength = (m_length == 0U) && (!isBitfieldMember());
    if (!validateSinglePropInstance(common::lengthStr(), mustHaveLength)) {
        return false;
    }

    static const std::size_t MaxLength = sizeof(std::uint64_t);
    auto& lengthStr = common::getStringProp(props(), common::lengthStr());
    do {
        if (lengthStr.empty()) {
            break;
        }

        bool ok = false;
        m_length = static_cast<decltype(m_length)>(common::strToUintMax(lengthStr, &ok));

        if ((!ok) || (m_length == 0U) || (MaxLength < m_length)) {
            reportUnexpectedPropertyValue(common::lengthStr(), lengthStr);
            return false;
        }

    } while (false);

    bool mustHaveBitLength = (isBitfieldMember() && m_bitLength == 0U && (m_length == 0U));
    if (!validateSinglePropInstance(common::bitLengthStr(), mustHaveBitLength)) {
        return false;
    }

    auto& bitLengthStr = common::getStringProp(props(), common::bitLengthStr());
    do {
        if (bitLengthStr.empty()) {
            break;
        }

        bool ok = false;
        m_bitLength = static_cast<decltype(m_length)>(common::strToUintMax(lengthStr, &ok));
        if ((!ok) || (m_bitLength == 0U)) {
            reportUnexpectedPropertyValue(common::bitLengthStr(), bitLengthStr);
            return false;
        }
    } while (false);

    if (lengthStr.empty() && bitLengthStr.empty()) {
        assert(m_length != 0U);
        assert(m_bitLength != 0U);
        assert(m_bitLength <= (m_length * 8U));
        assert(m_type < Type::NumOfValues);
        return true;
    }

    do {
        if (!bitLengthStr.empty()) {
            assert(0U < m_bitLength);
            auto minLength = ((m_bitLength - 1U) / 8U) + 1U;
            if (m_length == 0U) {
                assert(lengthStr.empty());
                m_length = minLength;
                break;
            }

            if (m_length < minLength) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                    "Specified bit length (" << m_bitLength << ") doesn't fit into "
                    "field length (" << m_length << ").";
                return false;
            }
            break;
        }

        auto maxBitLength = m_length * std::numeric_limits<std::uint8_t>::digits;
        if (m_bitLength == 0U) {
            assert(0U < m_length);
            m_bitLength = maxBitLength;
            break;
        }

        if (maxBitLength < m_bitLength) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Inherited bit length (" << m_bitLength << ") doesn't fit into "
                "field length (" << m_length << ").";
            return false;
        }
    } while (false);

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
    static_assert(MapSize == MaxLength, "Invalid map");

    assert(m_length < MapSize);
    m_type = Map[m_length - 1];

    if (m_revBits.empty()) {
        assert(m_bits.empty());
        return true;
    }

    auto lastIdx = m_revBits.rbegin()->first;
    assert(lastIdx < 64U);
    if (m_bitLength < lastIdx) {
        logError() << XmlWrap::logPrefix(getNode()) << "The set field cannot contain all the listed bits.";
        return false;
    }
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

    bool wasAllowed = m_nonUniqueAllowed;
    bool ok = false;
    m_nonUniqueAllowed = common::strToBool(valueStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::nonUniqueAllowedStr(), valueStr);
        return false;
    }

    if (wasAllowed && (!m_nonUniqueAllowed)) {
        assert(!"NYI");
        // TODO: check non duplicates
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
    m_defaultBitValue = common::strToBool(valueStr, &ok);
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
    m_reservedBitValue = common::strToBool(valueStr, &ok);
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
        if (!m_bits.empty()) {
            assert(!m_revBits.empty());
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
            common::reservedStr()
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

        auto nameIter = props.find(common::nameStr());
        assert(nameIter != props.end());

        if (!common::isValidName(nameIter->second)) {
            logError() << XmlWrap::logPrefix(b) <<
                  "Property \"" << common::nameStr() <<
                  "\" has unexpected value (" << nameIter->second << ").";
            return false;
        }

        auto bitsIter = m_bits.find(nameIter->second);
        if (bitsIter != m_bits.end()) {
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

        if (m_bitLength <= idx) {
            logError() << XmlWrap::logPrefix(b) <<
                          "Index of the bit (" << idx << ") cannot exceed number of available bits (" <<
                          m_bitLength << ").";
            return false;
        }

        if (!m_nonUniqueAllowed) {
            auto revBitsIter = m_revBits.find(idx);
            if (revBitsIter != m_revBits.end()) {
                logError() << XmlWrap::logPrefix(b) <<
                      "Bit \"" << revBitsIter->first << "\" has been already defined "
                      "as \"" << revBitsIter->second << "\".";
                return false;
            }
        }

        auto bitMask = static_cast<std::uintmax_t>(1U) << idx;
        bool bitValue = m_defaultBitValue;
        do {
            auto& bitDefaultValueStr = common::getStringProp(props, common::defaultValueStr());
            if (bitDefaultValueStr.empty()) {
                break;
            }

            ok = false;
            bitValue = common::strToBool(bitDefaultValueStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::defaultValueStr(), bitDefaultValueStr, protocol().logger());
                return false;
            }

            if (!m_nonUniqueAllowed) {
                // The bit hasn't been processed earlier
                assert(m_revBits.find(idx) == m_revBits.end());
                break;
            }

            auto iter = m_revBits.find(idx);
            if (iter == m_revBits.end()) {
                // The bit hasn't been processed earlier
                break;
            }

            auto prevBitValue = (m_defaultValue & bitMask) != 0;
            if (bitValue != prevBitValue) {
                logError() << XmlWrap::logPrefix(b) <<
                              "Inconsistent value of \"" << common::defaultValueStr() << "\" property "
                              "for bit " << idx << ".";
                return false;
            }
        } while (false);

        if (bitValue) {
            m_defaultValue |= bitMask;
        }
        else {
            m_defaultValue &= (~bitMask);
        }

        bool reserved = false;
        do{
            auto& bitReservedStr = common::getStringProp(props, common::reservedStr());
            if (bitReservedStr.empty()) {
                break;
            }

            ok = false;
            reserved = common::strToBool(bitReservedStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::reservedStr(), bitReservedStr, protocol().logger());
                return false;
            }

            if (!m_nonUniqueAllowed) {
                // The bit hasn't been processed earlier
                assert(m_revBits.find(idx) == m_revBits.end());
                break;
            }

            auto iter = m_revBits.find(idx);
            if (iter == m_revBits.end()) {
                // The bit hasn't been processed earlier
                break;
            }

            std::uint64_t andResult = 0U;
            if (reserved) {
                andResult = bitMask;
            }

            if ((m_reservedBits & bitMask) != andResult) {
                logError() << XmlWrap::logPrefix(b) <<
                              "Inconsistent value of \"" << common::reservedStr() << "\" property "
                              "for bit " << idx << ".";
                return false;
            }

        } while (false);

        if (reserved) {
            m_reservedBits |= bitMask;
        }
        else {
            assert((m_reservedBits & bitMask) == 0U);
        }

        bool reservedValue = m_reservedBitValue;
        do {
            auto& bitReservedValueStr = common::getStringProp(props, common::reservedValueStr());
            if (bitReservedValueStr.empty()) {
                break;
            }

            ok = false;
            reservedValue = common::strToBool(bitReservedValueStr, &ok);
            if (!ok) {
                XmlWrap::reportUnexpectedPropertyValue(b, nameIter->second, common::reservedValueStr(), bitReservedValueStr, protocol().logger());
                return false;
            }
        } while (false);

        if (reservedValue && reserved) {
            m_reservedValue |= bitMask;
        }
        else {
            m_reservedValue &= (~bitMask);
        }

        m_bits.emplace(nameIter->second, idx);
        m_revBits.emplace(idx, nameIter->second);
    }

    auto allBitsMask = ~(static_cast<decltype(m_implicitReserved)>(0U));
    if (m_bitLength < 64) {
        allBitsMask = (static_cast<std::uint64_t>(1U) << m_bitLength) - 1;
    }

    m_implicitReserved = allBitsMask;
    assert((m_defaultValue & allBitsMask) == m_defaultValue);
    assert((m_reservedValue & allBitsMask) == m_reservedValue);
//    m_defaultValue &= allBitsMask;
//    m_reservedValue &= allBitsMask;

    for (auto& b : m_revBits) {
        assert(b.first < m_bitLength);
        auto mask = static_cast<decltype(m_implicitReserved)>(1U) << b.first;
        m_implicitReserved &= ~mask;
    }

    if (m_defaultBitValue) {
        m_defaultValue |= m_implicitReserved;
    }
    else {
        m_defaultValue &= (~m_implicitReserved);
    }

    if (m_reservedBitValue) {
        m_reservedValue |= m_implicitReserved;
    }
    else {
        m_reservedValue &= ~m_implicitReserved;
    }
    return true;
}

} // namespace bbmp
