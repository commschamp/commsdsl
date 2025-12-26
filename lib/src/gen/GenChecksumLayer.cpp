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

#include "commsdsl/gen/GenChecksumLayer.h"

#include "commsdsl/gen/GenGenerator.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace gen
{

GenChecksumLayer::GenChecksumLayer(GenGenerator& generator, ParseLayer dslObj, GenElem* parent) :
    Base(generator, dslObj, parent)
{
    assert(dslObj.parseKind() == ParseLayer::ParseKind::Checksum);
}

GenChecksumLayer::~GenChecksumLayer() = default;

bool GenChecksumLayer::genForceCommsOrderImpl(GenLayersAccessList& layers, bool& success) const
{
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

    auto obj = genChecksumDslObj();
    auto& untilStr = obj.parseUntilLayer();
    if (!untilStr.empty()) {
        assert(obj.parseFromLayer().empty());
        auto untilIter =
            std::find_if(
                layers.begin(), layers.end(),
                [&untilStr](const auto* l)
                {
                    return l->genParseObj().parseName() == untilStr;
                });

        if (untilIter == layers.end()) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            success = false;
            return false;
        }

        if ((*untilIter)->genParseObj().parseKind() != ParseLayer::ParseKind::Payload) {
            genGenerator().genLogger().genError("Checksum prefix must be until payload layer");
            success = false;
            return false;
        }

        success = true;
        return false;
    }

    genGenerator().genLogger().genDebug("Layer \"" + genParseObj().parseName() + "\" is a suffix expected to be relocated");
    auto& fromStr = obj.parseFromLayer();
    if (fromStr.empty()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        genGenerator().genLogger().genError("Info on checksum layer is missing");
        success = false;
        return false;
    }

    auto fromIter =
        std::find_if(
            layers.begin(), layers.end(),
            [&fromStr](const auto* l)
            {
                return l->genParseObj().parseName() == fromStr;
            });

    if (fromIter == layers.end()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        success = false;
        return false;
    }

    auto dist = std::distance(iter, fromIter);
    if (dist < 0) {
        // Not relocated yet
        genGenerator().genLogger().genDebug("Relocating \"" + genParseObj().parseName() + "\" to precede \"" + (*fromIter)->genParseObj().parseName() + "\"");
        auto* thisPtr = *iter;
        layers.erase(iter);
        layers.insert(fromIter, thisPtr);
    }

    // Already in place
    return genAdjustSuffixLayersOrder(layers, success);
}

GenChecksumLayer::ParseChecksumLayer GenChecksumLayer::genChecksumDslObj() const
{
    return ParseChecksumLayer(genParseObj());
}

} // namespace gen

} // namespace commsdsl
