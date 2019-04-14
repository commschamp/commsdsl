//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "commsdsl/OptCond.h"

#include <cassert>

#include "OptCondImpl.h"

namespace commsdsl
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

commsdsl::OptCond::OptCond(const commsdsl::OptCondImpl* impl)
  : m_pImpl(impl)
{
}

commsdsl::OptCond::OptCond(const commsdsl::OptCond& ) = default;

commsdsl::OptCond::~OptCond() = default;

bool OptCond::valid() const
{
    return m_pImpl != nullptr;
}

commsdsl::OptCond::Kind commsdsl::OptCond::kind() const
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


} // namespace commsdsl
