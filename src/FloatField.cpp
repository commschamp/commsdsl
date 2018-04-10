#include "bbmp/FloatField.h"

#include <cassert>

#include "FloatFieldImpl.h"

namespace bbmp
{

namespace
{

const FloatFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const FloatFieldImpl*>(ptr);
}

} // namespace

FloatField::FloatField(const FloatFieldImpl* impl)
  : Base(impl)
{
}

FloatField::FloatField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Float);
}

FloatField::Type FloatField::type() const
{
    return cast(m_pImpl)->type();
}

Endian FloatField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::size_t FloatField::length() const
{
    return cast(m_pImpl)->length();
}

double FloatField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const FloatField::ValidRangesList& FloatField::validRanges() const
{
    return cast(m_pImpl)->validRanges();
}

const FloatField::SpecialValues& FloatField::specialValues() const
{
    return cast(m_pImpl)->specialValues();
}

bool FloatField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

} // namespace bbmp
