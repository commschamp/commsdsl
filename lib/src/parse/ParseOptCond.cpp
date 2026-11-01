//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "ParseOptCondImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

namespace
{

[[nodiscard]] const ParseOptCondExprImpl* parseCastExprInternal(const ParseOptCondImpl* ptr)
{
    assert(ptr != nullptr);
    assert(ptr->parseKind() == ParseOptCond::ParseKind::Expr);
    return static_cast<const ParseOptCondExprImpl*>(ptr);
}

[[nodiscard]] const ParseOptCondListImpl* parseCastListInternal(const ParseOptCondImpl* ptr)
{
    assert(ptr != nullptr);
    assert(ptr->parseKind() == ParseOptCond::ParseKind::List);
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

ParseOptCond::ParseKind ParseOptCond::parseKind() const
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
    assert(parseKind() == ParseKind::Expr);
}

const std::string& ParseOptCondExpr::parseLeft() const
{
    return parseCastExprInternal(m_pImpl)->parseLeft();
}

const std::string& ParseOptCondExpr::parseOp() const
{
    return parseCastExprInternal(m_pImpl)->parseOp();
}

const std::string& ParseOptCondExpr::parseRight() const
{
    return parseCastExprInternal(m_pImpl)->parseRight();
}

ParseOptCondExpr::ParseOperandInfo ParseOptCondExpr::parseLeftInfo() const
{
    return parseCastExprInternal(m_pImpl)->parseLeftInfo();
}

ParseOptCondExpr::ParseOperandInfo ParseOptCondExpr::parseRightInfo() const
{
    return parseCastExprInternal(m_pImpl)->parseRightInfo();
}

ParseOptCondList::ParseOptCondList(const ParseOptCondListImpl* impl)
  : Base(impl)
{
}

ParseOptCondList::ParseOptCondList(ParseOptCond cond)
  : Base(cond)
{
    assert(parseKind() == ParseKind::List);
}

ParseOptCondList::ParseType ParseOptCondList::parseType() const
{
    return parseCastListInternal(m_pImpl)->parseType();
}

ParseOptCondList::ParseCondList ParseOptCondList::parseConditions() const
{
    return parseCastListInternal(m_pImpl)->parseCondList();
}

} // namespace parse

} // namespace commsdsl
