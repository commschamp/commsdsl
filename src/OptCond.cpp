#include "bbmp/OptCond.h"

#include <cassert>
\
#include "OptCondImpl.h"

namespace bbmp
{

namespace
{

const OptCondExprImpl* castExpr(const OptCondImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const OptCondExprImpl*>(ptr);
}


} // namespace

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

OptCondExpr::OptCondExpr(const OptCondExprImpl* impl)
  : Base(impl)
{
}

OptCondExpr::OptCondExpr(OptCond cond)
  : Base(cond)
{
    assert(kind() == Kind::Expr);
}

const std::string& OptCondExpr::left() const
{
    return castExpr(m_pImpl)->left();
}

const std::string& OptCondExpr::op() const
{
    return castExpr(m_pImpl)->op();
}

const std::string& OptCondExpr::right() const
{
    return castExpr(m_pImpl)->right();
}

} // namespace bbmp
