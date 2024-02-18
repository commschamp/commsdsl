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

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/parse/Endian.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class RefFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    RefFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    RefFieldImpl(const RefFieldImpl& other);

    Field field() const
    {
        return Field(m_field);
    }

    const FieldImpl* fieldImpl() const
    {
        return m_field;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual std::size_t bitLengthImpl() const override;
    virtual bool isComparableToValueImpl(const std::string& val) const override;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual bool validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override;
    virtual bool verifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override;
    virtual bool isValidRefTypeImpl(FieldRefType type) const override;

private:
    using StrToValueFieldConvertFunc = std::function<bool (const FieldImpl& f, const std::string& ref)>;

    bool updateBitLength();

    bool strToValue(
        const std::string& ref,
        StrToValueFieldConvertFunc&& forwardFunc) const;

    struct State
    {
        std::size_t m_bitLength = 0U;
    };    

    State m_state;
    const FieldImpl* m_field = nullptr;
};

} // namespace parse

} // namespace commsdsl
