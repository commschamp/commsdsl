#include "bbmp/BundleField.h"

#include <cassert>

#include "BundleFieldImpl.h"

namespace bbmp
{

namespace
{

const BundleFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const BundleFieldImpl*>(ptr);
}

} // namespace

BundleField::BundleField(const BundleFieldImpl* impl)
  : Base(impl)
{
}

BundleField::BundleField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Bundle);
}

const BundleField::Members& BundleField::members() const
{
    return cast(m_pImpl)->members();
}

} // namespace bbmp
