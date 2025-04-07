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

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include "CommsGenerator.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsIdLayer::CommsIdLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsIdLayer::prepareImpl()
{
    return Base::prepareImpl() && CommsBase::commsPrepare();
}

CommsIdLayer::IncludesList CommsIdLayer::commsDefIncludesImpl() const
{
    assert(getParent()->elemType() == commsdsl::gen::Elem::Type_Frame);
    auto& frame = *(static_cast<const commsdsl::gen::Frame*>(getParent()));
    assert(frame.getParent()->elemType() == commsdsl::gen::Elem::Type_Namespace);
    auto& ns = *(static_cast<const commsdsl::gen::Namespace*>(frame.getParent()));

    IncludesList result = {
        "comms/frame/MsgIdLayer.h",
        comms::relHeaderForInput(strings::allMessagesStr(), generator(), ns)
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

    util::ReplacementMap repl = {
        {"PREV_LAYER", prevName},
        {"FIELD_TYPE", commsDefFieldType()},
        {"EXTRA_OPTS", commsDefExtraOpts()},
    };

    if (!repl["EXTRA_OPTS"].empty()) {
        repl["COMMA"] = std::string(",");
    }
    return util::processTemplate(Templ, repl);
}

bool CommsIdLayer::commsDefHasInputMessagesImpl() const
{
    return true;
}

bool CommsIdLayer::commsIsCustomizableImpl() const
{
    return true;
}

CommsIdLayer::StringsList CommsIdLayer::commsExtraBareMetalDefaultOptionsImpl() const
{
    return
        StringsList{
            "comms::option::app::InPlaceAllocation"
        };    
}

CommsIdLayer::StringsList CommsIdLayer::commsExtraMsgFactoryDefaultOptionsImpl() const
{
    return
        StringsList{
            "comms::option::app::MsgFactoryTempl<" + commsMsgFactoryAliasInOptions(getParent()) + ">"
        };    
}

} // namespace commsdsl2comms
