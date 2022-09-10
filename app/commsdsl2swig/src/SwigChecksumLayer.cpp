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

#include "SwigChecksumLayer.h"

#include "SwigGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

SwigChecksumLayer::SwigChecksumLayer(SwigGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigChecksumLayer::swigReorderImpl(SwigLayersList& siblings, bool& success) const 
{
    auto iter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [this](const SwigLayer* l)
            {
                return l == this;
            });

    if (iter == siblings.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return false;
    }

    auto obj = checksumDslObj();
    auto& gen = generator();
    auto& untilStr = obj.untilLayer();
    if (!untilStr.empty()) {
        assert(obj.fromLayer().empty());
        auto untilIter =
            std::find_if(
                siblings.begin(), siblings.end(),
                [&untilStr](const SwigLayer* l)
                {
                    return l->layer().dslObj().name() == untilStr;
                });

        if (untilIter == siblings.end()) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            success = false;
            return false;
        }

        if ((*untilIter)->layer().dslObj().kind() != commsdsl::parse::Layer::Kind::Payload) {
            gen.logger().error("Checksum prefix must be until payload layer");
            success = false;
            return false;
        }

        success = true;
        return false;
    }

    auto& fromStr = obj.fromLayer();
    if (fromStr.empty()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        gen.logger().error("Info on checksum layer is missing");
        success = false;
        return false;
    }

    auto fromIter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [&fromStr](const SwigLayer* l)
            {
                return l->layer().dslObj().name() == fromStr;
            });


    if (fromIter == siblings.end()) {
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
    siblings.erase(iter);
    siblings.insert(fromIter, std::move(thisPtr));
    success = true;
    return true;      
}


} // namespace commsdsl2swig
