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
#include "commsdsl/parse/ParseFloatField.h"
#include "ParseFieldImpl.h"

#include <map>
#include <utility>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseFloatFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using ParseType = ParseFloatField::ParseType;

    using ParseValidRangeInfo = ParseFloatField::ParseValidRangeInfo;
    using ParseValidRangesList = ParseFloatField::ParseValidRangesList;
    using ParseSpecialValueInfo = ParseFloatField::ParseSpecialValueInfo;
    using ParseSpecialValues = ParseFloatField::ParseSpecialValues;

    ParseFloatFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseFloatFieldImpl(const ParseFloatFieldImpl&);

    [[nodiscard]] ParseType parseType() const
    {
        return m_state.m_type;
    }

    [[nodiscard]] ParseEndian parseEndian() const
    {
        return m_state.m_endian;
    }

    [[nodiscard]] double parseDefaultValue() const
    {
        return m_state.m_defaultValue;
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

    [[nodiscard]] bool parseHasNonUniqueSpecials() const;

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual ParsePtr parseCloneImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const override;
    virtual ParseFieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const override;
    virtual bool parseIsValidRefTypeImpl(ParseFieldRefType type) const override;

private:
    [[nodiscard]] bool parseUpdateType();
    [[nodiscard]] bool parseUpdateEndian();
    [[nodiscard]] bool parseUpdateLength();
    [[nodiscard]] bool parseUpdateMinMaxValues();
    [[nodiscard]] bool parseUpdateDefaultValue();
    [[nodiscard]] bool parseUpdateValidCheckVersion();
    [[nodiscard]] bool parseUpdateValidRanges();
    [[nodiscard]] bool parseUpdateNonUniqueSpecialsAllowed();
    [[nodiscard]] bool parseUpdateSpecials();
    [[nodiscard]] bool parseUpdateUnits();
    [[nodiscard]] bool parseUpdateDisplayDecimals();
    [[nodiscard]] bool parseUpdateDisplaySpecials();
    [[nodiscard]] bool parseCheckFullRangeAsAttr(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckFullRangeAsChild(::xmlNodePtr child);
    [[nodiscard]] bool parseCheckFullRangeProps(const ParsePropsMap& xmlAttrs);
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
    [[nodiscard]] bool parseValidateValidRangeStr(const std::string& str, double& min, double& max);
    [[nodiscard]] bool parseValidateValidValueStr(
            const std::string& str,
            const std::string& type,
            double& val,
            bool allowSpecials = true);
    [[nodiscard]] bool parseStrToValue(const std::string& str, double& val, bool allowSpecials = true) const;

    struct ParseState
    {
        ParseType m_type = ParseType::NumOfValues;
        ParseEndian m_endian = ParseEndian_NumOfValues;
        std::size_t m_length = 0U;
        double m_typeAllowedMinValue = 0.0;
        double m_typeAllowedMaxValue = 0.0;
        double m_defaultValue = 0.0;
        ParseValidRangesList m_validRanges;
        ParseSpecialValues m_specials;
        ParseUnits m_units = ParseUnits::Unknown;
        unsigned m_displayDecimals = 0U;
        bool m_validCheckVersion = false;
        bool m_nonUniqueSpecialsAllowed = false;
    };

    ParseState m_state;
};

} // namespace parse

} // namespace commsdsl
