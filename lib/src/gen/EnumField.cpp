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

#include "commsdsl/gen/EnumField.h"

#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/IntField.h"
#include "commsdsl/gen/strings.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

EnumField::EnumField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Enum);
}

EnumField::~EnumField() = default;

bool EnumField::isUnsignedUnderlyingType() const
{
    return IntField::isUnsignedType(enumDslObj().type());
}

unsigned EnumField::hexWidth() const
{
    auto obj = enumDslObj();

    std::uintmax_t hexWidth = 0U;
    if (obj.hexAssign()) {
        hexWidth = obj.maxLength() * 2U;
    }
    return static_cast<unsigned>(hexWidth);
}

std::string EnumField::valueName(std::intmax_t value) const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    auto iter = revValues.find(value);
    if (iter != revValues.end()) {
        return iter->second;
    }

    return strings::emptyString();
}

commsdsl::parse::EnumField EnumField::enumDslObj() const
{
    return commsdsl::parse::EnumField(dslObj());
}

} // namespace gen

} // namespace commsdsl
