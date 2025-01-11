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

#include "commsdsl/parse/BundleField.h"

#include "AliasImpl.h"
#include "FieldImpl.h"
#include "OptCondImpl.h"

#include <cstdint>

namespace commsdsl
{

namespace parse
{

class BundleFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    BundleFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    BundleFieldImpl(const BundleFieldImpl& other);
    using Members = BundleField::Members;
    using AliasesList = BundleField::Aliases;

    Members membersList() const;
    AliasesList aliasesList() const;

    const std::vector<AliasImplPtr>& aliases() const
    {
        return m_aliases;
    }

    OptCond validCond() const
    {
        return OptCond(m_validCond.get());
    }    

    const OptCondImplPtr& validCondImpl() const
    {
        return m_validCond;
    }    


protected:

    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl &other) override;
    virtual bool parseImpl() override;
    virtual bool replaceMembersImpl(FieldsList& members) override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual bool verifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const override;
    virtual bool verifyAliasedMemberImpl(const std::string& fieldName) const override;
    virtual const XmlWrap::NamesList& supportedMemberTypesImpl() const override;
    virtual const FieldsList& membersImpl() const override;

private:
    bool updateMembers();
    bool updateAliases();
    bool updateSingleValidCond();
    bool updateMultiValidCond();
    bool copyValidCond();

    bool updateSingleCondInternal(const std::string& prop, OptCondImplPtr& cond);
    bool updateMultiCondInternal(const std::string& prop, OptCondImplPtr& cond);    

    FieldsList m_members;
    std::vector<AliasImplPtr> m_aliases;
    OptCondImplPtr m_validCond;    
};

} // namespace parse

} // namespace commsdsl
