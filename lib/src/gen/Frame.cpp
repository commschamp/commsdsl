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

#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Layer.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class FrameImpl
{
public:
    using LayersList = Frame::LayersList;

    FrameImpl(Generator& generator, commsdsl::parse::Frame dslObj, Elem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        if (!m_dslObj.valid()) {
            return true;
        }

        auto layers = m_dslObj.layers();
        m_layers.reserve(layers.size());
        for (auto& dslObj : layers) {
            auto ptr = Layer::create(m_generator, dslObj, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_layers.push_back(std::move(ptr));
        }

        return true;
    }

    bool write() const
    {
        bool result = 
            std::all_of(
                m_layers.begin(), m_layers.end(),
                [](auto& layerPtr) -> bool
                {
                    return layerPtr->write();
                });

        return result;
    }

    commsdsl::parse::Frame dslObj() const
    {
        return m_dslObj;
    }

    const LayersList& layers() const
    {
        return m_layers;
    }

    LayersList& layers()
    {
        return m_layers;
    }

    const Generator& generator() const
    {
        return m_generator;
    }

    Generator& generator()
    {
        return m_generator;
    }    

private:
    Generator& m_generator;
    commsdsl::parse::Frame m_dslObj;
    Elem* m_parent = nullptr;
    LayersList m_layers;
}; 

Frame::Frame(Generator& generator, commsdsl::parse::Frame dslObj, Elem* parent) :
    Base(parent),
    m_impl(std::make_unique<FrameImpl>(generator, dslObj, this))
{
}

Frame::~Frame() = default;

bool Frame::prepare()
{
    if (!m_impl->prepare()) {
        return false;
    }

    return prepareImpl();
}

bool Frame::write() const
{
    if (!m_impl->write()) {
        return false;
    }

    return writeImpl();
}

commsdsl::parse::Frame Frame::dslObj() const
{
    return m_impl->dslObj();
}

const Frame::LayersList& Frame::layers() const
{
    return m_impl->layers();
}

Generator& Frame::generator()
{
    return m_impl->generator();
}

const Generator& Frame::generator() const
{
    return m_impl->generator();
}

Frame::LayersAccessList Frame::getCommsOrderOfLayers(bool& success) const
{
    LayersAccessList result;
    for (auto& lPtr : layers()) {
        result.push_back(lPtr.get());
    }

    assert(!result.empty());
    while (true) {
        bool rearanged = false;
        for (auto* l : result) {
            rearanged = l->forceCommsOrder(result, success);

            if (!success) {
                break;
            }

            if (rearanged) {
                // Order has changed restart from the beginning
                break;
            }
        }

        if (!rearanged) {
            // reordering is complete
            break;
        }
    }   

    return result;    
}

Elem::Type Frame::elemTypeImpl() const
{
    return Type_Frame;
}

bool Frame::prepareImpl()
{
    return true;
}

bool Frame::writeImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
