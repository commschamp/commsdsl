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

#pragma once

#include <string>
#include <vector>

#include "CommsdslApi.h"

namespace commsdsl
{

class OptCondImpl;
class COMMSDSL_API OptCond
{
public:

    enum class Kind
    {
        Expr,
        List,
        NumOfValues
    };

    explicit OptCond(const OptCondImpl* impl);
    OptCond(const OptCond& other);
    ~OptCond();

    bool valid() const;

    Kind kind() const;

protected:
    const OptCondImpl* m_pImpl;
};

class OptCondExprImpl;
class COMMSDSL_API OptCondExpr : public OptCond
{
    using Base = OptCond;
public:
    OptCondExpr(const OptCondExprImpl* impl);
    OptCondExpr(OptCond cond);

    const std::string& left() const;
    const std::string& op() const;
    const std::string& right() const;
};

class OptCondListImpl;
class COMMSDSL_API OptCondList : public OptCond
{
    using Base = OptCond;
public:
    enum class Type
    {
        And,
        Or,
        NumOfValues
    };

    using CondList = std::vector<OptCond>;

    OptCondList(const OptCondListImpl* impl);
    OptCondList(OptCond cond);


    Type type() const;
    CondList conditions() const;
};

} // namespace commsdsl
