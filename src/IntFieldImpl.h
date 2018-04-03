#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "bbmp/Endian.h"
#include "bbmp/IntField.h"
#include "FieldImpl.h"

namespace bbmp
{

class IntFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = IntField::Type;

    using ValidRange = IntField::ValidRange;
    using ValidRangesList = IntField::ValidRangesList;
    using ScalingRatio = IntField::ScalingRatio;
    using SpecialValues = IntField::SpecialValues;

    IntFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    IntFieldImpl(const IntFieldImpl&) = default;

    Type type() const
    {
        return m_state.m_type;
    }

    Endian endian() const
    {
        return m_state.m_endian;
    }

    std::size_t length() const
    {
        return m_state.m_length;
    }

    std::size_t bitLength() const
    {
        return m_state.m_bitLength;
    }

    std::intmax_t serOffset() const
    {
        return m_state.m_serOffset;
    }

    std::intmax_t minValue() const
    {
        return m_state.m_minValue;
    }

    std::intmax_t maxValue() const
    {
        return m_state.m_maxValue;
    }

    std::intmax_t defaultValue() const
    {
        return m_state.m_defaultValue;
    }

    ScalingRatio scaling() const
    {
        return m_state.m_scaling;
    }

    const ValidRangesList& validRanges() const
    {
        return m_state.m_validRanges;
    }

    const SpecialValues& specialValues() const
    {
        return m_state.m_specials;
    }

    static Type parseTypeValue(const std::string& value);

    static std::size_t maxTypeLength(Type t);
    static std::intmax_t minTypeValue(Type t);
    static std::intmax_t maxTypeValue(Type t);
    static std::intmax_t calcMinValue(Type t, std::size_t bitsLen);
    static std::intmax_t calcMaxValue(Type t, std::size_t bitsLen);
    static bool isBigUnsigned(Type t)
    {
        return (t == Type::Uint64) || (t == Type::Uintvar);
    }


protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t lengthImpl() const override;
    virtual std::size_t bitLengthImpl() const override;

private:
    struct State
    {
        Type m_type = Type::NumOfValues;
        Endian m_endian = Endian_NumOfValues;
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
    };

    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateBitLength();
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

    State m_state;
};

} // namespace bbmp
