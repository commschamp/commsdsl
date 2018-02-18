#include "FieldImpl.h"

#include <functional>
#include <map>
#include <string>

#include "IntFieldImpl.h"
#include "common.h"

namespace bbmp
{

FieldImpl::Ptr FieldImpl::create(
    const std::string& kind,
    ::xmlNodePtr node,
    Logger& logger)
{
    using CreateFunc = std::function<Ptr (::xmlNodePtr n, Logger& l)>;
    static const std::map<std::string, CreateFunc> CreateMap = {
        std::make_pair(
            common::intStr(),
            [](::xmlNodePtr n, Logger& l)
            {
                return Ptr(new IntFieldImpl(n, l));
            })
    };

    auto iter = CreateMap.find(kind);
    if (iter == CreateMap.end()) {
        return Ptr();
    }

    return iter->second(node, logger);
}

bool FieldImpl::parse()
{
    m_props = XmlWrap::parseNodeProps(m_node);

    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::displayNameStr(),
        common::descriptionStr()
    };

    if (!XmlWrap::parseChildrenAsProps(m_node, CommonNames, m_logger, m_props)) {
        return false;
    }

    auto& extraPropsNames = extraPropsNamesImpl();
    do {
        if (extraPropsNames.empty()) {
            break;
        }

        if (!XmlWrap::parseChildrenAsProps(m_node, extraPropsNames, m_logger, m_props)) {
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
