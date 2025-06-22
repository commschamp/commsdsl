//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseOptCond.h"

#include <cassert>

#include "ParseOptCondImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseOptCondExprImpl* castExpr(const ParseOptCondImpl* ptr)
{
    assert(ptr != nullptr);
    assert(ptr->kind() == ParseOptCond::Kind::Expr);
    return static_cast<const ParseOptCondExprImpl*>(ptr);
}

const ParseOptCondListImpl* castList(const ParseOptCondImpl* ptr)
{
    assert(ptr != nullptr);
    assert(ptr->kind() == ParseOptCond::Kind::List);
    return static_cast<const ParseOptCondListImpl*>(ptr);
}

} // namespace

ParseOptCond::ParseOptCond(const ParseOptCondImpl* impl)
  : m_pImpl(impl)
{
}

ParseOptCond::ParseOptCond(const ParseOptCond& ) = default;

ParseOptCond::~ParseOptCond() = default;

bool ParseOptCond::valid() const
{
    return m_pImpl != nullptr;
}

ParseOptCond::Kind ParseOptCond::kind() const
{
    assert(valid());
    return m_pImpl->kind();
}

ParseOptCondExpr::ParseOptCondExpr(const ParseOptCondExprImpl* impl)
  : Base(impl)
{
}

ParseOptCondExpr::ParseOptCondExpr(ParseOptCond cond)
  : Base(cond)
{
    assert(kind() == Kind::Expr);
}

const std::string& ParseOptCondExpr::left() const
{
    return castExpr(m_pImpl)->left();
}

const std::string& ParseOptCondExpr::op() const
{
    return castExpr(m_pImpl)->op();
}

const std::string& ParseOptCondExpr::right() const
{
    return castExpr(m_pImpl)->right();
}

ParseOptCondExpr::OperandInfo ParseOptCondExpr::leftInfo() const
{
    return castExpr(m_pImpl)->leftInfo();
}

ParseOptCondExpr::OperandInfo ParseOptCondExpr::rightInfo() const
{
    return castExpr(m_pImpl)->rightInfo();
}

ParseOptCondList::ParseOptCondList(const ParseOptCondListImpl* impl)
  : Base(impl)
{
}

ParseOptCondList::ParseOptCondList(ParseOptCond cond)
  : Base(cond)
{
    assert(kind() == Kind::List);
}

ParseOptCondList::Type ParseOptCondList::type() const
{
    return castList(m_pImpl)->type();
}

ParseOptCondList::CondList ParseOptCondList::conditions() const
{
    return castList(m_pImpl)->condList();
}


} // namespace parse

} // namespace commsdsl
