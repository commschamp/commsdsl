#include "bbmp/IntField.h"

#include <cassert>

#include "IntFieldImpl.h"

namespace bbmp
{

namespace
{

const IntFieldImpl* cast(const FieldImpl* ptr)
{
    return static_cast<const IntFieldImpl*>(ptr);
}

} // namespace

IntField::IntField(const IntFieldImpl* impl)
  : Base(impl)
{
}

IntField::IntField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Int);
}

IntField::Type IntField::type() const
{
    return cast(m_pImpl)->type();
}

Endian IntField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::size_t IntField::length() const
{
    return cast(m_pImpl)->length();
}

std::size_t IntField::bitLength() const
{
    return cast(m_pImpl)->bitLength();
}

std::intmax_t IntField::serOffset() const
{
    return cast(m_pImpl)->serOffset();
}

std::intmax_t IntField::minValue() const
{
    return cast(m_pImpl)->minValue();
}

std::intmax_t IntField::maxValue() const
{
    return cast(m_pImpl)->maxValue();
}

std::intmax_t IntField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

IntField::ScalingRatio IntField::scaling() const
{
    return cast(m_pImpl)->scaling();
}

const IntField::ValidRangesList&IntField::validRanges() const
{
    return cast(m_pImpl)->validRanges();
}

const IntField::SpecialValues&IntField::specialValues() const
{
    return cast(m_pImpl)->specialValues();
}

} // namespace bbmp
