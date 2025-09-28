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

#include "CommsSyncLayer.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/util.h"

namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

CommsSyncLayer::CommsSyncLayer(CommsGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CommsBase(static_cast<GenBase&>(*this))
{
}

bool CommsSyncLayer::genPrepareImpl()
{
    bool result = GenBase::genPrepareImpl() && CommsBase::commsPrepare();
    if (!result) {
        return false;
    }

    commsSetForcedFailOnInvalidField();
    return true;

}

CommsSyncLayer::CommsIncludesList CommsSyncLayer::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/frame/SyncPrefixLayer.h"
    };

    return result;
}

std::string CommsSyncLayer::commsDefBaseTypeImpl(const std::string& prevName) const
{
    static const std::string Templ =
        "comms::frame::SyncPrefixLayer<\n"
        "    #^#FIELD_TYPE#$#,\n"
        "    #^#PREV_LAYER#$#\n"
        ">";

    util::GenReplacementMap repl = {
        {"FIELD_TYPE", commsDefFieldType()},
        {"PREV_LAYER", prevName}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2comms
