#include "BitfieldFieldImpl.h"

#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <algorithm>

namespace bbmp
{

namespace
{

const XmlWrap::NamesList& bitfieldSupportedTypes()
{
    static const XmlWrap::NamesList Names = {
        common::intStr()
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

Object::ObjKind BitfieldFieldImpl::objKindImpl() const
{
    return ObjKind::Bitfield;
}

FieldImpl::Kind BitfieldFieldImpl::kindImpl() const
{
    return Kind::Bitfield;
}

FieldImpl::Ptr BitfieldFieldImpl::cloneImpl() const
{
    return Ptr(new BitfieldFieldImpl(*this));
}

const XmlWrap::NamesList& BitfieldFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList Names = getExtraNames();
    return Names;
}

bool BitfieldFieldImpl::parseImpl()
{
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
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The \"" << common::bitfieldStr() << "\" must contain member fields.";
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

    static const std::size_t BitsInByte = std::numeric_limits<std::uint8_t>::digits;
    static const std::size_t SupportedBitLengths[] = {
        sizeof(std::uint8_t) * BitsInByte,
        sizeof(std::uint16_t) * BitsInByte,
        sizeof(std::uint32_t) * BitsInByte,
        sizeof(std::uint64_t) * BitsInByte
    };

    auto iter = std::find(std::begin(SupportedBitLengths), std::end(SupportedBitLengths), totalBitLength);
    if (iter == std::end(SupportedBitLengths)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The summary of member's bit lengths (" << totalBitLength <<
                      ") is expected to be one of the following: 8, 16, 32, 64.";
        return false;
    }

    assert(!m_members.empty());
    auto& firstMem = m_members.front();
    if (getMinSinceVersion() < firstMem->getMinSinceVersion()) {
        logError() << XmlWrap::logPrefix(firstMem->getNode()) <<
                      "First member mustn't have value of \"" << common::sinceVersionStr() <<
                      "\" property (" << firstMem->getMinSinceVersion() << ") to be greater "
                      "than value of the containing \"" << common::bitfieldStr() << "\" (" <<
                      getMinSinceVersion() << ").";
        return false;
    }

    assert(firstMem->getMinSinceVersion() == getMinSinceVersion());

    if (!validateMembersVersions(m_members)) {
        return false;
    }

    return true;
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



} // namespace bbmp
