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

#include <memory>
#include <map>
#include <string>
#include <cstdint>

#include "ParseXmlWrap.h"
#include "ParseLogger.h"
#include "ParseObject.h"
#include "commsdsl/parse/ParseFrame.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "ParseLayerImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseFrameImpl final : public ParseObject
{
    using Base = ParseObject;
public:
    using Ptr = std::unique_ptr<ParseFrameImpl>;
    using PropsMap = ParseXmlWrap::PropsMap;
    using LayersList = ParseFrame::LayersList;
    using ContentsList = ParseXmlWrap::ContentsList;

    ParseFrameImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseFrameImpl(const ParseFrameImpl&) = delete;
    ParseFrameImpl(ParseFrameImpl&&) = default;
    virtual ~ParseFrameImpl() = default;

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& description() const;

    LayersList layersList() const;

    std::string externalRef(bool schemaRef) const;

    const PropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_extraChildren;
    }

protected:

    virtual ObjKind objKindImpl() const override;

private:
    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    static const ParseXmlWrap::NamesList& commonProps();
    static ParseXmlWrap::NamesList allNames();

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool updateName();
    bool updateDescription();
    bool updateLayers();
    void cloneLayersFrom(const ParseFrameImpl& other);
    bool updateExtraAttrs();
    bool updateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;

    const std::string* m_name = nullptr;
    const std::string* m_description = nullptr;
    std::vector<ParseLayerImplPtr> m_layers;
};

using ParseFrameImplPtr = ParseFrameImpl::Ptr;

} // namespace parse

} // namespace commsdsl
