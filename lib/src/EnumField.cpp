#include "commsdsl/EnumField.h"

#include <cassert>

#include "EnumFieldImpl.h"

namespace commsdsl
{

namespace
{

const EnumFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const EnumFieldImpl*>(ptr);
}

} // namespace

EnumField::EnumField(const EnumFieldImpl* impl)
  : Base(impl)
{
}

EnumField::EnumField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Enum);
}

EnumField::Type EnumField::type() const
{
    return cast(m_pImpl)->type();
}

Endian EnumField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::intmax_t EnumField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const EnumField::Values& EnumField::values() const
{
    return cast(m_pImpl)->values();
}

const EnumField::RevValues& EnumField::revValues() const
{
    return cast(m_pImpl)->revValues();
}

bool EnumField::isNonUniqueAllowed() const
{
    return cast(m_pImpl)->isNonUniqueAllowed();
}

bool EnumField::isUnique() const
{
    return cast(m_pImpl)->isUnique();
}

bool commsdsl::EnumField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

bool EnumField::hexAssign() const
{
    return cast(m_pImpl)->hexAssign();
}

} // namespace commsdsl
