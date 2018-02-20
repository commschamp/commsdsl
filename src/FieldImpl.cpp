#include "FieldImpl.h"

#include <functional>
#include <map>
#include <string>

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

    return parseImpl();
}

const std::string& FieldImpl::name() const
{
    return common::getStringProp(m_props, common::nameStr());
}

const std::string& FieldImpl::displayName() const
{
    return common::getStringProp(m_props, common::displayNameStr());
}

const std::string& FieldImpl::description() const
{
    return common::getStringProp(m_props, common::descriptionStr());
}

LogWrapper FieldImpl::logError() const
{
    return bbmp::logError(m_protocol.logger());
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

} // namespace bbmp
