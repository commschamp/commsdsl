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
    using ParseFrame = GenFrame::ParseFrame;

    GenFrameImpl(GenGenerator& generator, ParseFrame parseObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        auto layers = m_dslObj.parseLayers();
        m_layers.reserve(layers.size());
        for (auto& parseObj : layers) {
            auto ptr = GenLayer::genCreate(m_generator, parseObj, m_parent);
            assert(ptr);
            if (!ptr->genPrepare()) {
                return false;
            }

            m_layers.push_back(std::move(ptr));
        }

        return true;
    }

    bool genWrite() const
    {
        bool result = 
            std::all_of(
                m_layers.begin(), m_layers.end(),
                [](auto& layerPtr) -> bool
                {
                    return layerPtr->genWrite();
                });

        return result;
    }

    ParseFrame genParseObj() const
    {
        return m_dslObj;
    }

    const LayersList& genLayers() const
    {
        return m_layers;
    }

    LayersList& genLayers()
    {
        return m_layers;
    }

    const GenGenerator& genGenerator() const
    {
        return m_generator;
    }

    GenGenerator& genGenerator()
    {
        return m_generator;
    }    

private:
    GenGenerator& m_generator;
    ParseFrame m_dslObj;
    GenElem* m_parent = nullptr;
    LayersList m_layers;
}; 

GenFrame::GenFrame(GenGenerator& generator, ParseFrame parseObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenFrameImpl>(generator, parseObj, this))
{
}

GenFrame::~GenFrame() = default;

bool GenFrame::genPrepare()
{
    if (!m_impl->genPrepare()) {
        return false;
    }

    return genPrepareImpl();
}

bool GenFrame::genWrite() const
{
    if (!m_impl->genWrite()) {
        return false;
    }

    return genWriteImpl();
}

GenFrame::ParseFrame GenFrame::genParseObj() const
{
    return m_impl->genParseObj();
}

const GenFrame::LayersList& GenFrame::genLayers() const
{
    return m_impl->genLayers();
}

GenGenerator& GenFrame::genGenerator()
{
    return m_impl->genGenerator();
}

const GenGenerator& GenFrame::genGenerator() const
{
    return m_impl->genGenerator();
}

GenFrame::LayersAccessList GenFrame::getCommsOrderOfLayers(bool& success) const
{
    LayersAccessList result;
    for (auto& lPtr : genLayers()) {
        result.push_back(lPtr.get());
    }

    assert(!result.empty());
    while (true) {
        bool rearanged = false;
        for (auto* l : result) {
            rearanged = l->genForceCommsOrder(result, success);

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

const GenNamespace* GenFrame::genParentNamespace() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == GenElem::Type_Namespace);
    return static_cast<const GenNamespace*>(parent);
}

GenElem::Type GenFrame::genElemTypeImpl() const
{
    return Type_Frame;
}

bool GenFrame::genPrepareImpl()
{
    return true;
}

bool GenFrame::genWriteImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
