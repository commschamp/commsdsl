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
    assert(ptr->kind() == OptCond::Kind::Expr);
    return static_cast<const OptCondExprImpl*>(ptr);
}

const OptCondListImpl* castList(const OptCondImpl* ptr)
{
    assert(ptr != nullptr);
    assert(ptr->kind() == OptCond::Kind::List);
    return static_cast<const OptCondListImpl*>(ptr);
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

OptCondList::OptCondList(const OptCondListImpl* impl)
  : Base(impl)
{
}

OptCondList::OptCondList(OptCond cond)
  : Base(cond)
{
    assert(kind() == Kind::List);
}

OptCondList::Type OptCondList::type() const
{
    return castList(m_pImpl)->type();
}

OptCondList::CondList OptCondList::conditions() const
{
    return castList(m_pImpl)->condList();
}


} // namespace bbmp
