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

    static Ptr create(const std::string& kind, ::xmlNodePtr node, ParseProtocolImpl& protocol);

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();
    bool verify(const LayersList& layers)
    {
        return verifyImpl(layers);
    }

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& description() const;

    Kind kind() const
    {
        return kindImpl();
    }

    bool hasField() const
    {
        return (m_extField != nullptr) ||
               static_cast<bool>(m_field);
    }

    ParseField field() const
    {
        if (m_extField != nullptr) {
            return ParseField(m_extField);
        }

        return ParseField(m_field.get());
    }

    static ParseXmlWrap::NamesList supportedTypes();

    const ParseXmlWrap::NamesList& extraPropsNames() const
    {
        return extraPropsNamesImpl();
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


protected:
    ParseLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseLayerImpl(const ParseLayerImpl&) = delete;

    ParseProtocolImpl& protocol()
    {
        return m_protocol;
    }

    const ParseProtocolImpl& protocol() const
    {
        return m_protocol;
    }

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    virtual ObjKind objKindImpl() const override final;
    virtual Kind kindImpl() const = 0;
    virtual const ParseXmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual bool parseImpl();
    virtual bool verifyImpl(const LayersList& layers);
    virtual bool mustHaveFieldImpl() const;

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool verifySingleLayer(const LayersList& layers, const std::string& kindStr);
    bool verifyBeforePayload(const LayersList& layers);
    std::size_t findThisLayerIndex(const LayersList& layers) const;
    std::size_t findLayerIndex(const LayersList& layers, Kind lKind);
    std::size_t findLayerIndex(const LayersList& layers, const std::string& name);

    static const ParseXmlWrap::NamesList& commonProps();
    static const ParseXmlWrap::NamesList& commonPossibleProps();

private:

    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ParseProtocolImpl& p)>;
    using CreateMap = std::map<std::string, CreateFunc>;

    bool updateName();
    bool updateDescription();
    bool updateField();
    bool updateExtraAttrs(const ParseXmlWrap::NamesList& names);
    bool updateExtraChildren(const ParseXmlWrap::NamesList& names);
    bool checkFieldFromRef();
    bool checkFieldAsChild();

    static const CreateMap& createMap();

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
