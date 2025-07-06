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

    enum class Kind
    {
        Expr,
        List,
        NumOfValues
    };

    explicit ParseOptCond(const ParseOptCondImpl* impl);
    ParseOptCond(const ParseOptCond& other);
    ~ParseOptCond();

    bool parseValid() const;

    Kind parseKind() const;

protected:
    const ParseOptCondImpl* m_pImpl;
};

class ParseOptCondExprImpl;
class COMMSDSL_API ParseOptCondExpr : public ParseOptCond
{
    using Base = ParseOptCond;
public:
    enum class OperandType
    {
        Invalid,
        Value,
        SiblingRef,
        InterfaceRef,
        NumOfValues
    };

    enum class AccMode
    {
        Itself,
        Size,
        Exists,
        NumOfValues
    };

    struct OperandInfo
    {
        OperandType m_type = OperandType::Invalid;
        AccMode m_mode = AccMode::Itself;
        std::string m_access;
    };

    ParseOptCondExpr(const ParseOptCondExprImpl* impl);
    ParseOptCondExpr(ParseOptCond cond);

    const std::string& parseLeft() const;
    const std::string& parseOp() const;
    const std::string& parseRight() const;
    OperandInfo parseLeftInfo() const;
    OperandInfo parseRightInfo() const;
};

class ParseOptCondListImpl;
class COMMSDSL_API ParseOptCondList : public ParseOptCond
{
    using Base = ParseOptCond;
public:
    enum class Type
    {
        And,
        Or,
        NumOfValues
    };

    using CondList = std::vector<ParseOptCond>;

    ParseOptCondList(const ParseOptCondListImpl* impl);
    ParseOptCondList(ParseOptCond cond);


    Type parseType() const;
    CondList parseConditions() const;
};

} // namespace parse

} // namespace commsdsl
