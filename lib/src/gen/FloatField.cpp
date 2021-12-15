//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/FloatField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

FloatField::FloatField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Float);
}

FloatField::~FloatField() = default;

commsdsl::parse::FloatField FloatField::floatDslObj() const
{
    return commsdsl::parse::FloatField(dslObj());
}

} // namespace gen

} // namespace commsdsl
