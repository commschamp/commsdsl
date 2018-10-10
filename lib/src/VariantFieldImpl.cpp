#include "VariantFieldImpl.h"

#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <algorithm>
#include <iterator>

#include "ProtocolImpl.h"
#include "OptionalFieldImpl.h"

namespace commsdsl
{

namespace
{

const XmlWrap::NamesList& variantSupportedTypes()
{
    static const XmlWrap::NamesList Names = FieldImpl::supportedTypes();
    return Names;
}

XmlWrap::NamesList getExtraNames()
{
    auto names = variantSupportedTypes();
    names.push_back(common::membersStr());
    return names;
}

} // namespace

VariantFieldImpl::VariantFieldImpl(xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

VariantFieldImpl::VariantFieldImpl(const VariantFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    m_members.reserve(other.m_members.size());
    for (auto& m : other.m_members) {
        m_members.push_back(m->clone());
    }
}

VariantFieldImpl::Members VariantFieldImpl::membersList() const
{
    Members result;
    result.reserve(m_members.size());
    std::transform(
        m_members.begin(), m_members.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return Field(elem.get());
        });
    return result;
}

FieldImpl::Kind VariantFieldImpl::kindImpl() const
{
    return Kind::Variant;
}

FieldImpl::Ptr VariantFieldImpl::cloneImpl() const
{
    return Ptr(new VariantFieldImpl(*this));
}

const XmlWrap::NamesList& VariantFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::defaultMemberStr(),
        common::displayIdxReadOnlyHiddenStr()
    };

    return List;
}

const XmlWrap::NamesList& VariantFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList Names = getExtraNames();
    return Names;
}

bool VariantFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const VariantFieldImpl&>(other);
    m_state = castedOther.m_state;
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

bool VariantFieldImpl::parseImpl()
{
    return
        updateMembers() &&
        updateDefaultMember() &&
        updateIdxHidden();
}

std::size_t VariantFieldImpl::minLengthImpl() const
{
    return 0U;
}

std::size_t VariantFieldImpl::maxLengthImpl() const
{
    std::size_t sum = 0U;
    for (auto& m : m_members) {
        auto val = m->maxLength();
        if (val == std::numeric_limits<std::size_t>::max()) {
            return val;
        }

        sum = std::max(val, sum);
    }

    return sum;
}

bool VariantFieldImpl::updateMembers()
{
    if (!m_members.empty()) {
        m_members.erase(
            std::remove_if(
                m_members.begin(), m_members.end(),
                [this](auto& elem)
                {
                    return
                        (elem->isDeprecatedRemoved()) &&
                        (elem->getDeprecated() <= this->getSinceVersion());
                }),
            m_members.end());

        for (auto& m : m_members) {
            m->setSinceVersion(std::max(getSinceVersion(), m->getSinceVersion()));
        }
    }

    do {
        auto membersNodes = XmlWrap::getChildren(getNode(), common::membersStr());
        if (1U < membersNodes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "Only single \"" << common::membersStr() << "\" child element is "
                          "supported for \"" << common::variantStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = XmlWrap::getChildren(getNode(), variantSupportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::variantStr() << "\" element does not support "
                          "list of stand alone member fields as child elements together with \"" <<
                          common::membersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "The \"" << common::variantStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = XmlWrap::getChildren(getNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "The member types of \"" << common::variantStr() <<
                              "\" must be defined inside \"<" << common::membersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = XmlWrap::getChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = XmlWrap::getChildren(membersNodes.front(), variantSupportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(membersNodes.front()) <<
                              "The \"" << common::membersStr() << "\" child node of \"" <<
                              common::variantStr() << "\" element must contain only supported types.";
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

            // if (!mem->verifySiblings(m_members)) {
            //     return false;
            // }

            m_members.push_back(std::move(mem));
        }

        if (!validateMembersNames(m_members)) {
            return false;
        }

    } while (false);

    bool hasSameVer =
        std::any_of(
            m_members.begin(), m_members.end(),
            [this](auto& m)
            {
                return m->getSinceVersion() == this->getSinceVersion();
            });

    if (!hasSameVer) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "There must be at least one member with the same version as the parent variant.";
        return false;
    }

    return true;
}

bool VariantFieldImpl::updateDefaultMember()
{
    auto& propName = common::defaultMemberStr();
    if (!validateSinglePropInstance(propName)) {
        return false;
    }

    auto& valueStr = common::getStringProp(props(), propName);
    if (valueStr.empty()) {
        return true;
    }

    if (common::isValidName(valueStr)) {
        auto iter = 
            std::find_if(
                m_members.begin(), m_members.end(),
                [&valueStr](auto& m)
                {
                    return valueStr == m->name(); 
                });

        if (iter == m_members.end()) {
            reportUnexpectedPropertyValue(propName, valueStr);
            return false;
        }

        m_state.m_defaultIdx = 
            static_cast<decltype(m_state.m_defaultIdx)>(
                std::distance(m_members.begin(), iter));

        return true;
    }

    std::intmax_t val = 0U;
    do {
        if (common::isValidRefName(valueStr)) {
            if (protocol().strToEnumValue(valueStr, val, false)) {
                break;
            }

            reportUnexpectedPropertyValue(propName, valueStr);
            return false;
        }
        
        bool ok = false;
        val = common::strToIntMax(valueStr, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(propName, valueStr);
            return false;
        }
    } while (false);

    if (val < 0) {
        m_state.m_defaultIdx = std::numeric_limits<std::size_t>::max();
        return true;
    }

    if (m_members.size() <= static_cast<std::size_t>(val)) {
        reportUnexpectedPropertyValue(propName, valueStr);
        return false;
    }

    m_state.m_defaultIdx = static_cast<std::size_t>(val);
    return true;
}
    
bool VariantFieldImpl::updateIdxHidden()
{
    return validateAndUpdateBoolPropValue(common::displayIdxReadOnlyHiddenStr(), m_state.m_idxHidden);
}

} // namespace commsdsl
