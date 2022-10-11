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

class LayerImpl
{
public:    
    LayerImpl(Generator& generator, const commsdsl::parse::Layer& dslObj, Elem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        auto dslField = m_dslObj.field();
        if (!dslField.valid()) {
            if (m_dslObj.kind() != commsdsl::parse::Layer::Kind::Payload) {
                m_generator.logger().error("Layer field definition is missing.");
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
                assert(Should_not_happen);
                return false;
            }

            return true;
        }

        auto extRef = dslField.externalRef();
        if (!extRef.empty()) {
            m_externalField = m_generator.findField(extRef);
            assert(m_externalField != nullptr);
            m_externalField->setReferenced();
            return true;
        }

        m_memberField = Field::create(m_generator, dslField, m_parent);
        if (!m_memberField->prepare()) {
            return false;
        }

        m_memberField->setReferenced();
        return true;
    }

    bool write()
    {
        if (m_memberField != nullptr) {
            return m_memberField->write();
        }

        return true;
    }

    commsdsl::parse::Layer dslObj() const
    {
        return m_dslObj;
    }


    Field* externalField()
    {
        return m_externalField;
    }

    const Field* externalField() const
    {
        return m_externalField;
    }

    Field* memberField()
    {
        return m_memberField.get();
    }

    const Field* memberField() const
    {
        return m_memberField.get();
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
    commsdsl::parse::Layer m_dslObj;
    Elem* m_parent = nullptr;
    Field* m_externalField = nullptr;
    FieldPtr m_memberField;    
};

Layer::Layer(Generator& generator, const commsdsl::parse::Layer& dslObj, Elem* parent) :
    Base(parent),
    m_impl(std::make_unique<LayerImpl>(generator, dslObj, this))
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
    if (!m_impl->prepare()) {
        return false;
    }
    return prepareImpl();
}

bool Layer::write() const
{
    return m_impl->write() && writeImpl();
}

commsdsl::parse::Layer Layer::dslObj() const
{
    return m_impl->dslObj();
}

Field* Layer::externalField()
{
    return m_impl->externalField();
}

const Field* Layer::externalField() const
{
    return m_impl->externalField();
}

Field* Layer::memberField()
{
    return m_impl->memberField();
}

const Field* Layer::memberField() const
{
    return m_impl->memberField();
}

Generator& Layer::generator()
{
    return m_impl->generator();
}

const Generator& Layer::generator() const
{
    return m_impl->generator();
}

Elem::Type Layer::elemTypeImpl() const
{
    return Type_Layer;
}

bool Layer::prepareImpl()
{
    return true;
}

bool Layer::writeImpl() const
{
    return true;
}



} // namespace gen

} // namespace commsdsl
