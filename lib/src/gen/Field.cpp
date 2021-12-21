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

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

Field::Field(Generator& generator, const commsdsl::parse::Field& dslObj, Elem* parent) :
    Base(parent),
    m_generator(generator),
    m_dslObj(dslObj)
{
}

Field::~Field() = default;

Field::Ptr Field::create(Generator& generator, commsdsl::parse::Field dslobj, Elem* parent)
{
    using CreateFunc = FieldPtr (Generator::*)(commsdsl::parse::Field dslobj, Elem* parent);
    static const CreateFunc Map[] = {
        /* Int */ &Generator::createIntField,
        /* Enum */ &Generator::createEnumField,
        /* Set */ &Generator::createSetField,
        /* Float */ &Generator::createFloatField,
        /* Bitfield */ &Generator::createBitfieldField,
        /* Bundle */ &Generator::createBundleField,
        /* String */ &Generator::createStringField,
        /* Data */ &Generator::createDataField,
        /* List */ &Generator::createListField,
        /* Ref */ &Generator::createRefField,
        /* Optional */ &Generator::createOptionalField,
        /* Variant */ &Generator::createVariantField,
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(commsdsl::parse::Field::Kind::NumOfValues), "Invalid map");

    auto idx = static_cast<std::size_t>(dslobj.kind());
    if (MapSize <= idx) {
        static constexpr bool Unexpected_kind = false;
        static_cast<void>(Unexpected_kind);
        assert(Unexpected_kind);          
        return Ptr();
    }

    auto func = Map[idx];
    assert(func != nullptr); // NYI
    return (generator.*func)(dslobj, parent);
}

bool Field::prepare()
{
    return prepareImpl();
}

std::string Field::genCode()
{
    return genCodeImpl();
}

bool Field::write()
{
    return writeImpl();
}

Elem::Type Field::elemTypeImpl() const
{
    return Type_Field;
}

bool Field::prepareImpl()
{
    return true;
}

std::string Field::genCodeImpl()
{
    return std::string();
}

bool Field::writeImpl()
{
    return true;
}

Generator& Field::generator()
{
    return m_generator;
}

const commsdsl::parse::Field& Field::dslObj() const
{
    return m_dslObj;
}


} // namespace gen

} // namespace commsdsl
