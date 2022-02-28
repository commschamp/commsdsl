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

#include "CommsValueLayer.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2new
{

CommsValueLayer::CommsValueLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsValueLayer::prepareImpl()
{
    bool result = Base::prepareImpl() && CommsBase::commsPrepare();
    if (!result) {
        return false;
    }

    auto obj = valueDslObj();
    if (obj.pseudo()) {
        commsSetForcedPseudoField();
    }

    return true;
}

CommsValueLayer::IncludesList CommsValueLayer::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/protocol/TransportValueLayer.h"
    };

    return result;
}

std::string CommsValueLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static const std::string Templ = 
        "comms::protocol::TransportValueLayer<\n"
        "    #^#FIELD_TYPE#$#,\n"
        "    #^#INTERFACE_FIELD_IDX#$#,\n"
        "    #^#PREV_LAYER#$##^#COMMA#$#\n"
        "    #^#EXTRA_OPTS#$#\n"
        ">";    

    util::ReplacementMap repl = {
        {"FIELD_TYPE", commsDefFieldType()},
        {"INTERFACE_FIELD_IDX", util::numToString(valueDslObj().fieldIdx())},
        {"PREV_LAYER", prevName},
        {"EXTRA_OPTS", commsDefExtraOpts()}
    };

    if (!repl["EXTRA_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

CommsValueLayer::StringsList CommsValueLayer::commsDefExtraOptsImpl() const
{
    StringsList result;
    auto obj = valueDslObj();
    if (obj.pseudo()) {
        result.push_back("comms::option::def::PseudoValue");
    }
    return result;
}


} // namespace commsdsl2new
