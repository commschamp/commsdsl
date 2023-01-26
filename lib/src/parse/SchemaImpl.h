//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "commsdsl/parse/Endian.h"

#include "XmlWrap.h"
#include "NamespaceImpl.h"

namespace commsdsl
{

namespace parse
{

class ProtocolImpl;
class SchemaImpl final : public Object
{
    using Base = Object;
public:
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using NamespacesList = NamespaceImpl::NamespacesList;
    using NamespacesMap = NamespaceImpl::NamespacesMap;
    using PlatformsList = Schema::PlatformsList;
    using MessagesList = Schema::MessagesList;

    SchemaImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    bool processNode();

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

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

    unsigned id() const
    {
        return m_id;
    }

    unsigned version() const
    {
        return m_version;
    }

    unsigned dslVersion() const
    {
        return m_dslVersion;
    }

    Endian endian() const
    {
        return m_endian;
    }

    bool nonUniqueMsgIdAllowed() const
    {
        return m_nonUniqueMsgIdAllowed;
    }

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    PropsMap& extraAttributes()
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildrenElements() const
    {
        return m_extraChildren;
    }

    ContentsList& extraChildrenElements()
    {
        return m_extraChildren;
    }

    const NamespacesMap& namespaces() const
    {
        return m_namespaces;
    }

    NamespacesList namespacesList() const;    

    const FieldImpl* findField(const std::string& ref, bool checkRef = true) const;

    const MessageImpl* findMessage(const std::string& ref, bool checkRef = true) const;

    const InterfaceImpl* findInterface(const std::string& ref, bool checkRef = true) const;    

    const PlatformsList& platforms() const
    {
        return m_platforms;
    }

    MessagesList allMessages() const;

    bool addPlatform(const std::string& name);
    void addNamespace(NamespaceImplPtr ns);
    NamespaceImpl& defaultNamespace();

    bool validateAllMessages();
    unsigned countMessageIds() const;

    std::string externalRef() const;

protected:
    virtual ObjKind objKindImpl() const override;    

private:

    bool updateStringProperty(const PropsMap& map, const std::string& name, std::string& prop);
    bool updateUnsignedProperty(const PropsMap& map, const std::string& name, unsigned& prop);
    bool updateEndianProperty(const PropsMap& map, const std::string& name, Endian& prop);
    bool updateBooleanProperty(const PropsMap& map, const std::string& name, bool& prop);
    bool updateExtraAttrs();
    bool updateExtraChildren();
    const NamespaceImpl* getNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const;

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;

    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;
    std::string m_name;
    std::string m_description;
    NamespacesMap m_namespaces;
    PlatformsList m_platforms;
    unsigned m_id = 0U;
    unsigned m_version = 0;
    unsigned m_dslVersion = 0;
    Endian m_endian = Endian_NumOfValues;
    bool m_nonUniqueMsgIdAllowed = false;
};

using SchemaImplPtr = std::unique_ptr<SchemaImpl>;

} // namespace parse

} // namespace commsdsl
