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

#include "commsdsl/gen/DataField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

DataField::DataField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Data);
}

DataField::~DataField() = default;

commsdsl::parse::DataField DataField::dataDslObj() const
{
    return commsdsl::parse::DataField(dslObj());
}

} // namespace gen

} // namespace commsdsl
