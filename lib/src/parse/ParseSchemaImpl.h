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
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;
    using ParseNamespacesList = ParseNamespaceImpl::ParseNamespacesList;
    using ParseNamespacesMap = ParseNamespaceImpl::ParseNamespacesMap;
    using ParsePlatformsList = ParseSchema::ParsePlatformsList;
    using ParseMessagesList = ParseSchema::ParseMessagesList;
    using ParseInterfacesList = ParseNamespaceImpl::ParseInterfacesList;
    using ParseImplInterfacesList = ParseNamespaceImpl::ParseImplInterfacesList;
    using ParseFieldRefInfosList = ParseNamespaceImpl::ParseFieldRefInfosList;

    ParseSchemaImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);

    bool parseProcessNode();

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    const ParsePropsMap& parseProps() const
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

    const ParsePropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    ParsePropsMap& parseExtraAttributes()
    {
        return m_extraAttrs;
    }

    const ParseContentsList& parseExtraChildrenElements() const
    {
        return m_extraChildren;
    }

    ParseContentsList& parseExtraChildrenElements()
    {
        return m_extraChildren;
    }

    const ParseNamespacesMap& parseNamespaces() const
    {
        return m_namespaces;
    }

    ParseNamespacesList parseNamespacesList() const;    

    const ParseFieldImpl* parseFindField(const std::string& ref, bool checkRef = true) const;

    const ParseMessageImpl* parseFindMessage(const std::string& ref, bool checkRef = true) const;

    const ParseInterfaceImpl* parseFindInterface(const std::string& ref, bool checkRef = true) const;    

    const ParsePlatformsList& parsePlatforms() const
    {
        return m_platforms;
    }

    ParseMessagesList parseAllMessages() const;
    ParseInterfacesList parseAllInterfaces() const;
    ParseImplInterfacesList parseAllImplInterfaces() const;

    bool parseAddPlatform(const std::string& name);
    void parseAddNamespace(ParseNamespaceImplPtr ns);
    ParseNamespaceImpl& parseDefaultNamespace();

    bool parseValidateAllMessages();
    unsigned parseCountMessageIds() const;

    std::string parseExternalRef() const;

    ParseFieldRefInfosList parseProcessInterfaceFieldRef(const std::string& refStr) const;

protected:
    virtual ParseObjKind parseObjKindImpl() const override;    

private:

    bool parseUpdateStringProperty(const ParsePropsMap& map, const std::string& name, std::string& prop);
    bool parseUpdateUnsignedProperty(const ParsePropsMap& map, const std::string& name, unsigned& prop);
    bool parseUpdateEndianProperty(const ParsePropsMap& map, const std::string& name, ParseEndian& prop);
    bool parseUpdateBooleanProperty(const ParsePropsMap& map, const std::string& name, bool& prop);
    bool parseUpdateExtraAttrs();
    bool parseUpdateExtraChildren();
    const ParseNamespaceImpl* parseGetNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const;

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;

    ParsePropsMap m_props;
    ParsePropsMap m_extraAttrs;
    ParseContentsList m_extraChildren;
    std::string m_name;
    std::string m_description;
    ParseNamespacesMap m_namespaces;
    ParsePlatformsList m_platforms;
    unsigned m_id = 0U;
    unsigned m_version = 0;
    unsigned m_dslVersion = 0;
    ParseEndian m_endian = ParseEndian_NumOfValues;
    bool m_nonUniqueMsgIdAllowed = false;
};

using ParseSchemaImplPtr = std::unique_ptr<ParseSchemaImpl>;

} // namespace parse

} // namespace commsdsl
