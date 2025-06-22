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
#include "commsdsl/parse/ParseInterface.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "ParseFieldImpl.h"
#include "ParseAliasImpl.h"
#include "ParseBundleFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseInterfaceImpl final : public ParseObject
{
    using Base = ParseObject;
public:
    using Ptr = std::unique_ptr<ParseInterfaceImpl>;
    using PropsMap = ParseXmlWrap::PropsMap;
    using FieldsList = ParseInterface::FieldsList;
    using AliasesList = ParseInterface::AliasesList;
    using ContentsList = ParseXmlWrap::ContentsList;
    using ImplFieldsList = std::vector<ParseFieldImpl*>;
    using FieldRefInfo = ParseFieldImpl::FieldRefInfo;
    using FieldRefType = ParseFieldImpl::FieldRefType;

    ParseInterfaceImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseInterfaceImpl(const ParseInterfaceImpl&) = delete;
    ParseInterfaceImpl(ParseInterfaceImpl&&) = default;
    virtual ~ParseInterfaceImpl() = default;

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
        std::vector<ParseFieldImplPtr> m_fields;
        std::vector<ParseAliasImplPtr> m_aliases;
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

    static const ParseXmlWrap::NamesList& commonProps();
    static ParseXmlWrap::NamesList allNames();

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
    void cloneFieldsFrom(const ParseInterfaceImpl& other);
    void cloneFieldsFrom(const ParseBundleFieldImpl& other);
    void cloneAliasesFrom(const ParseInterfaceImpl& other);
    void cloneAliasesFrom(const ParseBundleFieldImpl& other);
    bool updateExtraAttrs();
    bool updateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;
    ReusableState m_state;

    const ParseInterfaceImpl* m_copyFieldsFromInterface = nullptr;
    const ParseBundleFieldImpl* m_copyFieldsFromBundle = nullptr;
};

using ParseInterfaceImplPtr = ParseInterfaceImpl::Ptr;

} // namespace parse

} // namespace commsdsl
