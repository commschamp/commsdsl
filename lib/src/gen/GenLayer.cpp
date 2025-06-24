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

#include "commsdsl/gen/GenLayer.h"

#include "commsdsl/gen/GenCustomLayer.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class GenLayerImpl
{
public:    
    GenLayerImpl(GenGenerator& generator, const commsdsl::parse::ParseLayer& dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        auto dslField = m_dslObj.parseField();
        if (!dslField.parseValid()) {
            if (m_dslObj.parseKind() != commsdsl::parse::ParseLayer::Kind::Payload) {
                m_generator.logger().error("GenLayer field definition is missing.");
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                return false;
            }

            return true;
        }

        auto extRef = dslField.parseExternalRef();
        if (!extRef.empty()) {
            m_externalField = m_generator.findField(extRef);
            assert(m_externalField != nullptr);
            m_externalField->setReferenced();
            return true;
        }

        m_memberField = GenField::create(m_generator, dslField, m_parent);
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

    commsdsl::parse::ParseLayer dslObj() const
    {
        return m_dslObj;
    }


    GenField* externalField()
    {
        return m_externalField;
    }

    const GenField* externalField() const
    {
        return m_externalField;
    }

    GenField* memberField()
    {
        return m_memberField.get();
    }

    const GenField* memberField() const
    {
        return m_memberField.get();
    }        

    GenGenerator& generator()
    {
        return m_generator;
    }

    const GenGenerator& generator() const
    {
        return m_generator;
    }    

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseLayer m_dslObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalField = nullptr;
    FieldPtr m_memberField;    
};

GenLayer::GenLayer(GenGenerator& generator, const commsdsl::parse::ParseLayer& dslObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenLayerImpl>(generator, dslObj, this))
{
}

GenLayer::~GenLayer() = default;

GenLayer::Ptr GenLayer::create(GenGenerator& generator, commsdsl::parse::ParseLayer dslobj, GenElem* parent)
{
    using CreateFunc = LayerPtr (GenGenerator::*)(commsdsl::parse::ParseLayer dslobj, GenElem* parent);
    static const CreateFunc Map[] = {
        /* Custom */ &GenGenerator::createCustomLayer,
        /* Sync */ &GenGenerator::createSyncLayer,
        /* Size */ &GenGenerator::createSizeLayer,
        /* Id */ &GenGenerator::createIdLayer,
        /* Value */ &GenGenerator::createValueLayer,
        /* Payload */ &GenGenerator::createPayloadLayer,
        /* Checksum */ &GenGenerator::createChecksumLayer,
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(commsdsl::parse::ParseLayer::Kind::NumOfValues), "Invalid map");

    auto idx = static_cast<std::size_t>(dslobj.parseKind());
    if (MapSize <= idx) {
        [[maybe_unused]] static constexpr bool Unexpected_kind = false;
        assert(Unexpected_kind);          
        return Ptr();
    }

    auto func = Map[idx];
    assert(func != nullptr); // NYI
    return (generator.*func)(dslobj, parent);
}

bool GenLayer::prepare()
{
    if (!m_impl->prepare()) {
        return false;
    }
    return prepareImpl();
}

bool GenLayer::write() const
{
    return m_impl->write() && writeImpl();
}

commsdsl::parse::ParseLayer GenLayer::dslObj() const
{
    return m_impl->dslObj();
}

GenField* GenLayer::externalField()
{
    return m_impl->externalField();
}

const GenField* GenLayer::externalField() const
{
    return m_impl->externalField();
}

GenField* GenLayer::memberField()
{
    return m_impl->memberField();
}

const GenField* GenLayer::memberField() const
{
    return m_impl->memberField();
}

GenGenerator& GenLayer::generator()
{
    return m_impl->generator();
}

const GenGenerator& GenLayer::generator() const
{
    return m_impl->generator();
}

bool GenLayer::forceCommsOrder(LayersAccessList& layers, bool& success) const
{
    return forceCommsOrderImpl(layers, success);
}

std::string GenLayer::templateScopeOfComms(const std::string& iFaceStr, const std::string& allMessagesStr, const std::string& protOptionsStr) const
{
    auto commsScope = comms::scopeFor(*this, generator());
    std::string optionsParams = "<" + protOptionsStr + ">";

    auto* parent = getParent();
    assert(parent != nullptr);
    assert(parent->elemType() == commsdsl::gen::GenElem::Type_Frame);

    auto optLevelScope = comms::scopeFor(*parent, generator()) + strings::layersSuffixStr();
    assert(optLevelScope.size() < commsScope.size());
    assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
    
    auto result = optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());

    auto* frame = static_cast<const GenFrame*>(parent);
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
        auto kind = (*iterTmp)->dslObj().parseKind();
        if (kind == commsdsl::parse::ParseLayer::Kind::Id) {
            addIdParams();
            break;
        }

        if (kind != commsdsl::parse::ParseLayer::Kind::Custom) {
            continue;
        }

        auto& customLayer = static_cast<const GenCustomLayer&>(**iterTmp);
        auto customKind = customLayer.customDslObj().parseSemanticLayerType();
        if (customKind == commsdsl::parse::ParseLayer::Kind::Id) {
            addIdParams();
            break;            
        }
    }

    return result;
}

GenElem::Type GenLayer::elemTypeImpl() const
{
    return Type_Layer;
}

bool GenLayer::prepareImpl()
{
    return true;
}

bool GenLayer::writeImpl() const
{
    return true;
}

bool GenLayer::forceCommsOrderImpl([[maybe_unused]] LayersAccessList& layers, bool& success) const
{
    success = true;
    return false;
}

} // namespace gen

} // namespace commsdsl
