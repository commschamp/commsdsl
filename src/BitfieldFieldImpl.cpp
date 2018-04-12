#include "BitfieldFieldImpl.h"

#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <algorithm>

#include "ProtocolImpl.h"

namespace bbmp
{

namespace
{

const XmlWrap::NamesList& bitfieldSupportedTypes()
{
    static const XmlWrap::NamesList Names = {
        common::intStr(),
        common::enumStr(),
        common::setStr()
    };

    return Names;
}

XmlWrap::NamesList getExtraNames()
{
    auto names = bitfieldSupportedTypes();
    names.push_back(common::membersStr());
    return names;
}

} // namespace

BitfieldFieldImpl::BitfieldFieldImpl(xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

BitfieldFieldImpl::BitfieldFieldImpl(const BitfieldFieldImpl& other)
  : Base(other)
{
    m_members.reserve(other.m_members.size());
    for (auto& m : other.m_members) {
        m_members.push_back(m->clone());
    }
}

FieldImpl::Kind BitfieldFieldImpl::kindImpl() const
{
    return Kind::Bitfield;
}

FieldImpl::Ptr BitfieldFieldImpl::cloneImpl() const
{
    return Ptr(new BitfieldFieldImpl(*this));
}

const XmlWrap::NamesList&BitfieldFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::endianStr(),
    };

    return List;
}

const XmlWrap::NamesList& BitfieldFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList Names = getExtraNames();
    return Names;
}

bool BitfieldFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const BitfieldFieldImpl&>(other);
    m_endian = castedOther.m_endian;
    assert(m_members.empty());
    m_members.reserve(castedOther.m_members.size());
    std::transform(
        castedOther.m_members.begin(), castedOther.m_members.end(), std::back_inserter(m_members),
        [](auto& elem)
        {
            return elem->clone();
        });
    assert(m_members.size() == castedOther.m_members.size());
    return true;
}

bool BitfieldFieldImpl::parseImpl()
{
    return
        updateEndian() &&
        updateMembers();
}

std::size_t BitfieldFieldImpl::lengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), 0U,
            [](std::size_t soFar, auto& m)
            {
                return soFar + m->bitLength();
    }) / 8U;
}

bool BitfieldFieldImpl::updateEndian()
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

bool BitfieldFieldImpl::updateMembers()
{
    if (!m_members.empty()) {
        for (auto& m : m_members) {
            m->setSinceVersion(std::max(getSinceVersion(), m->getSinceVersion()));
            assert(m->getSinceVersion() == getSinceVersion());
            assert(m->getDeprecated() == Protocol::notYetDeprecated());
        }
    }

    do {
        auto membersNodes = XmlWrap::getChildren(getNode(), common::membersStr());
        if (1U < membersNodes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "Only single \"" << common::membersStr() << "\" child element is "
                          "supported for \"" << common::bitfieldStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = XmlWrap::getChildren(getNode(), bitfieldSupportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::bitfieldStr() << "\" element does not support "
                          "list of stand member fields as child elements together with \"" <<
                          common::membersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "The \"" << common::bitfieldStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if (!m_members.empty()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::bitfieldStr() << "\" cannot add member fields after reuse.";
            return false;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = XmlWrap::getChildren(getNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "The member types of \"" << common::bitfieldStr() <<
                              "\" must be defined inside \"<" << common::membersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = XmlWrap::getChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = XmlWrap::getChildren(membersNodes.front(), bitfieldSupportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(membersNodes.front()) <<
                              "The \"" << common::membersStr() << "\" child node of \"" <<
                              common::bitfieldStr() << "\" element must contain only supported types.";
                return false;
            }

            // memberFieldsTypes is updated with the list from <members>
        }

        m_members.reserve(m_members.size() + memberFieldsTypes.size());
        for (auto* memNode : memberFieldsTypes) {
            std::string memKind(reinterpret_cast<const char*>(memNode->name));
            auto mem = FieldImpl::create(memKind, memNode, protocol());
            if (!mem) {
                assert(!"Internal error");
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "Internal error, failed to create objects for member fields.";
                return false;
            }

            mem->setParent(this);
            if (!mem->parse()) {
                return false;
            }

            if ((mem->getSinceVersion() != getSinceVersion()) ||
                (mem->getDeprecated() != getDeprecated())) {
                logError() << XmlWrap::logPrefix(mem->getNode()) <<
                    "Bitfield members are not allowed to update \"" << common::sinceVersionStr() << "\" and "
                    "\"" << common::deprecatedStr() << "\" properties.";
                return false;
            }

            m_members.push_back(std::move(mem));
        }

        if (!validateMembersNames(m_members)) {
            return false;
        }

        auto totalBitLength =
            std::accumulate(
                m_members.begin(), m_members.end(), 0U,
                [this](std::size_t soFar, auto& elem)
                {
                    return soFar + elem->bitLength();
                });

        if ((totalBitLength % 8U) != 0) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The summary of member's bit lengths (" << totalBitLength <<
                          ") is expected to be devisable by 8.";
            return false;
        }

        static const std::size_t MaxBits = std::numeric_limits<std::uint64_t>::digits;
        if (MaxBits < totalBitLength) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The summary of member's bit lengths (" << totalBitLength <<
                          ") cannot be greater than " << MaxBits << '.';
            return false;
        }
    } while (false);

    m_membersList.clear();
    m_membersList.reserve(m_members.size());
    std::transform(
        m_members.begin(), m_members.end(), std::back_inserter(m_membersList),
        [](auto& elem)
        {
            return Field(elem.get());
        });

    return true;
}



} // namespace bbmp
