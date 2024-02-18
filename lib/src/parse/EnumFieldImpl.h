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
#include "commsdsl/parse/EnumField.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class EnumFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = EnumField::Type;

    using ValueInfo = EnumField::ValueInfo;
    using Values = EnumField::Values;
    using RevValues = EnumField::RevValues;

    EnumFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    EnumFieldImpl(const EnumFieldImpl&);

    Type type() const
    {
        return m_state.m_type;
    }

    Endian endian() const
    {
        return m_state.m_endian;
    }

    std::intmax_t defaultValue() const
    {
        return m_state.m_defaultValue;
    }

    const Values& values() const
    {
        return m_state.m_values;
    }

    const RevValues& revValues() const
    {
        return m_state.m_revValues;
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

    bool hexAssign() const
    {
        return m_state.m_hexAssign;
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
    virtual std::size_t maxLengthImpl() const override;
    virtual std::size_t bitLengthImpl() const override;
    virtual bool isComparableToValueImpl(const std::string& val) const override;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override;
    virtual bool verifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override;
    virtual bool isValidRefTypeImpl(FieldRefType type) const override;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateBitLength();
    bool updateNonUniqueAllowed();
    bool updateValidCheckVersion();
    bool updateMinMaxValues();
    bool updateValues();
    bool updateDefaultValue();
    bool updateHexAssign();
    bool updateAvailableLengthLimit();
    bool strToValue(const std::string& str, std::intmax_t& val) const;

    struct State
    {
        Type m_type = Type::NumOfValues;
        Endian m_endian = Endian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        std::intmax_t m_typeAllowedMinValue = 0;
        std::intmax_t m_typeAllowedMaxValue = 0;
        std::intmax_t m_minValue = 0;
        std::intmax_t m_maxValue = 0;
        std::intmax_t m_defaultValue = 0;
        Values m_values;
        RevValues m_revValues;
        bool m_nonUniqueAllowed = false;
        bool m_validCheckVersion = false;
        bool m_hexAssign = false;
        bool m_availableLengthLimit = false;
    };

    State m_state;
};

} // namespace parse

} // namespace commsdsl
