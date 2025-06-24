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
    assert(ptr->parseKind() == ParseOptCond::Kind::Expr);
    return static_cast<const ParseOptCondExprImpl*>(ptr);
}

const ParseOptCondListImpl* castList(const ParseOptCondImpl* ptr)
{
    assert(ptr != nullptr);
    assert(ptr->parseKind() == ParseOptCond::Kind::List);
    return static_cast<const ParseOptCondListImpl*>(ptr);
}

} // namespace

ParseOptCond::ParseOptCond(const ParseOptCondImpl* impl)
  : m_pImpl(impl)
{
}

ParseOptCond::ParseOptCond(const ParseOptCond& ) = default;

ParseOptCond::~ParseOptCond() = default;

bool ParseOptCond::parseValid() const
{
    return m_pImpl != nullptr;
}

ParseOptCond::Kind ParseOptCond::parseKind() const
{
    assert(parseValid());
    return m_pImpl->parseKind();
}

ParseOptCondExpr::ParseOptCondExpr(const ParseOptCondExprImpl* impl)
  : Base(impl)
{
}

ParseOptCondExpr::ParseOptCondExpr(ParseOptCond cond)
  : Base(cond)
{
    assert(parseKind() == Kind::Expr);
}

const std::string& ParseOptCondExpr::parseLeft() const
{
    return castExpr(m_pImpl)->parseLeft();
}

const std::string& ParseOptCondExpr::parseOp() const
{
    return castExpr(m_pImpl)->parseOp();
}

const std::string& ParseOptCondExpr::parseRight() const
{
    return castExpr(m_pImpl)->parseRight();
}

ParseOptCondExpr::OperandInfo ParseOptCondExpr::parseLeftInfo() const
{
    return castExpr(m_pImpl)->parseLeftInfo();
}

ParseOptCondExpr::OperandInfo ParseOptCondExpr::parseRightInfo() const
{
    return castExpr(m_pImpl)->parseRightInfo();
}

ParseOptCondList::ParseOptCondList(const ParseOptCondListImpl* impl)
  : Base(impl)
{
}

ParseOptCondList::ParseOptCondList(ParseOptCond cond)
  : Base(cond)
{
    assert(parseKind() == Kind::List);
}

ParseOptCondList::Type ParseOptCondList::parseType() const
{
    return castList(m_pImpl)->parseType();
}

ParseOptCondList::CondList ParseOptCondList::parseConditions() const
{
    return castList(m_pImpl)->parseCondList();
}


} // namespace parse

} // namespace commsdsl
