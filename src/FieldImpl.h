#pragma once

#include <memory>

#include "XmlWrap.h"
#include "Logger.h"

namespace bbmp
{

class ProtocolImpl;
class FieldImpl
{
public:
    using Ptr = std::unique_ptr<FieldImpl>;
    using PropsMap = XmlWrap::PropsMap;

    static Ptr create(const std::string& kind, ::xmlNodePtr node, ProtocolImpl& protocol);

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;

    std::size_t length() const
    {
        return lengthImpl();
    }

protected:
    explicit FieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
      : m_node(node),
        m_protocol(protocol)
    {
    }

    ProtocolImpl& protocol()
    {
        return m_protocol;
    }

    const ProtocolImpl& protocol() const
    {
        return m_protocol;
    }

    LogWrapper logError() const;
    LogWrapper logWarning() const;

    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual bool parseImpl();
    virtual std::size_t lengthImpl() const = 0;

private:

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
};

using FieldImplPtr = FieldImpl::Ptr;

} // namespace bbmp
