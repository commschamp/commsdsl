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

class FieldImpl
{
public:
    FieldImpl(Generator& generator, const commsdsl::parse::Field& dslObj) : 
        m_generator(generator),
        m_dslObj(dslObj)
    {
    }

    bool isPrepared()
    {
        return m_prepared;
    }

    void setPrepared()
    {
        m_prepared = true;
    }

    const commsdsl::parse::Field& dslObj() const
    {
        return m_dslObj;
    } 

    Generator& generator()
    {
        return m_generator;
    }

    const Generator& generator() const
    {
        return m_generator;
    }

private:
    Generator& m_generator;
    commsdsl::parse::Field m_dslObj;
    bool m_prepared = false;
};    

Field::Field(Generator& generator, const commsdsl::parse::Field& dslObj, Elem* parent) :
    Base(parent),
    m_impl(std::make_unique<FieldImpl>(generator, dslObj))
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

bool Field::isPrepared() const
{
    return m_impl->isPrepared();
}

bool Field::prepare()
{
    if (m_impl->isPrepared()) {
        return true;
    }

    bool result = prepareImpl();
    if (result) {
        m_impl->setPrepared();
    }
    return result;
}

bool Field::write() const
{
    return writeImpl();
}

const commsdsl::parse::Field& Field::dslObj() const
{
    return m_impl->dslObj();
}

Generator& Field::generator()
{
    return m_impl->generator();
}

const Generator& Field::generator() const
{
    return m_impl->generator();
}

Elem::Type Field::elemTypeImpl() const
{
    return Type_Field;
}

bool Field::prepareImpl()
{
    return true;
}

bool Field::writeImpl() const
{
    return true;
}


} // namespace gen

} // namespace commsdsl
