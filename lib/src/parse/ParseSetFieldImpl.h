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
#include "commsdsl/parse/ParseSetField.h"
#include "ParseFieldImpl.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseSetFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using ParseType = ParseSetField::ParseType;
    using ParseBitInfo = ParseSetField::ParseBitInfo;
    using ParseBits = ParseSetField::ParseBits;
    using ParseRevBits = ParseSetField::ParseRevBits;

    ParseSetFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseSetFieldImpl(const ParseSetFieldImpl&);

    ParseType parseType() const
    {
        return m_state.m_type;
    }

    ParseEndian parseEndian() const
    {
        return m_state.m_endian;
    }

    bool parseDefaultBitValue() const
    {
        return m_state.m_defaultBitValue;
    }

    bool parseReservedBitValue() const
    {
        return m_state.m_reservedBitValue;
    }

    const ParseBits& parseBits() const
    {
        return m_state.m_bits;
    }

    const ParseRevBits& parseRevBits() const
    {
        return m_state.m_revBits;
    }

    bool parseIsNonUniqueAllowed() const
    {
        return m_state.m_nonUniqueAllowed;
    }

    bool parseIsUnique() const;

    bool parseValidCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    bool parseAvailableLengthLimit() const
    {
        return m_state.m_availableLengthLimit;
    }

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual ParsePtr parseCloneImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseBitLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseIsComparableToFieldImpl(const ParseFieldImpl& field) const override;    
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override;
    virtual ParseFieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const override;
    virtual bool parseIsValidRefTypeImpl(ParseFieldRefType type) const override;

private:
    bool parseUpdateEndian();
    bool parseUpdateType();
    bool parseUpdateLength();
    bool parseUpdateNonUniqueAllowed();
    bool parseUpdateValidCheckVersion();
    bool parseUpdateDefaultValue();
    bool parseUpdateReservedValue();
    bool parseUpdateAvailableLengthLimit();
    bool parseUpdateBits();
    bool parseStrToValue(const std::string& str, bool& val) const;

    struct ParseState
    {
        ParseType m_type = ParseType::NumOfValues;
        ParseEndian m_endian = ParseEndian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        ParseBits m_bits;
        ParseRevBits m_revBits;
        bool m_nonUniqueAllowed = false;
        bool m_defaultBitValue = false;
        bool m_reservedBitValue = false;
        bool m_validCheckVersion = false;
        bool m_availableLengthLimit = false;
    };
    ParseState m_state;
};

} // namespace parse

} // namespace commsdsl
