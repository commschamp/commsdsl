#include "bbmp/SetField.h"

#include <cassert>

#include "SetFieldImpl.h"

namespace bbmp
{

namespace
{

const SetFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const SetFieldImpl*>(ptr);
}

} // namespace

SetField::SetField(const SetFieldImpl* impl)
  : Base(impl)
{
}

SetField::SetField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Set);
}

SetField::Type SetField::type() const
{
    return cast(m_pImpl)->type();
}

Endian SetField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::size_t SetField::length() const
{
    return cast(m_pImpl)->length();
}

std::size_t SetField::bitLength() const
{
    return cast(m_pImpl)->bitLength();
}

bool SetField::defaultBitValue() const
{
    return cast(m_pImpl)->defaultBitValue();
}

bool SetField::reservedBitValue() const
{
    return cast(m_pImpl)->reservedBitValue();
}

const SetField::Bits& SetField::bits() const
{
    return cast(m_pImpl)->bits();
}

const SetField::RevBits& SetField::revBits() const
{
    return cast(m_pImpl)->revBits();
}

bool SetField::isNonUniqueAllowed() const
{
    return cast(m_pImpl)->isNonUniqueAllowed();
}

bool SetField::isUnique() const
{
    return cast(m_pImpl)->isUnique();
}

} // namespace bbmp
