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

#include <cstdint>
#include <memory>
#include <algorithm>
#include <cctype>

#include "commsdsl/parse/ParseNamespace.h"

#include "ParseXmlWrap.h"
#include "ParseLogger.h"
#include "ParseFieldImpl.h"
#include "ParseMessageImpl.h"
#include "ParseInterfaceImpl.h"
#include "ParseFrameImpl.h"
#include "ParseObject.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseNamespaceImpl final : public ParseObject
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

    using Ptr = std::unique_ptr<ParseNamespaceImpl>;
    using PropsMap = ParseXmlWrap::PropsMap;
    using ContentsList = ParseXmlWrap::ContentsList;
    using NamespacesList = ParseNamespace::NamespacesList;
    using FieldsList = ParseNamespace::FieldsList;
    using MessagesList = ParseNamespace::MessagesList;
    using InterfacesList = ParseNamespace::InterfacesList;
    using ImplInterfacesList = std::vector<ParseInterfaceImpl*>;
    using FramesList = ParseNamespace::FramesList;
    using NamespacesMap = std::map<std::string, Ptr>;
    using FieldsMap = std::map<std::string, ParseFieldImplPtr, KeyComp>;
    using MessagesMap = std::map<std::string, ParseMessageImplPtr, KeyComp>;
    using InterfacesMap = std::map<std::string, ParseInterfaceImplPtr, KeyComp>;
    using FramesMap = std::map<std::string, ParseFrameImplPtr, KeyComp>;
    using FieldRefInfo = ParseFieldImpl::FieldRefInfo;
    using FieldRefInfosList = ParseFieldImpl::FieldRefInfosList;

    ParseNamespaceImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    virtual ~ParseNamespaceImpl() = default;

    static const ParseXmlWrap::NamesList& expectedChildrenNames();

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parseProps();

    bool parseChildren(ParseNamespaceImpl* realNs = nullptr);

    bool parse();

    bool processChild(::xmlNodePtr node, ParseNamespaceImpl* realNs = nullptr);

    static const ParseXmlWrap::NamesList& supportedChildren();

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

    const InterfacesMap& interfaces() const
    {
        return m_interfaces;
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

    const PropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    PropsMap& parseExtraAttributes()
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

    const ParseFieldImpl* findField(const std::string& fieldName) const;
    const ParseMessageImpl* findMessage(const std::string& msgName) const;
    const ParseInterfaceImpl* findInterface(const std::string& intName) const;
    const ParseFrameImpl* findFrame(const std::string& intName) const;

    std::string externalRef(bool schemaRef) const;

    unsigned countMessageIds() const;

    bool strToNumeric(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const;
    bool strToFp(const std::string& ref, double& val) const;
    bool strToBool(const std::string& ref, bool& val) const;
    bool strToString(const std::string& ref, std::string& val) const;
    bool strToData(const std::string& ref, std::vector<std::uint8_t>& val) const;

    ImplInterfacesList allImplInterfaces() const;

    FieldRefInfosList processInterfaceFieldRef(const std::string& refStr) const;

    bool validateAllMessages(bool allowNonUniquIds);

protected:
    virtual ObjKind objKindImpl() const override;

private:

    using StrToValueNsConvertFunc = std::function<bool (const ParseNamespaceImpl& ns, const std::string& ref)>;
    using StrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;

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
    bool strToValue(const std::string& ref, StrToValueNsConvertFunc&& nsFunc, StrToValueFieldConvertFunc&& fFunc) const;

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;

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

using ParseNamespaceImplPtr = ParseNamespaceImpl::Ptr;

} // namespace parse

} // namespace commsdsl
