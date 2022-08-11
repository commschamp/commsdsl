//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include <limits>

#include "commsdsl/parse/VariantField.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class VariantFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    VariantFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    VariantFieldImpl(const VariantFieldImpl& other);
    using Members = VariantField::Members;

    const FieldsList& members() const
    {
        return m_members;
    }

    Members membersList() const;

    std::size_t defaultMemberIdx() const
    {
        return m_state.m_defaultIdx;
    }

    bool displayIdxReadOnlyHidden() const
    {
        return m_state.m_idxHidden;
    }

protected:

    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
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

private:

    bool updateMembers();
    bool updateDefaultMember();
    bool updateIdxHidden();

    struct ReusableState
    {
        std::size_t m_defaultIdx = std::numeric_limits<std::size_t>::max();
        bool m_idxHidden = false;
    };

    ReusableState m_state;
    FieldsList m_members;
};

} // namespace parse

} // namespace commsdsl
