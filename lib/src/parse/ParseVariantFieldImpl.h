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

#include "commsdsl/parse/ParseVariantField.h"
#include "ParseFieldImpl.h"

#include <cstdint>
#include <limits>


namespace commsdsl
{

namespace parse
{

class ParseVariantFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    ParseVariantFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseVariantFieldImpl(const ParseVariantFieldImpl& other);
    using Members = ParseVariantField::Members;

    Members parseMembersList() const;

    std::size_t parseDefaultMemberIdx() const
    {
        return m_state.m_defaultIdx;
    }

protected:

    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl &other) override;
    virtual bool parseImpl() override;
    virtual bool parseReplaceMembersImpl(FieldsList& members) override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseMaxLengthImpl() const override;
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const override;
    virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual const FieldsList& parseMembersImpl() const override;

private:

    bool parseUpdateMembers();
    bool parseUpdateDefaultMember();
    bool parseUpdateIdxHidden();

    struct ReusableState
    {
        std::size_t m_defaultIdx = std::numeric_limits<std::size_t>::max();
    };

    ReusableState m_state;
    FieldsList m_members;
};

} // namespace parse

} // namespace commsdsl
