#include "commsdsl/VariantField.h"

#include <cassert>

#include "VariantFieldImpl.h"

namespace commsdsl
{

namespace
{

const VariantFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const VariantFieldImpl*>(ptr);
}

} // namespace

VariantField::VariantField(const VariantFieldImpl* impl)
  : Base(impl)
{
}

VariantField::VariantField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Variant);
}

VariantField::Members VariantField::members() const
{
    return cast(m_pImpl)->membersList();
}

std::size_t VariantField::defaultMemberIdx() const
{
    return cast(m_pImpl)->defaultMemberIdx();
}

bool VariantField::displayIdxReadOnlyHidden() const
{
    return cast(m_pImpl)->displayIdxReadOnlyHidden();
}

} // namespace commsdsl
