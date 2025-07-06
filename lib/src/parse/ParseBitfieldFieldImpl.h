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

#include "ParseFieldImpl.h"
#include "ParseOptCondImpl.h"

#include "commsdsl/parse/ParseBitfieldField.h"
#include "commsdsl/parse/ParseEndian.h"

namespace commsdsl
{

namespace parse
{

class ParseBitfieldFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    ParseBitfieldFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseBitfieldFieldImpl(const ParseBitfieldFieldImpl& other);
    using Members = ParseBitfieldField::Members;

    ParseEndian parseEndian() const
    {
        return m_endian;
    }

    Members parseMembersList() const;

    static const ParseXmlWrap::NamesList& parseSupportedTypes();

    ParseOptCond parseValidCond() const
    {
        return ParseOptCond(m_validCond.get());
    }       

protected:

    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPossiblePropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl &other) override;
    virtual bool parseImpl() override;
    virtual bool parseReplaceMembersImpl(FieldsList& members) override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const override;
    virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool parseVerifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const override;
    virtual bool parseVerifyAliasedMemberImpl(const std::string& fieldName) const override;
    virtual const ParseXmlWrap::NamesList& parseSupportedMemberTypesImpl() const override;
    virtual const FieldsList& parseMembersImpl() const override;

private:
    bool parseUpdateEndian();
    bool parseUpdateMembers();
    bool parseUpdateSingleValidCond();
    bool parseUpdateMultiValidCond();
    bool parseCopyValidCond();

    bool parseUpdateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond);
    bool parseUpdateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond);    

    ParseEndian m_endian = ParseEndian_NumOfValues;
    FieldsList m_members;
    ParseOptCondImplPtr m_validCond;
};

} // namespace parse

} // namespace commsdsl
