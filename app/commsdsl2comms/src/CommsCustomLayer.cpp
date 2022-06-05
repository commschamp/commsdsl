//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsCustomLayer.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"

#include "CommsGenerator.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsCustomLayer::CommsCustomLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsCustomLayer::prepareImpl()
{
    return Base::prepareImpl() && CommsBase::commsPrepare();
}

bool CommsCustomLayer::commsReorderImpl(CommsLayersList& siblings, bool& success) const
{
    auto obj = customDslObj();
    if (obj.semanticLayerType() != commsdsl::parse::Layer::Kind::Checksum) {
        return CommsBase::commsReorderImpl(siblings, success);
    }

    auto iter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [this](const CommsLayer* l)
            {
                return l == this;
            });

    if (iter == siblings.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return false;
    }    

    auto& gen = generator();
    auto& untilStr = obj.checksumUntilLayer();
    if (!untilStr.empty()) {
        assert(obj.checksumFromLayer().empty());
        auto untilIter =
            std::find_if(
                siblings.begin(), siblings.end(),
                [&untilStr](const CommsLayer* l)
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
            gen.logger().error("Custom checksum prefix must be until payload layer");
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
        gen.logger().error("Info on custom checksum layer is missing");
        success = false;
        return false;
    }

    auto fromIter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [&fromStr](const CommsLayer* l)
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

CommsCustomLayer::IncludesList CommsCustomLayer::commsDefIncludesImpl() const
{
    IncludesList result = {
        comms::relHeaderForLayer(comms::className(dslObj().name()), generator())
    };

    return result;
}

std::string CommsCustomLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static const std::string Templ = 
        "#^#CUSTOM_LAYER_TYPE#$#<\n"
        "    #^#FIELD_TYPE#$#,\n"
        "    #^#ID_TEMPLATE_PARAMS#$#\n"
        "    #^#PREV_LAYER#$##^#COMMA#$#\n"
        "    #^#EXTRA_OPT#$#\n"
        ">";

    util::ReplacementMap repl = {
        {"CUSTOM_LAYER_TYPE", comms::scopeForCustomLayer(*this, generator())},
        {"FIELD_TYPE", commsDefFieldType()},
        {"PREV_LAYER", prevName},
        {"EXTRA_OPT", commsDefExtraOpts()},
    };

    if (customDslObj().semanticLayerType() == commsdsl::parse::Layer::Kind::Id) {
        repl["ID_TEMPLATE_PARAMS"] = "TMessage,\nTAllMessages,";
    }

    if (!repl["EXTRA_OPT"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

bool CommsCustomLayer::commsDefHasInputMessagesImpl() const
{
    return (customDslObj().semanticLayerType() == commsdsl::parse::Layer::Kind::Id);
}

bool CommsCustomLayer::commsIsCustomizableImpl() const
{
    return true;
}

} // namespace commsdsl2comms
