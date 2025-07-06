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

#include "commsdsl/parse/ParseEndian.h"
#include "ParseFieldImpl.h"

#include <cstdint>
#include <utility>
#include <vector>


namespace commsdsl
{

namespace parse
{

class ParseRefFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    ParseRefFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseRefFieldImpl(const ParseRefFieldImpl& other);

    ParseField parseField() const
    {
        return ParseField(m_field);
    }

    const ParseFieldImpl* parseFieldImpl() const
    {
        return m_field;
    }

protected:
    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPropsNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseMaxLengthImpl() const override;
    virtual std::size_t parseBitLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseIsComparableToFieldImpl(const ParseFieldImpl& field) const override;
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const override;
    virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual bool parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override;
    virtual bool parseVerifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const override;
    virtual FieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const override;
    virtual bool parseIsValidRefTypeImpl(FieldRefType type) const override;

private:
    using StrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;

    bool parseUpdateBitLength();

    bool parseStrToValue(
        const std::string& ref,
        StrToValueFieldConvertFunc&& forwardFunc) const;

    struct State
    {
        std::size_t m_bitLength = 0U;
    };    

    State m_state;
    const ParseFieldImpl* m_field = nullptr;
};

} // namespace parse

} // namespace commsdsl
