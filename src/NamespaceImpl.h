#pragma once

#include <memory>

#include "bbmp/Namespace.h"

#include "XmlWrap.h"
#include "Logger.h"
#include "FieldImpl.h"

namespace bbmp
{

class ProtocolImpl;
class NamespaceImpl
{
public:
    using Ptr = std::unique_ptr<NamespaceImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using FieldsList = Namespace::FieldsList;

    NamespaceImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    bool parse();

    bool processChild(::xmlNodePtr node);

    bool merge(NamespaceImpl& other);

    bool finalise();

    static const XmlWrap::NamesList& supportedChildren();

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const
    {
        return m_name;
    }

    const std::string& description() const
    {
        return m_description;
    }

    const FieldsList& fieldsList() const
    {
        return m_fieldsList;
    }

    const PropsMap& unknownAttributes() const
    {
        return m_unknownAttrs;
    }

    PropsMap& unknownAttributes()
    {
        return m_unknownAttrs;
    }

    const ContentsList& unknownChildren() const
    {
        return m_unknownChildren;
    }

    ContentsList& unknownChildren()
    {
        return m_unknownChildren;
    }

private:
    using FieldsMap = std::map<std::string, FieldImplPtr>;

    bool processMultipleFields(::xmlNodePtr node);
    bool processMessage(::xmlNodePtr node);
    bool processMultipleMessages(::xmlNodePtr node);
    bool processFrame(::xmlNodePtr node);
    bool processMultipleFrames(::xmlNodePtr node);

    LogWrapper logError() const;
    LogWrapper logWarning() const;

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;

    PropsMap m_props;
    PropsMap m_unknownAttrs;
    ContentsList m_unknownChildren;

    std::string m_name;
    std::string m_description;

    FieldsMap m_fields;
    FieldsList m_fieldsList;
};

using NamespaceImplPtr = NamespaceImpl::Ptr;

} // namespace bbmp
