//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsIdLayer.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsIdLayer::CommsIdLayer(CommsGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CommsBase(static_cast<GenBase&>(*this))
{
}

bool CommsIdLayer::genPrepareImpl()
{
    return GenBase::genPrepareImpl() && CommsBase::commsPrepare();
}

CommsIdLayer::CommsIncludesList CommsIdLayer::commsDefIncludesImpl() const
{
    assert(genGetParent()->genElemType() == commsdsl::gen::GenElem::GenType_Frame);
    auto& frame = *(static_cast<const commsdsl::gen::GenFrame*>(genGetParent()));
    assert(frame.genGetParent()->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto& ns = *(static_cast<const commsdsl::gen::GenNamespace*>(frame.genGetParent()));

    CommsIncludesList result = {
        "comms/frame/MsgIdLayer.h",
        comms::genRelHeaderForInput(strings::genAllMessagesStr(), genGenerator(), ns)
    };

    return result;
}

std::string CommsIdLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static const std::string Templ =
        "comms::frame::MsgIdLayer<\n"
        "    #^#FIELD_TYPE#$#,\n"
        "    TMessage,\n"
        "    TAllMessages,\n"
        "    #^#PREV_LAYER#$##^#COMMA#$#\n"
        "    #^#EXTRA_OPTS#$#\n"
        ">";

    util::GenReplacementMap repl = {
        {"PREV_LAYER", prevName},
        {"FIELD_TYPE", commsDefFieldType()},
        {"EXTRA_OPTS", commsDefExtraOpts()},
    };

    if (!repl["EXTRA_OPTS"].empty()) {
        repl["COMMA"] = std::string(",");
    }
    return util::genProcessTemplate(Templ, repl);
}

bool CommsIdLayer::commsDefHasInputMessagesImpl() const
{
    return true;
}

bool CommsIdLayer::commsIsCustomizableImpl() const
{
    return true;
}

CommsIdLayer::GenStringsList CommsIdLayer::commsExtraBareMetalDefaultOptionsImpl() const
{
    return
        GenStringsList{
            "comms::option::app::InPlaceAllocation"
        };
}

CommsIdLayer::GenStringsList CommsIdLayer::commsExtraMsgFactoryDefaultOptionsImpl() const
{
    return
        GenStringsList{
            "comms::option::app::MsgFactoryTempl<" + commsMsgFactoryAliasInOptions(genGetParent()) + ">"
        };
}

} // namespace commsdsl2comms
