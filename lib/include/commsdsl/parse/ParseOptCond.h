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

#pragma once

#include "commsdsl/CommsdslApi.h"

#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseOptCondImpl;
class COMMSDSL_API ParseOptCond
{
public:

    enum class ParseKind
    {
        Expr,
        List,
        NumOfValues
    };

    explicit ParseOptCond(const ParseOptCondImpl* impl);
    ParseOptCond(const ParseOptCond& other);
    ~ParseOptCond();

    bool parseValid() const;

    ParseKind parseKind() const;

protected:
    const ParseOptCondImpl* m_pImpl;
};

class ParseOptCondExprImpl;
class COMMSDSL_API ParseOptCondExpr : public ParseOptCond
{
    using Base = ParseOptCond;
public:
    enum class ParseOperandType
    {
        Invalid,
        Value,
        SiblingRef,
        InterfaceRef,
        NumOfValues
    };

    enum class ParseAccMode
    {
        Itself,
        Size,
        Exists,
        NumOfValues
    };

    struct ParseOperandInfo
    {
        ParseOperandType m_type = ParseOperandType::Invalid;
        ParseAccMode m_mode = ParseAccMode::Itself;
        std::string m_access;
    };

    ParseOptCondExpr(const ParseOptCondExprImpl* impl);
    ParseOptCondExpr(ParseOptCond cond);

    const std::string& parseLeft() const;
    const std::string& parseOp() const;
    const std::string& parseRight() const;
    ParseOperandInfo parseLeftInfo() const;
    ParseOperandInfo parseRightInfo() const;
};

class ParseOptCondListImpl;
class COMMSDSL_API ParseOptCondList : public ParseOptCond
{
    using Base = ParseOptCond;
public:
    enum class ParseType
    {
        And,
        Or,
        NumOfValues
    };

    using ParseCondList = std::vector<ParseOptCond>;

    ParseOptCondList(const ParseOptCondListImpl* impl);
    ParseOptCondList(ParseOptCond cond);

    ParseType parseType() const;
    ParseCondList parseConditions() const;
};

} // namespace parse

} // namespace commsdsl
