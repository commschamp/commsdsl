#pragma once

#include <memory>
#include <algorithm>
#include <cctype>

#include "commsdsl/Namespace.h"

#include "XmlWrap.h"
#include "Logger.h"
#include "FieldImpl.h"
#include "MessageImpl.h"
#include "InterfaceImpl.h"
#include "FrameImpl.h"
#include "Object.h"

namespace commsdsl
{

class ProtocolImpl;
class NamespaceImpl : public Object
{
public:

    struct KeyComp
    {
        bool operator()(const std::string& str1, const std::string& str2) const
        {
            if (str1.empty()) {
                return !str2.empty();
            }

            if (str2.empty()) {
                return false;
            }

            auto leftFirst = static_cast<char>(std::tolower(static_cast<int>(str1[0])));
            auto rightFirst = static_cast<char>(std::tolower(static_cast<int>(str2[0])));
            if (leftFirst != rightFirst) {
                return leftFirst < rightFirst;
            }

            return std::lexicographical_compare(str1.begin() + 1, str1.end(), str2.begin() + 1, str2.end());
        }
    };

    using Ptr = std::unique_ptr<NamespaceImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using NamespacesList = Namespace::NamespacesList;
    using FieldsList = Namespace::FieldsList;
    using MessagesList = Namespace::MessagesList;
    using InterfacesList = Namespace::InterfacesList;
    using FramesList = Namespace::FramesList;
    using NamespacesMap = std::map<std::string, Ptr>;
    using FieldsMap = std::map<std::string, FieldImplPtr, KeyComp>;
    using MessagesMap = std::map<std::string, MessageImplPtr, KeyComp>;
    using InterfacesMap = std::map<std::string, InterfaceImplPtr, KeyComp>;
    using FramesMap = std::map<std::string, FrameImplPtr, KeyComp>;

    NamespaceImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    virtual ~NamespaceImpl() = default;

    static const XmlWrap::NamesList& expectedChildrenNames();

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parseProps();

    bool parseChildren(NamespaceImpl* realNs = nullptr);

    bool parse();

    bool processChild(::xmlNodePtr node, NamespaceImpl* realNs = nullptr);

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

    void updateDescription(const std::string& value)
    {
        m_description = value;
    }

    NamespacesList namespacesList() const;
    FieldsList fieldsList() const;
    MessagesList messagesList() const;
    InterfacesList interfacesList() const;
    FramesList framesList() const;

    const MessagesMap& messages() const
    {
        return m_messages;
    }

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    PropsMap& extraAttributes()
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_extraChildren;
    }

    ContentsList& extraChildren()
    {
        return m_extraChildren;
    }

    const NamespacesMap& namespacesMap() const
    {
        return m_namespaces;
    }

    const FieldImpl* findField(const std::string& fieldName) const;
    const MessageImpl* findMessage(const std::string& msgName) const;
    const InterfaceImpl* findInterface(const std::string& intName) const;
    const FrameImpl* findFrame(const std::string& intName) const;

    std::string externalRef() const;

protected:
    virtual ObjKind objKindImpl() const override;

private:

    bool processNamespace(::xmlNodePtr node);
    bool processMultipleFields(::xmlNodePtr node);
    bool processMessage(::xmlNodePtr node);
    bool processMultipleMessages(::xmlNodePtr node);
    bool processInterface(::xmlNodePtr node);
    bool processMultipleInterfaces(::xmlNodePtr node);
    bool processFrame(::xmlNodePtr node);
    bool processMultipleFrames(::xmlNodePtr node);
    bool updateExtraAttrs();
    bool updateExtraChildren();

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;

    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;

    std::string m_name;
    std::string m_description;

    NamespacesMap m_namespaces;
    FieldsMap m_fields;
    MessagesMap m_messages;
    InterfacesMap m_interfaces;
    FramesMap m_frames;
};

using NamespaceImplPtr = NamespaceImpl::Ptr;

} // namespace commsdsl
