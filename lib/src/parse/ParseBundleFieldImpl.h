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

#include "commsdsl/parse/ParseBundleField.h"

#include "commsdsl/parse/ParseField.h"
#include "ParseAliasImpl.h"
#include "ParseOptCondImpl.h"

#include <cstdint>

namespace commsdsl
{

namespace parse
{

class ParseBundleFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using ParseMembers = ParseBundleField::ParseMembers;
    using ParseAliasesList = ParseBundleField::ParseAliases;

    ParseBundleFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseBundleFieldImpl(const ParseBundleFieldImpl& other);

    ParseMembers parseMembersList() const;
    ParseAliasesList parseAliasesList() const;

    const std::vector<ParseAliasImplPtr>& parseAliases() const
    {
        return m_aliases;
    }

    ParseOptCond parseValidCond() const
    {
        return ParseOptCond(m_validCond.get());
    }    

    const ParseOptCondImplPtr& parseValidCondImpl() const
    {
        return m_validCond;
    }    


protected:

    virtual ParseKind parseKindImpl() const override;
    virtual ParsePtr parseCloneImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual bool parseReplaceMembersImpl(ParseFieldsList& members) override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseMaxLengthImpl() const override;
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const override;
    virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual bool parseVerifySemanticTypeImpl(::xmlNodePtr node, ParseSemanticType type) const override;
    virtual bool parseVerifyAliasedMemberImpl(const std::string& fieldName) const override;
    virtual const ParseXmlWrap::ParseNamesList& parseSupportedMemberTypesImpl() const override;
    virtual const ParseFieldsList& parseMembersImpl() const override;

private:
    bool parseUpdateMembers();
    bool parseUpdateAliases();
    bool parseUpdateSingleValidCond();
    bool parseUpdateMultiValidCond();
    bool parseCopyValidCond();

    bool parseUpdateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond);
    bool parseUpdateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond);    

    ParseFieldsList m_members;
    std::vector<ParseAliasImplPtr> m_aliases;
    ParseOptCondImplPtr m_validCond;    
};

} // namespace parse

} // namespace commsdsl
