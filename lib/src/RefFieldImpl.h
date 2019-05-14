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

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "FieldImpl.h"

namespace commsdsl
{

class RefFieldImpl : public FieldImpl
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
    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl& other) override final;
    virtual bool parseImpl() override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;
    virtual std::size_t bitLengthImpl() const override final;
    virtual bool isComparableToValueImpl(const std::string& val) const override final;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override final;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override final;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override final;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override final;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const override final;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override final;
    virtual bool validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override final;

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

} // namespace commsdsl
