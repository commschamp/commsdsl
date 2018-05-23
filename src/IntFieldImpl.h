#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "commsdsl/IntField.h"
#include "commsdsl/Units.h"
#include "FieldImpl.h"

namespace commsdsl
{

class IntFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = IntField::Type;

    using ValidRangeInfo = IntField::ValidRangeInfo;
    using ValidRangesList = IntField::ValidRangesList;
    using ScalingRatio = IntField::ScalingRatio;
    using SpecialValueInfo = IntField::SpecialValueInfo;
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

    bool validCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    Units units() const
    {
        return m_state.m_units;
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

    static bool isTypeUnsigned(IntFieldImpl::Type t);

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual std::size_t bitLengthImpl() const override;
    virtual bool isComparableToValueImpl(const std::string& val) const override;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateBitLength();
    bool updateSerOffset();
    bool updateMinMaxValues();
    bool updateDefaultValue();
    bool updateScaling();
    bool updateValidCheckVersion();
    bool updateValidRanges();
    bool updateSpecials();
    bool updateUnits();
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
    bool validateValidRangeStr(const std::string& str, std::intmax_t& minVal, std::intmax_t& maxVal);
    bool validateValidValueStr(const std::string& str, const std::string& type, std::intmax_t& val);
    bool strToNumeric(const std::string& str, std::intmax_t& val) const;

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
        Units m_units = Units::Unknown;
        bool m_validCheckVersion = false;
    };

    State m_state;
};

} // namespace commsdsl
