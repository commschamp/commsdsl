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

#include <utility>
#include <vector>
#include <map>

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseFloatField.h"
#include "ParseFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseFloatFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using Type = ParseFloatField::Type;

    using ValidRangeInfo = ParseFloatField::ValidRangeInfo;
    using ValidRangesList = ParseFloatField::ValidRangesList;
    using SpecialValueInfo = ParseFloatField::SpecialValueInfo;
    using SpecialValues = ParseFloatField::SpecialValues;


    ParseFloatFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseFloatFieldImpl(const ParseFloatFieldImpl&);

    Type parseType() const
    {
        return m_state.m_type;
    }

    ParseEndian parseEndian() const
    {
        return m_state.m_endian;
    }

    double parseDefaultValue() const
    {
        return m_state.m_defaultValue;
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

    bool parseHasNonUniqueSpecials() const;

protected:
    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const override;
    virtual FieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const override;
    virtual bool parseIsValidRefTypeImpl(FieldRefType type) const override;

private:
    bool parseUpdateType();
    bool parseUpdateEndian();
    bool parseUpdateLength();
    bool parseUpdateMinMaxValues();
    bool parseUpdateDefaultValue();
    bool parseUpdateValidCheckVersion();
    bool parseUpdateValidRanges();
    bool parseUpdateNonUniqueSpecialsAllowed();
    bool parseUpdateSpecials();
    bool parseUpdateUnits();
    bool parseUpdateDisplayDecimals();
    bool parseUpdateDisplaySpecials();
    bool parseCheckFullRangeAsAttr(const PropsMap& xmlAttrs);
    bool parseCheckFullRangeAsChild(::xmlNodePtr child);
    bool parseCheckFullRangeProps(const PropsMap& xmlAttrs);
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
    bool parseValidateValidRangeStr(const std::string& str, double& min, double& max);
    bool parseValidateValidValueStr(
            const std::string& str,
            const std::string& type,
            double& val,
            bool allowSpecials = true);
    bool parseStrToValue(const std::string& str, double& val, bool allowSpecials = true) const;

    struct State
    {
        Type m_type = Type::NumOfValues;
        ParseEndian m_endian = ParseEndian_NumOfValues;
        std::size_t m_length = 0U;
        double m_typeAllowedMinValue = 0.0;
        double m_typeAllowedMaxValue = 0.0;
        double m_defaultValue = 0.0;
        ValidRangesList m_validRanges;
        SpecialValues m_specials;
        ParseUnits m_units = ParseUnits::Unknown;
        unsigned m_displayDecimals = 0U;
        bool m_validCheckVersion = false;
        bool m_nonUniqueSpecialsAllowed = false;
    };

    State m_state;
};

} // namespace parse

} // namespace commsdsl
