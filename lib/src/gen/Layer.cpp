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

#include "commsdsl/gen/Layer.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

Layer::Layer(Generator& generator, const commsdsl::parse::Layer& dslObj, Elem* parent) :
    Base(parent),
    m_generator(generator),
    m_dslObj(dslObj)
{
}

Layer::~Layer() = default;

Layer::Ptr Layer::create(Generator& generator, commsdsl::parse::Layer dslobj, Elem* parent)
{
    using CreateFunc = LayerPtr (Generator::*)(commsdsl::parse::Layer dslobj, Elem* parent);
    static const CreateFunc Map[] = {
        /* Custom */ &Generator::createCustomLayer,
        /* Sync */ &Generator::createSyncLayer,
        /* Size */ &Generator::createSizeLayer,
        /* Id */ &Generator::createIdLayer,
        /* Value */ &Generator::createValueLayer,
        /* Payload */ &Generator::createPayloadLayer,
        /* Checksum */ &Generator::createChecksumLayer,
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(commsdsl::parse::Layer::Kind::NumOfValues), "Invalid map");

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

bool Layer::prepare()
{
    return prepareImpl();
}

bool Layer::write()
{
    return writeImpl();
}

Elem::Type Layer::elemTypeImpl() const
{
    return Type_Layer;
}

bool Layer::writeImpl()
{
    return true;
}

Generator& Layer::generator()
{
    return m_generator;
}

const commsdsl::parse::Layer& Layer::dslObj() const
{
    return m_dslObj;
}


} // namespace gen

} // namespace commsdsl
