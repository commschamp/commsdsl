//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/Layer.h"
#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"
#include "FieldImpl.h"

namespace commsdsl
{

class ProtocolImpl;
class LayerImpl : public Object
{
    using Base = Object;
public:
    using Ptr = std::unique_ptr<LayerImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using LayersList = std::vector<Ptr>;
    using Kind = Layer::Kind;

    virtual ~LayerImpl() = default;

    static Ptr create(const std::string& kind, ::xmlNodePtr node, ProtocolImpl& protocol);

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

    Field field() const
    {
        if (m_extField != nullptr) {
            return Field(m_extField);
        }

        return Field(m_field.get());
    }

    static XmlWrap::NamesList supportedTypes();

    const XmlWrap::NamesList& extraPropsNames() const
    {
        return extraPropsNamesImpl();
    }

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    PropsMap& extraAttributes()
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
    LayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    LayerImpl(const LayerImpl&) = delete;

    ProtocolImpl& protocol()
    {
        return m_protocol;
    }

    const ProtocolImpl& protocol() const
    {
        return m_protocol;
    }

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    virtual ObjKind objKindImpl() const override final;
    virtual Kind kindImpl() const = 0;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const;
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

    static const XmlWrap::NamesList& commonProps();
    static const XmlWrap::NamesList& commonPossibleProps();

private:

    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ProtocolImpl& p)>;
    using CreateMap = std::map<std::string, CreateFunc>;

    bool updateName();
    bool updateDescription();
    bool updateField();
    bool updateExtraAttrs(const XmlWrap::NamesList& names);
    bool updateExtraChildren(const XmlWrap::NamesList& names);
    bool checkFieldFromRef();
    bool checkFieldAsChild();

    static const CreateMap& createMap();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
    const std::string* m_name = nullptr;
    const std::string* m_description = nullptr;
    const FieldImpl* m_extField = nullptr;
    FieldImplPtr m_field;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;
};

using LayerImplPtr = LayerImpl::Ptr;

} // namespace commsdsl
