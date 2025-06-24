//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/GenNamespace.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class GenFrameImpl
{
public:
    using LayersList = GenFrame::LayersList;

    GenFrameImpl(GenGenerator& generator, commsdsl::parse::ParseFrame dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        auto layers = m_dslObj.parseLayers();
        m_layers.reserve(layers.size());
        for (auto& dslObj : layers) {
            auto ptr = GenLayer::create(m_generator, dslObj, m_parent);
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

    commsdsl::parse::ParseFrame dslObj() const
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

    const GenGenerator& generator() const
    {
        return m_generator;
    }

    GenGenerator& generator()
    {
        return m_generator;
    }    

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseFrame m_dslObj;
    GenElem* m_parent = nullptr;
    LayersList m_layers;
}; 

GenFrame::GenFrame(GenGenerator& generator, commsdsl::parse::ParseFrame dslObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenFrameImpl>(generator, dslObj, this))
{
}

GenFrame::~GenFrame() = default;

bool GenFrame::prepare()
{
    if (!m_impl->prepare()) {
        return false;
    }

    return prepareImpl();
}

bool GenFrame::write() const
{
    if (!m_impl->write()) {
        return false;
    }

    return writeImpl();
}

commsdsl::parse::ParseFrame GenFrame::dslObj() const
{
    return m_impl->dslObj();
}

const GenFrame::LayersList& GenFrame::layers() const
{
    return m_impl->layers();
}

GenGenerator& GenFrame::generator()
{
    return m_impl->generator();
}

const GenGenerator& GenFrame::generator() const
{
    return m_impl->generator();
}

GenFrame::LayersAccessList GenFrame::getCommsOrderOfLayers(bool& success) const
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

const GenNamespace* GenFrame::parentNamespace() const
{
    auto* parent = getParent();
    assert(parent != nullptr);
    assert(parent->elemType() == GenElem::Type_Namespace);
    return static_cast<const GenNamespace*>(parent);
}

GenElem::Type GenFrame::elemTypeImpl() const
{
    return Type_Frame;
}

bool GenFrame::prepareImpl()
{
    return true;
}

bool GenFrame::writeImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
