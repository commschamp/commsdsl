#include "commsdsl/SetField.h"

#include <cassert>

#include "SetFieldImpl.h"

namespace commsdsl
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

bool SetField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

} // namespace commsdsl
