#include "FieldImpl.h"

#include <functional>
#include <map>
#include <string>
#include <cassert>

#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "common.h"

namespace bbmp
{

FieldImpl::Ptr FieldImpl::create(
    const std::string& kind,
    ::xmlNodePtr node,
    ProtocolImpl& protocol)
{
    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ProtocolImpl& p)>;
    static const std::map<std::string, CreateFunc> CreateMap = {
        std::make_pair(
            common::intStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new IntFieldImpl(n, p));
            })
    };

    auto iter = CreateMap.find(kind);
    if (iter == CreateMap.end()) {
        return Ptr();
    }

    return iter->second(node, protocol);
}

bool FieldImpl::parse()
{
    m_props = XmlWrap::parseNodeProps(m_node);

    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::displayNameStr(),
        common::descriptionStr()
    };

    if (!XmlWrap::parseChildrenAsProps(m_node, CommonNames, m_protocol.logger(), m_props)) {
        return false;
    }

    auto& extraPropsNames = extraPropsNamesImpl();
    do {
        if (extraPropsNames.empty()) {
            break;
        }

        if (!XmlWrap::parseChildrenAsProps(m_node, extraPropsNames, m_protocol.logger(), m_props)) {
            return false;
        }

    } while (false);

    bool result =
        updateName() &&
        updateDisplayName() &&
        updateDescription();

    if (!result) {
        return false;
    }

    return parseImpl();
}

const std::string& FieldImpl::name() const
{
    assert(m_name != nullptr);
    return *m_name;
}

const std::string& FieldImpl::displayName() const
{
    assert(m_displayName != nullptr);
    return *m_displayName;
}

const std::string& FieldImpl::description() const
{
    assert(m_description != nullptr);
    return *m_description;
}

FieldImpl::FieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol),
    m_name(&common::emptyString()),
    m_displayName(&common::emptyString()),
    m_description(&common::emptyString())
{
}

LogWrapper FieldImpl::logError() const
{
    return bbmp::logError(m_protocol.logger());
}

LogWrapper FieldImpl::logWarning() const
{
    return bbmp::logWarning(m_protocol.logger());
}

const XmlWrap::NamesList& FieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList Names;
    return Names;
}

bool FieldImpl::parseImpl()
{
    return true;
}

bool FieldImpl::validateSinglePropInstance(const std::string& str, bool mustHave)
{
    auto count = m_props.count(str);
    if (1U < count) {
        logError() << XmlWrap::logPrefix(m_node) <<
                      "Too many values of \"" << str << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    if ((count == 0U) && mustHave) {
        logError() << XmlWrap::logPrefix(m_node) <<
                      "Missing value for mandatory property \"" << str << "\" for \"" << m_node->name << "\" element.";
        return false;
    }

    return true;
}

bool FieldImpl::validateAndUpdateStringPropValue(
    const std::string& str,
    const std::string*& valuePtr,
    bool mustHave)
{
    if (!validateSinglePropInstance(str, mustHave)) {
        return false;
    }

    auto iter = m_props.find(str);
    if (iter != m_props.end()) {
        valuePtr = &iter->second;
    }

    assert(iter != m_props.end() || (!mustHave));
    return true;
}

bool FieldImpl::updateName()
{
    return validateAndUpdateStringPropValue(common::nameStr(), m_name, true);
}

bool FieldImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_description);
}

bool FieldImpl::updateDisplayName()
{
    return validateAndUpdateStringPropValue(common::displayNameStr(), m_displayName);
}

} // namespace bbmp
