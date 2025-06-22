//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseEndian.h"

#include "ParseXmlWrap.h"
#include "ParseNamespaceImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseSchemaImpl final : public ParseObject
{
    using Base = ParseObject;
public:
    using PropsMap = ParseXmlWrap::PropsMap;
    using ContentsList = ParseXmlWrap::ContentsList;
    using NamespacesList = ParseNamespaceImpl::NamespacesList;
    using NamespacesMap = ParseNamespaceImpl::NamespacesMap;
    using PlatformsList = ParseSchema::PlatformsList;
    using MessagesList = ParseSchema::MessagesList;
    using InterfacesList = ParseNamespaceImpl::InterfacesList;
    using ImplInterfacesList = ParseNamespaceImpl::ImplInterfacesList;
    using FieldRefInfosList = ParseNamespaceImpl::FieldRefInfosList;

    ParseSchemaImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);

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

    ParseEndian endian() const
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

    const ParseFieldImpl* findField(const std::string& ref, bool checkRef = true) const;

    const ParseMessageImpl* findMessage(const std::string& ref, bool checkRef = true) const;

    const ParseInterfaceImpl* findInterface(const std::string& ref, bool checkRef = true) const;    

    const PlatformsList& platforms() const
    {
        return m_platforms;
    }

    MessagesList allMessages() const;
    InterfacesList allInterfaces() const;
    ImplInterfacesList allImplInterfaces() const;

    bool addPlatform(const std::string& name);
    void addNamespace(ParseNamespaceImplPtr ns);
    ParseNamespaceImpl& defaultNamespace();

    bool validateAllMessages();
    unsigned countMessageIds() const;

    std::string externalRef() const;

    FieldRefInfosList processInterfaceFieldRef(const std::string& refStr) const;

protected:
    virtual ObjKind objKindImpl() const override;    

private:

    bool updateStringProperty(const PropsMap& map, const std::string& name, std::string& prop);
    bool updateUnsignedProperty(const PropsMap& map, const std::string& name, unsigned& prop);
    bool updateEndianProperty(const PropsMap& map, const std::string& name, ParseEndian& prop);
    bool updateBooleanProperty(const PropsMap& map, const std::string& name, bool& prop);
    bool updateExtraAttrs();
    bool updateExtraChildren();
    const ParseNamespaceImpl* getNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const;

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;

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
    ParseEndian m_endian = ParseEndian_NumOfValues;
    bool m_nonUniqueMsgIdAllowed = false;
};

using ParseSchemaImplPtr = std::unique_ptr<ParseSchemaImpl>;

} // namespace parse

} // namespace commsdsl
