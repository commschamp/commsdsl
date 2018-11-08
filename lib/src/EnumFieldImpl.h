#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "commsdsl/EnumField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class EnumFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = EnumField::Type;

    using ValueInfo = EnumField::ValueInfo;
    using Values = EnumField::Values;
    using RevValues = EnumField::RevValues;

    EnumFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    EnumFieldImpl(const EnumFieldImpl&);

    Type type() const
    {
        return m_state.m_type;
    }

    Endian endian() const
    {
        return m_state.m_endian;
    }

    std::intmax_t defaultValue() const
    {
        return m_state.m_defaultValue;
    }

    const Values& values() const
    {
        return m_state.m_values;
    }

    const RevValues& revValues() const
    {
        return m_state.m_revValues;
    }

    bool isNonUniqueAllowed() const
    {
        return m_state.m_nonUniqueAllowed;
    }

    bool isUnique() const;

    bool validCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    bool hexAssign() const
    {
        return m_state.m_hexAssign;
    }

protected:
    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl& other) override final;
    virtual bool parseImpl() override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;
    virtual std::size_t bitLengthImpl() const override final;
    virtual bool isComparableToValueImpl(const std::string& val) const override final;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override final;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateBitLength();
    bool updateNonUniqueAllowed();
    bool updateValidCheckVersion();
    bool updateMinMaxValues();
    bool updateValues();
    bool updateDefaultValue();
    bool updateHexAssign();
    bool strToNumeric(const std::string& str, std::intmax_t& val) const;

    struct State
    {
        Type m_type = Type::NumOfValues;
        Endian m_endian = Endian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        std::intmax_t m_typeAllowedMinValue = 0;
        std::intmax_t m_typeAllowedMaxValue = 0;
        std::intmax_t m_minValue = 0;
        std::intmax_t m_maxValue = 0;
        std::intmax_t m_defaultValue = 0;
        Values m_values;
        RevValues m_revValues;
        bool m_nonUniqueAllowed = false;
        bool m_validCheckVersion = false;
        bool m_hexAssign = false;
    };

    State m_state;
};

} // namespace commsdsl
