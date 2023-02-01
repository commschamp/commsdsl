//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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
#include "commsdsl/parse/SetField.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class SetFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = SetField::Type;
    using BitInfo = SetField::BitInfo;
    using Bits = SetField::Bits;
    using RevBits = SetField::RevBits;

    SetFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    SetFieldImpl(const SetFieldImpl&);

    Type type() const
    {
        return m_state.m_type;
    }

    Endian endian() const
    {
        return m_state.m_endian;
    }

    bool defaultBitValue() const
    {
        return m_state.m_defaultBitValue;
    }

    bool reservedBitValue() const
    {
        return m_state.m_reservedBitValue;
    }

    const Bits& bits() const
    {
        return m_state.m_bits;
    }

    const RevBits& revBits() const
    {
        return m_state.m_revBits;
    }

    bool isNonUniqueAllowed() const
    {
        return m_state.m_nonUniqueAllowed;
    }

    bool isUnique() const;

    bool validCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    bool availableLengthLimit() const
    {
        return m_state.m_availableLengthLimit;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t bitLengthImpl() const override;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override;

private:
    bool updateEndian();
    bool updateType();
    bool updateLength();
    bool updateNonUniqueAllowed();
    bool updateValidCheckVersion();
    bool updateDefaultValue();
    bool updateReservedValue();
    bool updateAvailableLengthLimit();
    bool updateBits();
    bool strToValue(const std::string& str, bool& val) const;

    struct State
    {
        Type m_type = Type::NumOfValues;
        Endian m_endian = Endian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        Bits m_bits;
        RevBits m_revBits;
        bool m_nonUniqueAllowed = false;
        bool m_defaultBitValue = false;
        bool m_reservedBitValue = false;
        bool m_validCheckVersion = false;
        bool m_availableLengthLimit = false;
    };
    State m_state;
};

} // namespace parse

} // namespace commsdsl
