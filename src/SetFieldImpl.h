#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "bbmp/Endian.h"
#include "bbmp/SetField.h"
#include "FieldImpl.h"

namespace bbmp
{

class SetFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = SetField::Type;
    using BitInfo = SetField::BitInfo;
    using Bits = SetField::Bits;
    using RevBits = SetField::RevBits;

    SetFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    SetFieldImpl(const SetFieldImpl&);

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

    bool defaultBitValue() const
    {
        return m_state.m_defaultBitValue;
    }

    bool reservedBitValue() const
    {
        return m_state.m_reservedBitValue;
    }

    const Bits& bits() const
    {
        return m_state.m_bits;
    }

    const RevBits& revBits() const
    {
        return m_state.m_revBits;
    }

    bool isNonUniqueAllowed() const
    {
        return m_state.m_nonUniqueAllowed;
    }

    bool isUnique() const;

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual std::size_t lengthImpl() const override;
    virtual std::size_t bitLengthImpl() const override;

private:
    bool updateEndian();
    bool updateLength();
    bool updateNonUniqueAllowed();
    bool updateDefaultValue();
    bool updateReservedValue();
    bool updateBits();

    struct State
    {
        Type m_type = Type::NumOfValues;
        Endian m_endian = Endian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        Bits m_bits;
        RevBits m_revBits;
        bool m_nonUniqueAllowed = false;
        bool m_defaultBitValue = false;
        bool m_reservedBitValue = false;
    };
    State m_state;
};

} // namespace bbmp
