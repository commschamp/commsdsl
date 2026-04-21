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

#include "WiresharkIdLayer.h"

#include "WiresharkFrame.h"
#include "WiresharkGenerator.h"
#include "WiresharkMessage.h"
#include "WiresharkNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <map>
#include <vector>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkIdLayer::WiresharkIdLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkIdLayer::wiresharkDissectBodyImpl() const
{
    static const std::string Templ =
        "#^#FIELD#$#\n"
        "local id = tvb(offset, next_offset - offset):#^#VALUE#$#\n"
        "local msg = #^#MAP#$#[id]\n"
        "offset = next_offset\n"
        "#^#NEXT#$#\n"
        ;

    auto* field = wiresharkField();
    assert(field != nullptr);

    util::GenReplacementMap repl = {
        {"FIELD", wiresharkDissectFieldCode()},
        {"VALUE", field->wiresharkTvbRangeAccess()},
        {"NEXT", wiresharkNextFuncCode()},
        {"MAP", wiresharkMsgMapNameInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkIdLayer::wiresharkExtraDissectCodeImpl() const
{
    auto* parentFrame = genParentFrame();
    assert(parentFrame != nullptr);
    auto parentNs = parentFrame->genParentNamespace();
    assert(parentNs != nullptr);
    auto messages = parentNs->genGetAllMessagesIdSorted();

    using DissectMap = std::map<std::uintmax_t /*id */, util::GenStringsList /*dissect_funcs*/>;

    DissectMap map;
    for (auto* m : messages) {
        auto& wiresharkMsg = WiresharkMessage::wiresharkCast(*m);
        map[m->genParseObj().parseId()].push_back(wiresharkMsg.wiresharkDissectName());
    }

    util::GenStringsList elems;
    for (auto& info : map) {
        static const std::string ElemTempl =
            "[#^#ID#$#] = {#^#LIST#$#}"
            ;

        util::GenReplacementMap elemRepl = {
            {"ID", std::to_string(info.first)},
            {"LIST", util::genStrListToString(info.second, ", ", "")},
        };

        elems.push_back(util::genProcessTemplate(ElemTempl, elemRepl));
    }

    static const std::string Templ =
        "#^#NAME#$# = {\n"
        "    #^#ELEMS#$#\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkMsgMapNameInternal()},
        {"ELEMS", util::genStrListToString(elems, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkIdLayer::wiresharkMsgMapNameInternal() const
{
    return WiresharkGenerator::wiresharkCast(genGenerator()).wiresharkFuncNameFor(*this, "_msg");
}

} // namespace commsdsl2wireshark
