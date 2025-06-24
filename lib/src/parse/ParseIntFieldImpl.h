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

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseIntField.h"
#include "commsdsl/parse/ParseUnits.h"
#include "ParseFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseIntFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using Type = ParseIntField::Type;

    using ValidRangeInfo = ParseIntField::ValidRangeInfo;
    using ValidRangesList = ParseIntField::ValidRangesList;
    using ScalingRatio = ParseIntField::ScalingRatio;
    using SpecialValueInfo = ParseIntField::SpecialValueInfo;
    using SpecialValues = ParseIntField::SpecialValues;

    ParseIntFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseIntFieldImpl(const ParseIntFieldImpl&) = default;

    Type parseType() const
    {
        return m_state.m_type;
    }

    ParseEndian parseEndian() const
    {
        return m_state.m_endian;
    }

    std::intmax_t parseSerOffset() const
    {
        return m_state.m_serOffset;
    }

    std::intmax_t parseMinValue() const
    {
        return m_state.m_minValue;
    }

    std::intmax_t parseMaxValue() const
    {
        return m_state.m_maxValue;
    }

    std::intmax_t parseDefaultValue() const
    {
        return m_state.m_defaultValue;
    }

    ScalingRatio parseScaling() const
    {
        return m_state.m_scaling;
    }

    const ValidRangesList& parseValidRanges() const
    {
        return m_state.m_validRanges;
    }

    const SpecialValues& parseSpecialValues() const
    {
        return m_state.m_specials;
    }

    bool parseValidCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    ParseUnits parseUnits() const
    {
        return m_state.m_units;
    }

    unsigned parseDisplayDecimals() const
    {
        return m_state.m_displayDecimals;
    }

    std::intmax_t parseDisplayOffset() const
    {
        return m_state.m_displayOffset;
    }

    bool parseSignExt() const
    {
        return m_state.m_signExt;
    }

    bool parseAvailableLengthLimit() const
    {
        return m_state.m_availableLengthLimit;
    }

    static Type parseTypeValue(const std::string& value);

    static std::size_t parseMaxTypeLength(Type t);
    static std::intmax_t parseMinTypeValue(Type t);
    static std::intmax_t parseMaxTypeValue(Type t);
    static std::intmax_t parseCalcMinValue(Type t, std::size_t bitsLen);
    static std::intmax_t parseCalcMaxValue(Type t, std::size_t bitsLen);
    static bool parseIsUnsigned(Type t);
    static bool parseIsBigUnsigned(Type t)
    {
        return (t == Type::Uint64) || (t == Type::Uintvar);
    }

    static bool parseIsTypeUnsigned(ParseIntFieldImpl::Type t);

protected:
    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseMaxLengthImpl() const override;
    virtual std::size_t parseBitLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseIsComparableToFieldImpl(const ParseFieldImpl& field) const override;
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override;
    virtual bool parseVerifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const override;
    virtual FieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const override;
    virtual bool parseIsValidRefTypeImpl(FieldRefType type) const override;

private:
    bool parseUpdateType();
    bool parseUpdateEndian();
    bool parseUpdateLength();
    bool parseUpdateBitLength();
    bool parseUpdateSerOffset();
    bool parseUpdateMinMaxValues();
    bool parseUpdateDefaultValue();
    bool parseUpdateDefaultValidValue();
    bool parseUpdateScaling();
    bool parseUpdateValidCheckVersion();
    bool parseUpdateValidRanges();
    bool parseUpdateNonUniqueSpecialsAllowed();
    bool parseUpdateSpecials();
    bool parseUpdateUnits();
    bool parseUpdateDisplayDecimals();
    bool parseUpdateDisplayOffset();
    bool parseUpdateSignExt();
    bool parseUpdateDisplaySpecials();
    bool parseUpdateAvailableLengthLimit();
    bool parseCheckValidRangeAsAttr(const PropsMap& xmlAttrs);
    bool parseCheckValidRangeAsChild(::xmlNodePtr child);
    bool parseCheckValidRangeProps(const PropsMap& xmlAttrs);
    bool parseCheckValidValueAsAttr(const PropsMap& xmlAttrs);
    bool parseCheckValidValueAsChild(::xmlNodePtr child);
    bool parseCheckValidValueProps(const PropsMap& xmlAttrs);
    bool parseCheckValidMinAsAttr(const PropsMap& xmlAttrs);
    bool parseCheckValidMinAsChild(::xmlNodePtr child);
    bool parseCheckValidMinProps(const PropsMap& xmlAttrs);
    bool parseCheckValidMaxAsAttr(const PropsMap& xmlAttrs);
    bool parseCheckValidMaxAsChild(::xmlNodePtr child);
    bool parseCheckValidMaxProps(const PropsMap& xmlAttrs);
    bool parseValidateValidRangeStr(const std::string& str, std::intmax_t& minVal, std::intmax_t& maxVal);
    bool parseValidateValidValueStr(const std::string& str, const std::string& type, std::intmax_t& val);
    bool parseStrToValue(const std::string& str, std::intmax_t& val) const;
    bool parseUpdateDefaultValueInternal(const std::string& valueStr);

    struct State
    {
        Type m_type = Type::NumOfValues;
        ParseEndian m_endian = ParseEndian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        std::intmax_t m_serOffset = 0;
        std::intmax_t m_typeAllowedMinValue = 0;
        std::intmax_t m_typeAllowedMaxValue = 0;
        std::intmax_t m_minValue = 0;
        std::intmax_t m_maxValue = 0;
        std::intmax_t m_defaultValue = 0;
        ScalingRatio m_scaling;
        ValidRangesList m_validRanges;
        SpecialValues m_specials;
        ParseUnits m_units = ParseUnits::Unknown;
        unsigned m_displayDecimals = 0U;
        std::intmax_t m_displayOffset = 0U;
        bool m_validCheckVersion = false;
        bool m_signExt = true;
        bool m_nonUniqueSpecialsAllowed = false;
        bool m_availableLengthLimit = false;
    };

    State m_state;
};

} // namespace parse

} // namespace commsdsl
