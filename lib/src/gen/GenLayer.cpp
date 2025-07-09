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
    using ParseLayer = GenLayer::ParseLayer;

    GenLayerImpl(GenGenerator& generator, const ParseLayer& parseObj, GenElem* parent) :
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        auto dslField = m_parseObj.parseField();
        if (!dslField.parseValid()) {
            if (m_parseObj.parseKind() != ParseLayer::ParseKind::Payload) {
                m_generator.genLogger().genError("GenLayer field definition is missing.");
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                return false;
            }

            return true;
        }

        auto extRef = dslField.parseExternalRef();
        if (!extRef.empty()) {
            m_externalField = m_generator.genFindField(extRef);
            assert(m_externalField != nullptr);
            m_externalField->genSetReferenced();
            return true;
        }

        m_memberField = GenField::genCreate(m_generator, dslField, m_parent);
        if (!m_memberField->genPrepare()) {
            return false;
        }

        m_memberField->genSetReferenced();
        return true;
    }

    bool genWrite()
    {
        if (m_memberField != nullptr) {
            return m_memberField->genWrite();
        }

        return true;
    }

    ParseLayer genParseObj() const
    {
        return m_parseObj;
    }


    GenField* genExternalField()
    {
        return m_externalField;
    }

    const GenField* genExternalField() const
    {
        return m_externalField;
    }

    GenField* genMemberField()
    {
        return m_memberField.get();
    }

    const GenField* genMemberField() const
    {
        return m_memberField.get();
    }        

    GenGenerator& genGenerator()
    {
        return m_generator;
    }

    const GenGenerator& genGenerator() const
    {
        return m_generator;
    }    

private:
    GenGenerator& m_generator;
    ParseLayer m_parseObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalField = nullptr;
    GenFieldPtr m_memberField;    
};

GenLayer::GenLayer(GenGenerator& generator, const ParseLayer& parseObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenLayerImpl>(generator, parseObj, this))
{
}

GenLayer::~GenLayer() = default;

GenLayer::GenPtr GenLayer::genCreate(GenGenerator& generator, ParseLayer dslobj, GenElem* parent)
{
    using CreateFunc = GenLayerPtr (GenGenerator::*)(ParseLayer dslobj, GenElem* parent);
    static const CreateFunc Map[] = {
        /* Custom */ &GenGenerator::genCreateCustomLayer,
        /* Sync */ &GenGenerator::genCreateSyncLayer,
        /* Size */ &GenGenerator::genCreateSizeLayer,
        /* Id */ &GenGenerator::genCreateIdLayer,
        /* Value */ &GenGenerator::genCreateValueLayer,
        /* Payload */ &GenGenerator::genCreatePayloadLayer,
        /* Checksum */ &GenGenerator::genCreateChecksumLayer,
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(ParseLayer::ParseKind::NumOfValues), "Invalid map");

    auto idx = static_cast<std::size_t>(dslobj.parseKind());
    if (MapSize <= idx) {
        [[maybe_unused]] static constexpr bool Unexpected_kind = false;
        assert(Unexpected_kind);          
        return GenPtr();
    }

    auto func = Map[idx];
    assert(func != nullptr); // NYI
    return (generator.*func)(dslobj, parent);
}

bool GenLayer::genPrepare()
{
    if (!m_impl->genPrepare()) {
        return false;
    }
    return genPrepareImpl();
}

bool GenLayer::genWrite() const
{
    return m_impl->genWrite() && genWriteImpl();
}

GenLayer::ParseLayer GenLayer::genParseObj() const
{
    return m_impl->genParseObj();
}

GenField* GenLayer::genExternalField()
{
    return m_impl->genExternalField();
}

const GenField* GenLayer::genExternalField() const
{
    return m_impl->genExternalField();
}

GenField* GenLayer::genMemberField()
{
    return m_impl->genMemberField();
}

const GenField* GenLayer::genMemberField() const
{
    return m_impl->genMemberField();
}

GenGenerator& GenLayer::genGenerator()
{
    return m_impl->genGenerator();
}

const GenGenerator& GenLayer::genGenerator() const
{
    return m_impl->genGenerator();
}

bool GenLayer::genForceCommsOrder(GenLayersAccessList& layers, bool& success) const
{
    return genForceCommsOrderImpl(layers, success);
}

std::string GenLayer::genTemplateScopeOfComms(const std::string& iFaceStr, const std::string& allMessagesStr, const std::string& protOptionsStr) const
{
    auto commsScope = comms::genScopeFor(*this, genGenerator());
    std::string optionsParams = "<" + protOptionsStr + ">";

    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == commsdsl::gen::GenElem::Type_Frame);

    auto optLevelScope = comms::genScopeFor(*parent, genGenerator()) + strings::genLayersSuffixStr();
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

            util::GenReplacementMap repl = {
                {"INTERFACE", iFaceStr},
                {"ALL_MESSAGES", allMessagesStr}
            };

            result += util::genProcessTemplate(Templ, repl);
        };

    for (auto iterTmp = iter; iterTmp != allLayers.end(); ++iterTmp) {
        auto kind = (*iterTmp)->genParseObj().parseKind();
        if (kind == ParseLayer::ParseKind::Id) {
            addIdParams();
            break;
        }

        if (kind != ParseLayer::ParseKind::Custom) {
            continue;
        }

        auto& customLayer = static_cast<const GenCustomLayer&>(**iterTmp);
        auto customKind = customLayer.genCustomLayerParseObj().parseSemanticLayerType();
        if (customKind == ParseLayer::ParseKind::Id) {
            addIdParams();
            break;            
        }
    }

    return result;
}

GenElem::Type GenLayer::genElemTypeImpl() const
{
    return Type_Layer;
}

bool GenLayer::genPrepareImpl()
{
    return true;
}

bool GenLayer::genWriteImpl() const
{
    return true;
}

bool GenLayer::genForceCommsOrderImpl([[maybe_unused]] GenLayersAccessList& layers, bool& success) const
{
    success = true;
    return false;
}

} // namespace gen

} // namespace commsdsl
