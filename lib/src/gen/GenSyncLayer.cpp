//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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

#include "commsdsl/gen/GenSyncLayer.h"

#include "commsdsl/gen/GenGenerator.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenSyncLayerImpl
{
public:
    using ParseSyncLayer = GenSyncLayer::ParseSyncLayer;

    GenSyncLayerImpl(GenGenerator& generator, ParseSyncLayer parseObj, GenElem* parent):
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        do {
            if (!m_parseObj.parseHasEscField()) {
                break;
            }

            auto escField = m_parseObj.parseEscField();
            assert(escField.parseValid());
            if (!escField.parseExternalRef().empty()) {
                m_externalEscField = m_generator.genFindField(escField.parseExternalRef());
                assert(m_externalEscField != nullptr);
                break;
            }

            m_memberEscField = GenField::genCreate(m_generator, escField, m_parent);
            if (!m_memberEscField->genPrepare()) {
                return false;
            }
        } while (false);

        return true;
    }

    GenField* genExternalEscField()
    {
        return m_externalEscField;
    }

    const GenField* genExternalEscField() const
    {
        return m_externalEscField;
    }

    GenField* genMemberEscField()
    {
        return m_memberEscField.get();
    }

    const GenField* genMemberEscField() const
    {
        return m_memberEscField.get();
    }

private:
    GenGenerator& m_generator;
    ParseSyncLayer m_parseObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalEscField = nullptr;
    GenFieldPtr m_memberEscField;

};

GenSyncLayer::GenSyncLayer(GenGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenSyncLayerImpl>(generator, genSyncLayerDslObj(), this))
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Sync);
}

GenSyncLayer::~GenSyncLayer() = default;

GenField* GenSyncLayer::genExternalEscField()
{
    return m_impl->genExternalEscField();
}

const GenField* GenSyncLayer::genExternalEscField() const
{
    return m_impl->genExternalEscField();
}

GenField* GenSyncLayer::genMemberEscField()
{
    return m_impl->genMemberEscField();
}

const GenField* GenSyncLayer::genMemberEscField() const
{
    return m_impl->genMemberEscField();
}

bool GenSyncLayer::genPrepareImpl()
{
    return m_impl->genPrepare();
}

bool GenSyncLayer::genForceCommsOrderImpl(GenLayersAccessList& layers, bool& success) const
{
    auto parseObj = genSyncLayerDslObj();
    if (!parseObj.parseIsAfterPayload()) {
        return Base::genForceCommsOrderImpl(layers, success);
    }

    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [this](const auto* l)
            {
                return l == this;
            });

    if (iter == layers.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        success = false;
        return false;
    }

    auto fromStr = parseObj.parseFromLayer();
    GenLayersAccessList::iterator nextIter = layers.end();
    if (fromStr.empty()) {
        nextIter =
            std::find_if(
                layers.begin(), layers.end(),
                [](const auto* l)
                {
                    return l->genParseObj().parseKind() == commsdsl::parse::ParseLayer::ParseKind::Payload;
                });

        if (nextIter == layers.end()) {
            genGenerator().genLogger().genError("<payload> layer is not found");
        }
    }
    else {
        nextIter =
            std::find_if(
                layers.begin(), layers.end(),
                [&fromStr](const auto* l)
                {
                    return l->genParseObj().parseName() == fromStr;
                });
    }

    if (nextIter == layers.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        success = false;
        return false;
    }

    auto dist = std::distance(iter, nextIter);
    if (dist < 0) {
        // Not relocated yet
        genGenerator().genLogger().genDebug("Relocating \"" + genParseObj().parseName() + "\" to precede \"" + (*nextIter)->genParseObj().parseName() + "\"");
        auto* thisPtr = *iter;
        layers.erase(iter);
        layers.insert(nextIter, thisPtr);
    }

    // Already in place
    return genAdjustSuffixLayersOrder(layers, success);
}

GenSyncLayer::ParseSyncLayer GenSyncLayer::genSyncLayerDslObj() const
{
    return ParseSyncLayer(genParseObj());
}

} // namespace gen

} // namespace commsdsl
