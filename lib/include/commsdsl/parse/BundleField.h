//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "Alias.h"
#include "OptCond.h"
#include "Field.h"

namespace commsdsl
{

namespace parse
{

class BundleFieldImpl;
class COMMSDSL_API BundleField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;
    using Aliases = std::vector<Alias>;

    explicit BundleField(const BundleFieldImpl* impl);
    explicit BundleField(Field field);

    Members members() const;
    Aliases aliases() const;
    OptCond validCond() const;    
};

} // namespace parse

} // namespace commsdsl
