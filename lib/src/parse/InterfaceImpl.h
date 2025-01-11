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

#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"
#include "commsdsl/parse/Interface.h"
#include "commsdsl/parse/Protocol.h"
#include "FieldImpl.h"
#include "AliasImpl.h"
#include "BundleFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ProtocolImpl;
class InterfaceImpl final : public Object
{
    using Base = Object;
public:
    using Ptr = std::unique_ptr<InterfaceImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using FieldsList = Interface::FieldsList;
    using AliasesList = Interface::AliasesList;
    using ContentsList = XmlWrap::ContentsList;
    using ImplFieldsList = std::vector<FieldImpl*>;
    using FieldRefInfo = FieldImpl::FieldRefInfo;
    using FieldRefType = FieldImpl::FieldRefType;

    InterfaceImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    InterfaceImpl(const InterfaceImpl&) = delete;
    InterfaceImpl(InterfaceImpl&&) = default;
    virtual ~InterfaceImpl() = default;

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
    const std::string& displayName() const;
    const std::string& description() const;
    const std::string& copyCodeFrom() const;

    FieldsList fieldsList() const;
    AliasesList aliasesList() const;

    std::string externalRef(bool schemaRef) const;

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_extraChildren;
    }

    std::size_t findFieldIdx(const std::string& name) const;

    ImplFieldsList allImplFields() const;

    FieldRefInfo processInnerFieldRef(const std::string refStr) const;

protected:

    virtual ObjKind objKindImpl() const override;

private:
    struct ReusableState
    {
        std::string m_name;
        std::string m_description;
        std::vector<FieldImplPtr> m_fields;
        std::vector<AliasImplPtr> m_aliases;
        std::string m_copyCodeFrom;

        ReusableState() = default;
        ReusableState(ReusableState&&) = default;

        auto basicForwardAsTuple()
        {
            return 
                std::forward_as_tuple(
                    m_name,
                    m_description,
                    // m_fields,
                    // m_aliases,
                    m_copyCodeFrom
                );
        }

        auto basicForwardAsTuple() const
        {
            return const_cast<ReusableState*>(this)->basicForwardAsTuple();
        }

        ReusableState& operator=(const ReusableState& other)
        {
            basicForwardAsTuple() = other.basicForwardAsTuple();

            m_fields.clear();
            m_fields.reserve(other.m_fields.size());
            for (auto& f : other.m_fields) {
                m_fields.push_back(f->clone());
            }

            m_aliases.clear();
            m_aliases.reserve(other.m_aliases.size());
            for (auto& a : other.m_aliases) {
                m_aliases.push_back(a->clone());
            }            

            return *this;          
        }
    };

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    static const XmlWrap::NamesList& commonProps();
    static XmlWrap::NamesList allNames();

    bool validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, std::string& value, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool checkReuse();
    bool updateName();
    bool updateDescription();
    bool copyFields();
    bool updateFields();
    bool copyAliases();
    bool updateAliases();
    void cloneFieldsFrom(const InterfaceImpl& other);
    void cloneFieldsFrom(const BundleFieldImpl& other);
    void cloneAliasesFrom(const InterfaceImpl& other);
    void cloneAliasesFrom(const BundleFieldImpl& other);
    bool updateExtraAttrs();
    bool updateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;
    ReusableState m_state;

    const InterfaceImpl* m_copyFieldsFromInterface = nullptr;
    const BundleFieldImpl* m_copyFieldsFromBundle = nullptr;
};

using InterfaceImplPtr = InterfaceImpl::Ptr;

} // namespace parse

} // namespace commsdsl
