#pragma once

#include <memory>

#include "XmlWrap.h"
#include "Logger.h"

namespace bbmp
{

class FieldImpl
{
public:
    using Ptr = std::unique_ptr<FieldImpl>;

    static Ptr create(const std::string& kind, ::xmlNodePtr node, Logger& logger);

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();

    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;

protected:
    explicit FieldImpl(::xmlNodePtr node, Logger& logger)
      : m_node(node),
        m_logger(logger)
    {
    }

    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual bool parseImpl();

private:
    using PropsMap = XmlWrap::PropsMap;

    ::xmlNodePtr m_node = nullptr;
    Logger& m_logger;
    PropsMap m_props;
};

using FieldImplPtr = FieldImpl::Ptr;

} // namespace bbmp
