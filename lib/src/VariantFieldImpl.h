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

#include <limits>

#include "commsdsl/VariantField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class VariantFieldImpl : public FieldImpl
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

    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl &other) override final;
    virtual bool parseImpl() override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;

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

} // namespace commsdsl
