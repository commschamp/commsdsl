#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "FieldImpl.h"

namespace bbmp
{

class IntFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    enum Type
    {
        Type_int8,
        Type_uint8,
        Type_int16,
        Type_uint16,
        Type_int32,
        Type_uint32,
        Type_int64,
        Type_uint64,
        Type_intvar,
        Type_uintvar,
        Type_numOfValues
    };

    using ValidRange = std::pair<std::intmax_t, std::intmax_t>;
    using ValidRangesList = std::vector<ValidRange>;
    using ScalingRatio = std::pair<std::intmax_t, std::intmax_t>;
    using SpecialValue = std::pair<std::string, std::intmax_t>;
    using SpecialValuesList = std::vector<SpecialValue>;

    IntFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual std::size_t lengthImpl() const override;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateSerOffset();
    bool updateMinMaxValues();
    bool updateDefaultValue();
    bool updateScaling();
    bool updateValidRanges();
    bool updateSpecials();
    bool validateValidRangeStr(const std::string& str);
    bool validateValidValueStr(const std::string& str);
    bool validateValidMinValueStr(const std::string& str);
    bool validateValidMaxValueStr(const std::string& str);
    bool strToNumeric(const std::string& str, std::intmax_t& val);

    Type m_type = Type_numOfValues;
    Endian m_endian = Endian_NumOfValues;
    std::size_t m_length = 0U;
    std::intmax_t m_serOffset = 0;
    std::intmax_t m_typeAllowedMinValue = 0;
    std::intmax_t m_typeAllowedMaxValue = 0;
    std::intmax_t m_minValue = 0;
    std::intmax_t m_maxValue = 0;
    std::intmax_t m_defaultValue = 0;
    ScalingRatio m_scaling;
    ValidRangesList m_validRanges;
    SpecialValuesList m_specials;
};

} // namespace bbmp
