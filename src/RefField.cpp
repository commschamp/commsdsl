#include "commsdsl/RefField.h"

#include <cassert>

#include "RefFieldImpl.h"

namespace commsdsl
{

namespace
{

const RefFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const RefFieldImpl*>(ptr);
}

} // namespace

RefField::RefField(const RefFieldImpl* impl)
  : Base(impl)
{
}

RefField::RefField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Ref);
}

Field RefField::field() const
{
    return cast(m_pImpl)->field();
}

} // namespace commsdsl
