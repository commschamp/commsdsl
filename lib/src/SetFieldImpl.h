//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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
#include "commsdsl/SetField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class SetFieldImpl : public FieldImpl
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

protected:
    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl& other) override final;
    virtual bool parseImpl() override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t bitLengthImpl() const override final;
    virtual bool isBitCheckableImpl(const std::string& val) const override final;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override final;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override final;
    virtual bool validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override final;

private:
    bool updateEndian();
    bool updateType();
    bool updateLength();
    bool updateNonUniqueAllowed();
    bool updateValidCheckVersion();
    bool updateDefaultValue();
    bool updateReservedValue();
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
    };
    State m_state;
};

} // namespace commsdsl
