#include "commsdsl/OptionalField.h"

#include <cassert>

#include "OptionalFieldImpl.h"

namespace commsdsl
{

namespace
{

const OptionalFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const OptionalFieldImpl*>(ptr);
}

} // namespace

OptionalField::OptionalField(const OptionalFieldImpl* impl)
  : Base(impl)
{
}

OptionalField::OptionalField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Optional);
}

OptionalField::Mode OptionalField::defaultMode() const
{
    return cast(m_pImpl)->defaultMode();
}

Field OptionalField::field() const
{
    return cast(m_pImpl)->field();
}

OptCond OptionalField::cond() const
{
    return cast(m_pImpl)->wrappedCondition();
}

bool OptionalField::externalModeCtrl() const
{
    return cast(m_pImpl)->externalModeCtrl();
}


} // namespace commsdsl
