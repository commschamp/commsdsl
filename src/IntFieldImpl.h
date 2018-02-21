#pragma once

#include <cstdint>

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

    IntFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual bool parseImpl() override;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateDefaultValue();

    Type m_type = Type_numOfValues;
    Endian m_endian = Endian_NumOfValues;
    std::uintmax_t m_defaultValue = 0;
    std::size_t m_length = 0;
};

} // namespace bbmp
