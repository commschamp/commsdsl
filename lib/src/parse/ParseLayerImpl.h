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
#include <functional>
#include <string>

#include "commsdsl/parse/ParseLayer.h"
#include "ParseXmlWrap.h"
#include "ParseLogger.h"
#include "ParseObject.h"
#include "ParseFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseLayerImpl : public ParseObject
{
    using Base = ParseObject;
public:
    using Ptr = std::unique_ptr<ParseLayerImpl>;
    using PropsMap = ParseXmlWrap::PropsMap;
    using ContentsList = ParseXmlWrap::ContentsList;
    using LayersList = std::vector<Ptr>;
    using Kind = ParseLayer::Kind;

    virtual ~ParseLayerImpl() = default;

    static Ptr parseCreate(const std::string& kind, ::xmlNodePtr node, ParseProtocolImpl& protocol);

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parse();
    bool parseVerify(const LayersList& layers)
    {
        return parseVerifyImpl(layers);
    }

    const PropsMap& parseProps() const
    {
        return m_props;
    }

    const std::string& parseName() const;
    const std::string& parseDescription() const;

    Kind parseKind() const
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

    static ParseXmlWrap::NamesList parseSupportedTypes();

    const ParseXmlWrap::NamesList& parseExtraPropsNames() const
    {
        return parseExtraPropsNamesImpl();
    }

    const PropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    PropsMap& parseExtraAttributes()
    {
        return m_extraAttrs;
    }

    const ContentsList& parseExtraChildren() const
    {
        return m_extraChildren;
    }

    ContentsList& parseExtraChildren()
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

    LogWrapper parseLogError() const;
    LogWrapper parseLogWarning() const;
    LogWrapper parseLogInfo() const;

    virtual ObjKind parseObjKindImpl() const override final;
    virtual Kind parseKindImpl() const = 0;
    virtual const ParseXmlWrap::NamesList& parseExtraPropsNamesImpl() const;
    virtual bool parseImpl();
    virtual bool parseVerifyImpl(const LayersList& layers);
    virtual bool parseMustHaveFieldImpl() const;

    bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool parseValidateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool parseVerifySingleLayer(const LayersList& layers, const std::string& kindStr);
    bool parseVerifyBeforePayload(const LayersList& layers);
    std::size_t parseFindThisLayerIndex(const LayersList& layers) const;
    std::size_t parseFindLayerIndex(const LayersList& layers, Kind lKind);
    std::size_t parseFindLayerIndex(const LayersList& layers, const std::string& name);

    static const ParseXmlWrap::NamesList& parseCommonProps();
    static const ParseXmlWrap::NamesList& parseCommonPossibleProps();

private:

    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ParseProtocolImpl& p)>;
    using CreateMap = std::map<std::string, CreateFunc>;

    bool parseUpdateName();
    bool parseUpdateDescription();
    bool parseUpdateField();
    bool parseUpdateExtraAttrs(const ParseXmlWrap::NamesList& names);
    bool parseUpdateExtraChildren(const ParseXmlWrap::NamesList& names);
    bool parseCheckFieldFromRef();
    bool parseCheckFieldAsChild();

    static const CreateMap& parseCreateMap();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    PropsMap m_props;
    const std::string* m_name = nullptr;
    const std::string* m_description = nullptr;
    const ParseFieldImpl* m_extField = nullptr;
    ParseFieldImplPtr m_field;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;
};

using ParseLayerImplPtr = ParseLayerImpl::Ptr;

} // namespace parse

} // namespace commsdsl
