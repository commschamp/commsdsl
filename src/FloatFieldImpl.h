#pragma once

#include <utility>
#include <vector>

#include "bbmp/Endian.h"
#include "FieldImpl.h"

namespace bbmp
{

class FloatFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    enum Type
    {
        Type_float,
        Type_double,
        Type_numOfValues
    };

    using ValidRange = std::pair<double, double>;
    using ValidRangesList = std::vector<ValidRange>;
    using SpecialValue = std::pair<std::string, double>;
    using SpecialValuesList = std::vector<SpecialValue>;


    FloatFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
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

    Type m_type = Type_numOfValues;
    Endian m_endian = Endian_NumOfValues;
    std::size_t m_length = 0U;
    double m_typeAllowedMinValue = 0.0;
    double m_typeAllowedMaxValue = 0.0;
    double m_defaultValue = 0.0;
    ValidRangesList m_validRanges;
    SpecialValuesList m_specials;
};

} // namespace bbmp
