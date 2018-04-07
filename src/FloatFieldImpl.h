#pragma once

#include <utility>
#include <vector>
#include <map>

#include "bbmp/Endian.h"
#include "bbmp/FloatField.h"
#include "FieldImpl.h"

namespace bbmp
{

class FloatFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = FloatField::Type;

    using ValidRange = FloatField::ValidRange;
    using ValidRangesList = FloatField::ValidRangesList;
    using SpecialValueInfo = FloatField::SpecialValueInfo;
    using SpecialValues = FloatField::SpecialValues;


    FloatFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    FloatFieldImpl(const FloatFieldImpl&);

    Type type() const
    {
        return m_type;
    }

    Endian endian() const
    {
        return m_endian;
    }

    std::size_t length() const
    {
        return m_length;
    }

    double defaultValue() const
    {
        return m_defaultValue;
    }

    const ValidRangesList& validRanges() const
    {
        return m_validRanges;
    }

    const SpecialValues& specialValues() const
    {
        return m_specials;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual std::size_t lengthImpl() const override;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateMinMaxValues();
    bool updateDefaultValue();
    bool updateValidRanges();
    bool updateSpecials();
    bool validateValidRangeStr(const std::string& str);
    bool validateValidValueStr(const std::string& str);
    bool validateValidMinValueStr(const std::string& str);
    bool validateValidMaxValueStr(const std::string& str);

    Type m_type = Type::NumOfValues;
    Endian m_endian = Endian_NumOfValues;
    std::size_t m_length = 0U;
    double m_typeAllowedMinValue = 0.0;
    double m_typeAllowedMaxValue = 0.0;
    double m_defaultValue = 0.0;
    ValidRangesList m_validRanges;
    SpecialValues m_specials;
};

} // namespace bbmp
