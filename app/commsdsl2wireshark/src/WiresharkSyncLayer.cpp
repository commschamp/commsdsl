//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "WiresharkSyncLayer.h"

#include "WiresharkField.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkSyncLayer::WiresharkSyncLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkSyncLayer::wiresharkDissectBodyImpl() const
{
    auto parseObj = genSyncLayerParseObj();
    if (parseObj.parseIsAfterPayload()) {
        return wiresharkSuffixDissectCodeInternal();
    }

    static const std::string Templ =
        "#^#FIELD#$#\n"
        "#^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
        "#^#NEXT#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"FIELD", wiresharkDissectFieldCodeInternal()},
        {"NEXT", wiresharkNextFuncCode()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSyncLayer::wiresharkSuffixDissectCodeInternal() const
{
    // TODO
    return "-- TODO: implement sync suffix";
}

std::string WiresharkSyncLayer::wiresharkDissectFieldCodeInternal() const
{
    auto parseObj = genSyncLayerParseObj();
    if (!parseObj.parseSeekField()) {
        return WiresharkLayer::wiresharkDissectFieldCode();
    }

    // TODO
    return "-- TODO: implement seeked sync suffix";
}

} // namespace commsdsl2wireshark
