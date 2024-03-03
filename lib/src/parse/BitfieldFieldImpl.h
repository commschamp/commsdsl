//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/Endian.h"
#include "commsdsl/parse/BitfieldField.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class BitfieldFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    BitfieldFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    BitfieldFieldImpl(const BitfieldFieldImpl& other);
    using Members = BitfieldField::Members;

    Endian endian() const
    {
        return m_endian;
    }

    Members membersList() const;

    static const XmlWrap::NamesList& supportedTypes();

protected:

    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl &other) override;
    virtual bool parseImpl() override;
    virtual bool replaceMembersImpl(FieldsList& members) override;
    virtual std::size_t minLengthImpl() const override;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool verifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const override;
    virtual bool verifyAliasedMemberImpl(const std::string& fieldName) const override;
    virtual const XmlWrap::NamesList& supportedMemberTypesImpl() const override;
    virtual const FieldsList& membersImpl() const override;

private:
    bool updateEndian();
    bool updateMembers();

    Endian m_endian = Endian_NumOfValues;
    FieldsList m_members;
};

} // namespace parse

} // namespace commsdsl
