#pragma once

#include <memory>

#include "bbmp/Namespace.h"

#include "XmlWrap.h"
#include "Logger.h"
#include "FieldImpl.h"
#include "Object.h"

namespace bbmp
{

class ProtocolImpl;
class NamespaceImpl : public Object
{
public:
    using Ptr = std::unique_ptr<NamespaceImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using NamespacesList = Namespace::NamespacesList;
    using FieldsList = Namespace::FieldsList;
    using NamespacesMap = std::map<std::string, Ptr>;
    using FieldsMap = std::map<std::string, FieldImplPtr>;

    NamespaceImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    virtual ~NamespaceImpl() = default;

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parseProps();

    bool parseChildren(NamespaceImpl* realNs = nullptr);

    bool parse();

    bool processChild(::xmlNodePtr node, NamespaceImpl* realNs = nullptr);

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

    const NamespacesMap& namespacesMap() const
    {
        return m_namespaces;
    }

    const FieldImpl* findField(const std::string& fieldName) const;

protected:
    virtual ObjKind objKindImpl() const override;

private:

    bool processNamespace(::xmlNodePtr node);
    bool processMultipleFields(::xmlNodePtr node);
    bool processMessage(::xmlNodePtr node);
    bool processMultipleMessages(::xmlNodePtr node);
    bool processFrame(::xmlNodePtr node);
    bool processMultipleFrames(::xmlNodePtr node);

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;

    PropsMap m_props;
    PropsMap m_unknownAttrs;
    ContentsList m_unknownChildren;

    std::string m_name;
    std::string m_description;

    NamespacesMap m_namespaces;
    NamespacesList m_namespacesList;
    FieldsMap m_fields;
    FieldsList m_fieldsList;
};

using NamespaceImplPtr = NamespaceImpl::Ptr;

} // namespace bbmp
