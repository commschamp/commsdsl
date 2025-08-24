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

#include "commsdsl/parse/ParseLayer.h"
#include "ParseFieldImpl.h"
#include "ParseLogger.h"
#include "ParseObject.h"
#include "ParseXmlWrap.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseLayerImpl : public ParseObject
{
    using Base = ParseObject;
public:
    using ParsePtr = std::unique_ptr<ParseLayerImpl>;
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;
    using ParseLayersList = std::vector<ParsePtr>;
    using ParseKind = ParseLayer::ParseKind;

    virtual ~ParseLayerImpl() = default;

    static ParsePtr parseCreate(const std::string& kind, ::xmlNodePtr node, ParseProtocolImpl& protocol);

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parse();
    bool parseVerify(const ParseLayersList& layers)
    {
        return parseVerifyImpl(layers);
    }

    const ParsePropsMap& parseProps() const
    {
        return m_props;
    }

    const std::string& parseName() const;
    const std::string& parseDisplayName() const;
    const std::string& parseDescription() const;

    ParseKind parseKind() const
    {
        return parseKindImpl();
    }

    bool parseHasField() const
    {
        return (m_extField != nullptr) ||
               static_cast<bool>(m_field);
    }

    ParseField parseField() const
    {
        if (m_extField != nullptr) {
            return ParseField(m_extField);
        }

        return ParseField(m_field.get());
    }

    static ParseXmlWrap::ParseNamesList parseSupportedTypes();

    const ParseXmlWrap::ParseNamesList& parseExtraPropsNames() const
    {
        return parseExtraPropsNamesImpl();
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


protected:
    ParseLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseLayerImpl(const ParseLayerImpl&) = delete;

    ParseProtocolImpl& parseProtocol()
    {
        return m_protocol;
    }

    const ParseProtocolImpl& parseProtocol() const
    {
        return m_protocol;
    }

    ParseLogWrapper parseLogError() const;
    ParseLogWrapper parseLogWarning() const;
    ParseLogWrapper parseLogInfo() const;

    virtual ParseObjKind parseObjKindImpl() const override final;
    virtual ParseKind parseKindImpl() const = 0;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const;
    virtual bool parseImpl();
    virtual bool parseVerifyImpl(const ParseLayersList& layers);
    virtual bool parseMustHaveFieldImpl() const;

    bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool parseValidateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool parseVerifySingleLayer(const ParseLayersList& layers, const std::string& kindStr);
    bool parseVerifyBeforePayload(const ParseLayersList& layers);
    std::size_t parseFindThisLayerIndex(const ParseLayersList& layers) const;
    std::size_t parseFindLayerIndex(const ParseLayersList& layers, ParseKind lKind);
    std::size_t parseFindLayerIndex(const ParseLayersList& layers, const std::string& name);

    static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    static const ParseXmlWrap::ParseNamesList& parseCommonPossibleProps();

private:

    using ParseCreateFunc = std::function<ParsePtr (::xmlNodePtr n, ParseProtocolImpl& p)>;
    using ParseCreateMap = std::map<std::string, ParseCreateFunc>;

    bool parseUpdateName();
    bool parseUpdateDisplayName();
    bool parseUpdateDescription();
    bool parseUpdateField();
    bool parseUpdateExtraAttrs(const ParseXmlWrap::ParseNamesList& names);
    bool parseUpdateExtraChildren(const ParseXmlWrap::ParseNamesList& names);
    bool parseCheckFieldFromRef();
    bool parseCheckFieldAsChild();

    static const ParseCreateMap& parseCreateMap();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    ParsePropsMap m_props;
    const std::string* m_name = nullptr;
    const std::string* m_displayName = nullptr;
    const std::string* m_description = nullptr;
    const ParseFieldImpl* m_extField = nullptr;
    ParseFieldImplPtr m_field;
    ParsePropsMap m_extraAttrs;
    ParseContentsList m_extraChildren;
};

using ParseLayerImplPtr = ParseLayerImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
