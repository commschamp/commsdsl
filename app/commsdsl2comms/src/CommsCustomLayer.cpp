//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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
