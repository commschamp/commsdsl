//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/CustomLayer.h"
#include "commsdsl/gen/Generator.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace gen
{

CustomLayer::CustomLayer(Generator& generator, commsdsl::parse::Layer dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Custom);
}

CustomLayer::~CustomLayer() = default;

commsdsl::parse::CustomLayer CustomLayer::customDslObj() const
{
    return commsdsl::parse::CustomLayer(dslObj());
}

bool CustomLayer::forceCommsOrderImpl(LayersAccessList& layers, bool& success) const
{
    auto obj = customDslObj();
    if (obj.semanticLayerType() != commsdsl::parse::Layer::Kind::Checksum) {
        success = true;
        return false;
    }

    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [this](const auto* l)
            {
                return l == this;
            });

    if (iter == layers.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        success = false;
        return false;
    }    

    auto& untilStr = obj.checksumUntilLayer();
    if (!untilStr.empty()) {
        assert(obj.checksumFromLayer().empty());
        auto untilIter =
            std::find_if(
                layers.begin(), layers.end(),
                [&untilStr](const auto* l)
                {
                    return l->dslObj().name() == untilStr;
                });

        if (untilIter == layers.end()) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            success = false;
            return false;
        }

        if ((*untilIter)->dslObj().kind() != commsdsl::parse::Layer::Kind::Payload) {
            generator().logger().error("Custom checksum prefix must be until payload layer");
            success = false;
            return false;
        }

        success = true;
        return false;
    }   

    auto& fromStr = obj.checksumFromLayer();
    if (fromStr.empty()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        generator().logger().error("Info on custom checksum layer is missing");
        success = false;
        return false;
    }

    auto fromIter =
        std::find_if(
            layers.begin(), layers.end(),
            [&fromStr](const auto* l)
            {
                return l->dslObj().name() == fromStr;
            });


    if (fromIter == layers.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        success = false;
        return false;
    }            

    auto iterTmp = iter;
    std::advance(iterTmp, 1U);
    if (iterTmp == fromIter) {
        // Already in place
        success = true;
        return false;
    }

    auto thisPtr = std::move(*iter);
    layers.erase(iter);
    layers.insert(fromIter, std::move(thisPtr));
    success = true;
    return true;         

}

} // namespace gen

} // namespace commsdsl
