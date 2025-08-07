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

#include "commsdsl/parse/ParseFrame.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "ParseLayerImpl.h"
#include "ParseLogger.h"
#include "ParseObject.h"
#include "ParseXmlWrap.h"

#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseFrameImpl final : public ParseObject
{
    using Base = ParseObject;
public:
    using ParsePtr = std::unique_ptr<ParseFrameImpl>;
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseLayersList = ParseFrame::ParseLayersList;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;

    ParseFrameImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseFrameImpl(const ParseFrameImpl&) = delete;
    ParseFrameImpl(ParseFrameImpl&&) = default;
    virtual ~ParseFrameImpl() = default;

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parse();

    const ParsePropsMap& parseProps() const
    {
        return m_props;
    }

    const std::string& parseName() const;
    const std::string& parseDisplayName() const;
    const std::string& parseDescription() const;

    ParseLayersList parseLayersList() const;

    std::string parseExternalRef(bool schemaRef) const;

    const ParsePropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    const ParseContentsList& parseExtraChildren() const
    {
        return m_extraChildren;
    }

protected:

    virtual ParseObjKind parseObjKindImpl() const override;

private:
    ParseLogWrapper parseLogError() const;
    ParseLogWrapper parseLogWarning() const;
    ParseLogWrapper parseLogInfo() const;

    static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    static ParseXmlWrap::ParseNamesList parseAllNames();

    bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool parseValidateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool parseUpdateName();
    bool parseUpdateDisplayName();
    bool parseUpdateDescription();
    bool parseUpdateLayers();
    bool parseUpdateExtraAttrs();
    bool parseUpdateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    ParsePropsMap m_props;
    ParsePropsMap m_extraAttrs;
    ParseContentsList m_extraChildren;

    const std::string* m_name = nullptr;
    const std::string* m_displayName = nullptr;
    const std::string* m_description = nullptr;
    std::vector<ParseLayerImplPtr> m_layers;
};

using ParseFrameImplPtr = ParseFrameImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
