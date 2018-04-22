#include "bbmp/OptCond.h"

#include <cassert>
\
#include "OptCondImpl.h"

namespace bbmp
{

bbmp::OptCond::OptCond(const bbmp::OptCondImpl* impl)
  : m_pImpl(impl)
{
}

bbmp::OptCond::OptCond(const bbmp::OptCond& ) = default;

bbmp::OptCond::~OptCond() = default;

bool OptCond::valid() const
{
    return m_pImpl != nullptr;
}

bbmp::OptCond::Kind bbmp::OptCond::kind() const
{
    assert(valid());
    return m_pImpl->kind();
}

} // namespace bbmp
