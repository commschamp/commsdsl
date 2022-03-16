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

#include "CommsPayloadLayer.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include "CommsGenerator.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2new
{

CommsPayloadLayer::CommsPayloadLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsPayloadLayer::prepareImpl()
{
    return Base::prepareImpl() && CommsBase::commsPrepare();
}

CommsPayloadLayer::IncludesList CommsPayloadLayer::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/protocol/MsgDataLayer.h"
    };

    return result;
}

std::string CommsPayloadLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static_cast<void>(prevName);
    assert(prevName.empty());

    static const std::string Templ =
        "comms::protocol::MsgDataLayer<\n"
        "    #^#EXTRA_OPT#$#\n"
        ">";
    
    util::ReplacementMap repl {
        {"EXTRA_OPT", commsDefExtraOpts()}
    };
    return util::processTemplate(Templ, repl);    
}

bool CommsPayloadLayer::commsIsCustomizableImpl() const
{
    return true;
}

CommsPayloadLayer::StringsList CommsPayloadLayer::commsExtraDataViewDefaultOptionsImpl() const
{
    return
        StringsList{
            "comms::option::app::OrigDataView"
        };    
}

CommsPayloadLayer::StringsList CommsPayloadLayer::commsExtraBareMetalDefaultOptionsImpl() const
{
    return
        StringsList{
            "comms::option::app::FixedSizeStorage<DEFAULT_SEQ_FIXED_STORAGE_SIZE * 8>"
        };    
}

} // namespace commsdsl2new
