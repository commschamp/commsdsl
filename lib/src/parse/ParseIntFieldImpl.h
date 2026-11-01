//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

    [[nodiscard]] ParseType parseType() const
    {
        return m_state.m_type;
    }

    [[nodiscard]] ParseEndian parseEndian() const
    {
        return m_state.m_endian;
    }

    [[nodiscard]] std::intmax_t parseSerOffset() const
    {
        return m_state.m_serOffset;
    }

    [[nodiscard]] std::intmax_t parseMinValue() const
    {
        return m_state.m_minValue;
    }

    [[nodiscard]] std::intmax_t parseMaxValue() const
    {
        return m_state.m_maxValue;
    }

    [[nodiscard]] std::intmax_t parseDefaultValue() const
    {
        return m_state.m_defaultValue;
    }

    [[nodiscard]] ParseScalingRatio parseScaling() const
    {
        return m_state.m_scaling;
    }

    [[nodiscard]] const ParseValidRangesList& parseValidRanges() const
    {
        return m_state.m_validRanges;
    }

    [[nodiscard]] const ParseSpecialValues& parseSpecialValues() const
    {
        return m_state.m_specials;
    }

    [[nodiscard]] bool parseValidCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    [[nodiscard]] ParseUnits parseUnits() const
    {
        return m_state.m_units;
    }

    [[nodiscard]] unsigned parseDisplayDecimals() const
    {
        return m_state.m_displayDecimals;
    }

    [[nodiscard]] std::intmax_t parseDisplayOffset() const
    {
        return m_state.m_displayOffset;
    }

    [[nodiscard]] bool parseSignExt() const
    {
        return m_state.m_signExt;
    }

    [[nodiscard]] bool parseAvailableLengthLimit() const
    {
        return m_state.m_availableLengthLimit;
    }

    [[nodiscard]] static ParseType parseTypeValue(const std::string& value);

    [[nodiscard]] static std::size_t parseMaxTypeLength(ParseType t);
    [[nodiscard]] static std::intmax_t parseMinTypeValue(ParseType t);
    [[nodiscard]] static std::intmax_t parseMaxTypeValue(ParseType t);
    [[nodiscard]] static std::intmax_t parseCalcMinValue(ParseType t, std::size_t bitsLen);
    [[nodiscard]] static std::intmax_t parseCalcMaxValue(ParseType t, std::size_t bitsLen);
    [[nodiscard]] static bool parseIsUnsigned(ParseType t);
    [[nodiscard]] static bool parseIsBigUnsigned(ParseType t)
    {
        return (t == ParseType::Uint64) || (t == ParseType::Uintvar);
    }

    [[nodiscard]] static bool parseIsTypeUnsigned(ParseIntFieldImpl::ParseType t);

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
    [[nodiscard]] bool parseUpdateType();
    [[nodiscard]] bool parseUpdateEndian();
    [[nodiscard]] bool parseUpdateLength();
    [[nodiscard]] bool parseUpdateBitLength();
    [[nodiscard]] bool parseUpdateSerOffset();
    [[nodiscard]] bool parseUpdateMinMaxValues();
    [[nodiscard]] bool parseUpdateDefaultValue();
    [[nodiscard]] bool parseUpdateDefaultValidValue();
    [[nodiscard]] bool parseUpdateScaling();
    [[nodiscard]] bool parseUpdateValidCheckVersion();
    [[nodiscard]] bool parseUpdateValidRanges();
    [[nodiscard]] bool parseUpdateNonUniqueSpecialsAllowed();
    [[nodiscard]] bool parseUpdateSpecials();
    [[nodiscard]] bool parseUpdateUnits();
    [[nodiscard]] bool parseUpdateDisplayDecimals();
    [[nodiscard]] bool parseUpdateDisplayOffset();
    [[nodiscard]] bool parseUpdateSignExt();
    [[nodiscard]] bool parseUpdateDisplaySpecials();
    [[nodiscard]] bool parseUpdateAvailableLengthLimit();
    [[nodiscard]] bool parseCheckValidRangeAsAttr(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidRangeAsChild(::xmlNodePtr child);
    [[nodiscard]] bool parseCheckValidRangeProps(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidValueAsAttr(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidValueAsChild(::xmlNodePtr child);
    [[nodiscard]] bool parseCheckValidValueProps(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidMinAsAttr(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidMinAsChild(::xmlNodePtr child);
    [[nodiscard]] bool parseCheckValidMinProps(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidMaxAsAttr(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidMaxAsChild(::xmlNodePtr child);
    [[nodiscard]] bool parseCheckValidMaxProps(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseValidateValidRangeStr(const std::string& str, std::intmax_t& minVal, std::intmax_t& maxVal);
    [[nodiscard]] bool parseValidateValidValueStr(const std::string& str, const std::string& type, std::intmax_t& val);
    [[nodiscard]] bool parseStrToValue(const std::string& str, std::intmax_t& val) const;
    [[nodiscard]] bool parseUpdateDefaultValueInternal(const std::string& valueStr);

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
