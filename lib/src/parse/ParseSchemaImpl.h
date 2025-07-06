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

#include "ParseNamespaceImpl.h"
#include "ParseXmlWrap.h"

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

    bool parseProcessNode();

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    const PropsMap& parseProps() const
    {
        return m_props;
    }

    const std::string& parseName() const
    {
        return m_name;
    }

    const std::string& parseDescription() const
    {
        return m_description;
    }

    unsigned parseId() const
    {
        return m_id;
    }

    unsigned parseVersion() const
    {
        return m_version;
    }

    unsigned parseDslVersion() const
    {
        return m_dslVersion;
    }

    ParseEndian parseEndian() const
    {
        return m_endian;
    }

    bool parseNonUniqueMsgIdAllowed() const
    {
        return m_nonUniqueMsgIdAllowed;
    }

    const PropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    PropsMap& parseExtraAttributes()
    {
        return m_extraAttrs;
    }

    const ContentsList& parseExtraChildrenElements() const
    {
        return m_extraChildren;
    }

    ContentsList& parseExtraChildrenElements()
    {
        return m_extraChildren;
    }

    const NamespacesMap& parseNamespaces() const
    {
        return m_namespaces;
    }

    NamespacesList parseNamespacesList() const;    

    const ParseFieldImpl* parseFindField(const std::string& ref, bool checkRef = true) const;

    const ParseMessageImpl* parseFindMessage(const std::string& ref, bool checkRef = true) const;

    const ParseInterfaceImpl* parseFindInterface(const std::string& ref, bool checkRef = true) const;    

    const PlatformsList& parsePlatforms() const
    {
        return m_platforms;
    }

    MessagesList parseAllMessages() const;
    InterfacesList parseAllInterfaces() const;
    ImplInterfacesList parseAllImplInterfaces() const;

    bool parseAddPlatform(const std::string& name);
    void parseAddNamespace(ParseNamespaceImplPtr ns);
    ParseNamespaceImpl& parseDefaultNamespace();

    bool parseValidateAllMessages();
    unsigned parseCountMessageIds() const;

    std::string parseExternalRef() const;

    FieldRefInfosList parseProcessInterfaceFieldRef(const std::string& refStr) const;

protected:
    virtual ObjKind parseObjKindImpl() const override;    

private:

    bool parseUpdateStringProperty(const PropsMap& map, const std::string& name, std::string& prop);
    bool parseUpdateUnsignedProperty(const PropsMap& map, const std::string& name, unsigned& prop);
    bool parseUpdateEndianProperty(const PropsMap& map, const std::string& name, ParseEndian& prop);
    bool parseUpdateBooleanProperty(const PropsMap& map, const std::string& name, bool& prop);
    bool parseUpdateExtraAttrs();
    bool parseUpdateExtraChildren();
    const ParseNamespaceImpl* parseGetNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const;

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
