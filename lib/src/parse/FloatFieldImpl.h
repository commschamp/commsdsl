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

#include "commsdsl/parse/Endian.h"
#include "commsdsl/parse/FloatField.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class FloatFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = FloatField::Type;

    using ValidRangeInfo = FloatField::ValidRangeInfo;
    using ValidRangesList = FloatField::ValidRangesList;
    using SpecialValueInfo = FloatField::SpecialValueInfo;
    using SpecialValues = FloatField::SpecialValues;


    FloatFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    FloatFieldImpl(const FloatFieldImpl&);

    Type type() const
    {
        return m_state.m_type;
    }

    Endian endian() const
    {
        return m_state.m_endian;
    }

    double defaultValue() const
    {
        return m_state.m_defaultValue;
    }

    const ValidRangesList& validRanges() const
    {
        return m_state.m_validRanges;
    }

    const SpecialValues& specialValues() const
    {
        return m_state.m_specials;
    }

    bool validCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    Units units() const
    {
        return m_state.m_units;
    }

    unsigned displayDecimals() const
    {
        return m_state.m_displayDecimals;
    }

    bool hasNonUniqueSpecials() const;

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual bool isComparableToValueImpl(const std::string& val) const override;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override;
    virtual bool isValidRefTypeImpl(FieldRefType type) const override;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateMinMaxValues();
    bool updateDefaultValue();
    bool updateValidCheckVersion();
    bool updateValidRanges();
    bool updateNonUniqueSpecialsAllowed();
    bool updateSpecials();
    bool updateUnits();
    bool updateDisplayDecimals();
    bool updateDisplaySpecials();
    bool checkFullRangeAsAttr(const PropsMap& xmlAttrs);
    bool checkFullRangeAsChild(::xmlNodePtr child);
    bool checkFullRangeProps(const PropsMap& xmlAttrs);
    bool checkValidRangeAsAttr(const PropsMap& xmlAttrs);
    bool checkValidRangeAsChild(::xmlNodePtr child);
    bool checkValidRangeProps(const PropsMap& xmlAttrs);
    bool checkValidValueAsAttr(const PropsMap& xmlAttrs);
    bool checkValidValueAsChild(::xmlNodePtr child);
    bool checkValidValueProps(const PropsMap& xmlAttrs);
    bool checkValidMinAsAttr(const PropsMap& xmlAttrs);
    bool checkValidMinAsChild(::xmlNodePtr child);
    bool checkValidMinProps(const PropsMap& xmlAttrs);
    bool checkValidMaxAsAttr(const PropsMap& xmlAttrs);
    bool checkValidMaxAsChild(::xmlNodePtr child);
    bool checkValidMaxProps(const PropsMap& xmlAttrs);
    bool validateValidRangeStr(const std::string& str, double& min, double& max);
    bool validateValidValueStr(
            const std::string& str,
            const std::string& type,
            double& val,
            bool allowSpecials = true);
    bool strToValue(const std::string& str, double& val, bool allowSpecials = true) const;

    struct State
    {
        Type m_type = Type::NumOfValues;
        Endian m_endian = Endian_NumOfValues;
        std::size_t m_length = 0U;
        double m_typeAllowedMinValue = 0.0;
        double m_typeAllowedMaxValue = 0.0;
        double m_defaultValue = 0.0;
        ValidRangesList m_validRanges;
        SpecialValues m_specials;
        Units m_units = Units::Unknown;
        unsigned m_displayDecimals = 0U;
        bool m_validCheckVersion = false;
        bool m_nonUniqueSpecialsAllowed = false;
    };

    State m_state;
};

} // namespace parse

} // namespace commsdsl
