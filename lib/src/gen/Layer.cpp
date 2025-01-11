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

#include "commsdsl/gen/Layer.h"

#include "commsdsl/gen/CustomLayer.h"
#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

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
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
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
        [[maybe_unused]] static constexpr bool Unexpected_kind = false;
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

bool Layer::forceCommsOrder(LayersAccessList& layers, bool& success) const
{
    return forceCommsOrderImpl(layers, success);
}

std::string Layer::templateScopeOfComms(const std::string& iFaceStr, const std::string& allMessagesStr, const std::string& protOptionsStr) const
{
    auto commsScope = comms::scopeFor(*this, generator());
    std::string optionsParams = "<" + protOptionsStr + ">";

    auto* parent = getParent();
    assert(parent != nullptr);
    assert(parent->elemType() == commsdsl::gen::Elem::Type_Frame);

    auto optLevelScope = comms::scopeFor(*parent, generator()) + strings::layersSuffixStr();
    assert(optLevelScope.size() < commsScope.size());
    assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
    
    auto result = optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());

    auto* frame = static_cast<const Frame*>(parent);
    [[maybe_unused]] bool success = true;
    auto allLayers = frame->getCommsOrderOfLayers(success);
    assert(success);

    auto iter = 
        std::find_if(
            allLayers.begin(), allLayers.end(), 
            [this](const auto* l)
            {
                return this == l;
            });

    if (iter == allLayers.end()) {
        assert(false); // Mustn't happen
        return result;
    }

    auto addIdParams = 
        [&iFaceStr, &allMessagesStr, &result]()
        {
            static const std::string Templ = 
                "<#^#INTERFACE#$#, #^#ALL_MESSAGES#$#>";

            util::ReplacementMap repl = {
                {"INTERFACE", iFaceStr},
                {"ALL_MESSAGES", allMessagesStr}
            };

            result += util::processTemplate(Templ, repl);
        };

    for (auto iterTmp = iter; iterTmp != allLayers.end(); ++iterTmp) {
        auto kind = (*iterTmp)->dslObj().kind();
        if (kind == commsdsl::parse::Layer::Kind::Id) {
            addIdParams();
            break;
        }

        if (kind != commsdsl::parse::Layer::Kind::Custom) {
            continue;
        }

        auto& customLayer = static_cast<const CustomLayer&>(**iterTmp);
        auto customKind = customLayer.customDslObj().semanticLayerType();
        if (customKind == commsdsl::parse::Layer::Kind::Id) {
            addIdParams();
            break;            
        }
    }

    return result;
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

bool Layer::forceCommsOrderImpl([[maybe_unused]] LayersAccessList& layers, bool& success) const
{
    success = true;
    return false;
}

} // namespace gen

} // namespace commsdsl
