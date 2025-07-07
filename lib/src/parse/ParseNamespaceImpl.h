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

#include "commsdsl/parse/ParseNamespace.h"
#include "ParseFieldImpl.h"
#include "ParseFrameImpl.h"
#include "ParseInterfaceImpl.h"
#include "ParseLogger.h"
#include "ParseMessageImpl.h"
#include "ParseObject.h"
#include "ParseXmlWrap.h"

#include <cstdint>
#include <memory>
#include <algorithm>
#include <cctype>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseNamespaceImpl final : public ParseObject
{
public:
    struct ParseKeyComp
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

    using ParsePtr = std::unique_ptr<ParseNamespaceImpl>;
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;
    using ParseNamespacesList = ParseNamespace::ParseNamespacesList;
    using ParseFieldsList = ParseNamespace::ParseFieldsList;
    using ParseMessagesList = ParseNamespace::ParseMessagesList;
    using ParseInterfacesList = ParseNamespace::ParseInterfacesList;
    using ParseImplInterfacesList = std::vector<ParseInterfaceImpl*>;
    using ParseFramesList = ParseNamespace::ParseFramesList;
    using ParseNamespacesMap = std::map<std::string, ParsePtr>;
    using ParseFieldsMap = std::map<std::string, ParseFieldImplPtr, ParseKeyComp>;
    using ParseMessagesMap = std::map<std::string, ParseMessageImplPtr, ParseKeyComp>;
    using ParseInterfacesMap = std::map<std::string, ParseInterfaceImplPtr, ParseKeyComp>;
    using ParseFramesMap = std::map<std::string, ParseFrameImplPtr, ParseKeyComp>;
    using ParseFieldRefInfo = ParseFieldImpl::ParseFieldRefInfo;
    using ParseFieldRefInfosList = ParseFieldImpl::ParseFieldRefInfosList;

    ParseNamespaceImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    virtual ~ParseNamespaceImpl() = default;

    static const ParseXmlWrap::ParseNamesList& expectedChildrenNames();

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parseProps();

    bool parseChildren(ParseNamespaceImpl* realNs = nullptr);

    bool parse();

    bool processChild(::xmlNodePtr node, ParseNamespaceImpl* realNs = nullptr);

    static const ParseXmlWrap::ParseNamesList& parseSupportedChildren();

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

    void parseUpdateDescription(const std::string& value)
    {
        m_description = value;
    }

    const ParseInterfacesMap& parseInterfaces() const
    {
        return m_interfaces;
    }

    ParseNamespacesList parseNamespacesList() const;
    ParseFieldsList parseFieldsList() const;
    ParseMessagesList parseMessagesList() const;
    ParseInterfacesList parseInterfacesList() const;
    ParseFramesList parseFramesList() const;

    const ParseMessagesMap& parseMessages() const
    {
        return m_messages;
    }

    const ParsePropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    ParsePropsMap& parseExtraAttributes()
    {
        return m_extraAttrs;
    }

    const ParseContentsList& parseExtraChildren() const
    {
        return m_extraChildren;
    }

    ParseContentsList& parseExtraChildren()
    {
        return m_extraChildren;
    }

    const ParseNamespacesMap& parseNamespacesMap() const
    {
        return m_namespaces;
    }

    const ParseFieldImpl* parseFindField(const std::string& fieldName) const;
    const ParseMessageImpl* parseFindMessage(const std::string& msgName) const;
    const ParseInterfaceImpl* parseFindInterface(const std::string& intName) const;
    const ParseFrameImpl* parseFindFrame(const std::string& intName) const;

    std::string parseExternalRef(bool schemaRef) const;

    unsigned parseCountMessageIds() const;

    bool parseStrToNumeric(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const;
    bool parseStrToFp(const std::string& ref, double& val) const;
    bool parseStrToBool(const std::string& ref, bool& val) const;
    bool parseStrToString(const std::string& ref, std::string& val) const;
    bool parseStrToData(const std::string& ref, std::vector<std::uint8_t>& val) const;

    ParseImplInterfacesList parseAllImplInterfaces() const;

    ParseFieldRefInfosList parseProcessInterfaceFieldRef(const std::string& refStr) const;

    bool parseValidateAllMessages(bool allowNonUniquIds);

protected:
    virtual ParseObjKind parseObjKindImpl() const override;

private:
    using ParseStrToValueNsConvertFunc = std::function<bool (const ParseNamespaceImpl& ns, const std::string& ref)>;
    using ParseStrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;

    bool processNamespace(::xmlNodePtr node);
    bool processMultipleFields(::xmlNodePtr node);
    bool processMessage(::xmlNodePtr node);
    bool processMultipleMessages(::xmlNodePtr node);
    bool processInterface(::xmlNodePtr node);
    bool processMultipleInterfaces(::xmlNodePtr node);
    bool processFrame(::xmlNodePtr node);
    bool processMultipleFrames(::xmlNodePtr node);
    bool parseUpdateExtraAttrs();
    bool parseUpdateExtraChildren();
    bool parseStrToValue(const std::string& ref, ParseStrToValueNsConvertFunc&& nsFunc, ParseStrToValueFieldConvertFunc&& fFunc) const;

    ParseLogWrapper parseLogError() const;
    ParseLogWrapper parseLogWarning() const;
    ParseLogWrapper parseLogInfo() const;

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;

    ParsePropsMap m_props;
    ParsePropsMap m_extraAttrs;
    ParseContentsList m_extraChildren;

    std::string m_name;
    std::string m_description;

    ParseNamespacesMap m_namespaces;
    ParseFieldsMap m_fields;
    ParseMessagesMap m_messages;
    ParseInterfacesMap m_interfaces;
    ParseFramesMap m_frames;
};

using ParseNamespaceImplPtr = ParseNamespaceImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
