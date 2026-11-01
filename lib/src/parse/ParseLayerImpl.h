//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

    [[nodiscard]] static ParsePtr parseCreate(const std::string& kind, ::xmlNodePtr node, ParseProtocolImpl& protocol);

    [[nodiscard]] ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    [[nodiscard]] bool parse();
    [[nodiscard]] bool parseVerify(const ParseLayersList& layers)
    {
        return parseVerifyImpl(layers);
    }

    [[nodiscard]] const ParsePropsMap& parseProps() const
    {
        return m_props;
    }

    [[nodiscard]] const std::string& parseName() const;
    [[nodiscard]] const std::string& parseDisplayName() const;
    [[nodiscard]] const std::string& parseDescription() const;

    [[nodiscard]] ParseKind parseKind() const
    {
        return parseKindImpl();
    }

    [[nodiscard]] bool parseHasField() const
    {
        return (m_extField != nullptr) ||
               static_cast<bool>(m_field);
    }

    [[nodiscard]] ParseField parseField() const
    {
        if (m_extField != nullptr) {
            return ParseField(m_extField);
        }

        return ParseField(m_field.get());
    }

    [[nodiscard]] static ParseXmlWrap::ParseNamesList parseSupportedTypes();

    [[nodiscard]] const ParseXmlWrap::ParseNamesList& parseExtraPropsNames() const
    {
        return parseExtraPropsNamesImpl();
    }

    [[nodiscard]] const ParsePropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    [[nodiscard]] ParsePropsMap& parseExtraAttributes()
    {
        return m_extraAttrs;
    }

    [[nodiscard]] const ParseContentsList& parseExtraChildren() const
    {
        return m_extraChildren;
    }

    [[nodiscard]] ParseContentsList& parseExtraChildren()
    {
        return m_extraChildren;
    }

    [[nodiscard]] std::string parseInnerRef(bool schemaRef = true) const;

protected:
    ParseLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseLayerImpl(const ParseLayerImpl&) = delete;

    [[nodiscard]] ParseProtocolImpl& parseProtocol()
    {
        return m_protocol;
    }

    [[nodiscard]] const ParseProtocolImpl& parseProtocol() const
    {
        return m_protocol;
    }

    [[nodiscard]] ParseLogWrapper parseLogError() const;
    [[nodiscard]] ParseLogWrapper parseLogWarning() const;
    [[nodiscard]] ParseLogWrapper parseLogInfo() const;

    virtual ParseObjKind parseObjKindImpl() const override final;
    [[nodiscard]] virtual ParseKind parseKindImpl() const = 0;
    [[nodiscard]] virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const;
    [[nodiscard]] virtual const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNamesImpl() const;
    [[nodiscard]] virtual bool parseImpl();
    [[nodiscard]] virtual bool parseVerifyImpl(const ParseLayersList& layers);
    [[nodiscard]] virtual bool parseMustHaveFieldImpl() const;

    [[nodiscard]] bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    [[nodiscard]] bool parseValidateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    [[nodiscard]] bool parseVerifySingleLayer(const ParseLayersList& layers, const std::string& kindStr);
    [[nodiscard]] bool parseVerifyBeforePayload(const ParseLayersList& layers);
    [[nodiscard]] bool parseVerifySuffixLayersOrder(const ParseLayersList& layers, std::size_t payloadIdx, std::size_t layerIdx, std::size_t fromIdx);
    [[nodiscard]] std::size_t parseFindThisLayerIndex(const ParseLayersList& layers) const;
    [[nodiscard]] std::size_t parseFindLayerIndex(const ParseLayersList& layers, ParseKind lKind);
    [[nodiscard]] std::size_t parseFindLayerIndex(const ParseLayersList& layers, const std::string& name);

    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseCommonPossibleProps();

private:

    using ParseCreateFunc = std::function<ParsePtr (::xmlNodePtr n, ParseProtocolImpl& p)>;
    using ParseCreateMap = std::map<std::string, ParseCreateFunc>;

    [[nodiscard]] bool parseUpdateName();
    [[nodiscard]] bool parseUpdateDisplayName();
    [[nodiscard]] bool parseUpdateDescription();
    [[nodiscard]] bool parseUpdateField();
    [[nodiscard]] bool parseUpdateExtraAttrs(const ParseXmlWrap::ParseNamesList& names);
    [[nodiscard]] bool parseUpdateExtraChildren(const ParseXmlWrap::ParseNamesList& names);
    [[nodiscard]] bool parseCheckFieldFromRef();
    [[nodiscard]] bool parseCheckFieldAsChild();

    [[nodiscard]] static const ParseCreateMap& parseCreateMap();

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
