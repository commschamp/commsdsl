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
#include "commsdsl/parse/ParseIntField.h"
#include "commsdsl/parse/ParseUnits.h"
#include "ParseFieldImpl.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseIntFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using ParseType = ParseIntField::ParseType;

    using ParseValidRangeInfo = ParseIntField::ParseValidRangeInfo;
    using ParseValidRangesList = ParseIntField::ParseValidRangesList;
    using ParseScalingRatio = ParseIntField::ParseScalingRatio;
    using ParseSpecialValueInfo = ParseIntField::ParseSpecialValueInfo;
    using ParseSpecialValues = ParseIntField::ParseSpecialValues;

    ParseIntFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseIntFieldImpl(const ParseIntFieldImpl&) = default;

    ParseType parseType() const
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

    ParseScalingRatio parseScaling() const
    {
        return m_state.m_scaling;
    }

    const ParseValidRangesList& parseValidRanges() const
    {
        return m_state.m_validRanges;
    }

    const ParseSpecialValues& parseSpecialValues() const
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

    static ParseType parseTypeValue(const std::string& value);

    static std::size_t parseMaxTypeLength(ParseType t);
    static std::intmax_t parseMinTypeValue(ParseType t);
    static std::intmax_t parseMaxTypeValue(ParseType t);
    static std::intmax_t parseCalcMinValue(ParseType t, std::size_t bitsLen);
    static std::intmax_t parseCalcMaxValue(ParseType t, std::size_t bitsLen);
    static bool parseIsUnsigned(ParseType t);
    static bool parseIsBigUnsigned(ParseType t)
    {
        return (t == ParseType::Uint64) || (t == ParseType::Uintvar);
    }

    static bool parseIsTypeUnsigned(ParseIntFieldImpl::ParseType t);

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual ParsePtr parseCloneImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseMaxLengthImpl() const override;
    virtual std::size_t parseBitLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseIsComparableToFieldImpl(const ParseFieldImpl& field) const override;
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const override;
    virtual bool parseVerifySemanticTypeImpl(::xmlNodePtr node, ParseSemanticType type) const override;
    virtual ParseFieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const override;
    virtual bool parseIsValidRefTypeImpl(ParseFieldRefType type) const override;

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
    bool parseCheckValidRangeAsAttr(const ParsePropsMap& xmlAttrs);
    bool parseCheckValidRangeAsChild(::xmlNodePtr child);
    bool parseCheckValidRangeProps(const ParsePropsMap& xmlAttrs);
    bool parseCheckValidValueAsAttr(const ParsePropsMap& xmlAttrs);
    bool parseCheckValidValueAsChild(::xmlNodePtr child);
    bool parseCheckValidValueProps(const ParsePropsMap& xmlAttrs);
    bool parseCheckValidMinAsAttr(const ParsePropsMap& xmlAttrs);
    bool parseCheckValidMinAsChild(::xmlNodePtr child);
    bool parseCheckValidMinProps(const ParsePropsMap& xmlAttrs);
    bool parseCheckValidMaxAsAttr(const ParsePropsMap& xmlAttrs);
    bool parseCheckValidMaxAsChild(::xmlNodePtr child);
    bool parseCheckValidMaxProps(const ParsePropsMap& xmlAttrs);
    bool parseValidateValidRangeStr(const std::string& str, std::intmax_t& minVal, std::intmax_t& maxVal);
    bool parseValidateValidValueStr(const std::string& str, const std::string& type, std::intmax_t& val);
    bool parseStrToValue(const std::string& str, std::intmax_t& val) const;
    bool parseUpdateDefaultValueInternal(const std::string& valueStr);

    struct ParseState
    {
        ParseType m_type = ParseType::NumOfValues;
        ParseEndian m_endian = ParseEndian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        std::intmax_t m_serOffset = 0;
        std::intmax_t m_typeAllowedMinValue = 0;
        std::intmax_t m_typeAllowedMaxValue = 0;
        std::intmax_t m_minValue = 0;
        std::intmax_t m_maxValue = 0;
        std::intmax_t m_defaultValue = 0;
        ParseScalingRatio m_scaling;
        ParseValidRangesList m_validRanges;
        ParseSpecialValues m_specials;
        ParseUnits m_units = ParseUnits::Unknown;
        unsigned m_displayDecimals = 0U;
        std::intmax_t m_displayOffset = 0U;
        bool m_validCheckVersion = false;
        bool m_signExt = true;
        bool m_nonUniqueSpecialsAllowed = false;
        bool m_availableLengthLimit = false;
    };

    ParseState m_state;
};

} // namespace parse

} // namespace commsdsl
