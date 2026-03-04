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

#include "WiresharkPayloadLayer.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkPayloadLayer::WiresharkPayloadLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkPayloadLayer::wiresharkDissectBodyImpl() const
{
    static const std::string Templ =
        "if not msg then\n"
        "    tree:add_expert_info(PI_MALFORMED, PI_WARN, \"Invalid message id\")\n"
        "    return #^#SUCCESS#$#, offset_limit\n"
        "end\n"
        "\n"
        "local result = #^#SUCCESS#$#\n"
        "local next_offset = offset_limit\n"
        "-- msg is a list of dissect functions\n"
        "for _, f in ipairs(msg) do\n"
        "    local data_subtree = tree:add(#^#FIELD#$#, tvb(offset, offset_limit - offset))\n"
        "    result, next_offset = f(tvb, data_subtree, offset, offset_limit)\n"
        "    if result == #^#SUCCESS#$# then\n"
        "        return result, next_offset\n"
        "    end\n"
        "\n"
        "    -- Do not show partially dissected malformed data\n"
        "    data_subtree:set_hidden(true)\n"
        "end\n"
        "tree:add_expert_info(PI_MALFORMED, PI_WARN, \"Invalid message data\")\n"
        "result, next_offset = #^#SUCCESS#$#, offset_limit\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"FIELD", wiresharkDissectFieldNameInternal()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkPayloadLayer::wiresharkExtraDissectCodeImpl() const
{
    static const std::string Templ =
        "local #^#NAME#$# = #^#CREATE_FUNC#$#(ProtoField.bytes(\"#^#REF_NAME#$#\", \"#^#DISP_NAME#$#\", base.SPACE, #^#DESC#$#))\n"
        ;

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"NAME", wiresharkDissectFieldNameInternal()},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"REF_NAME", wiresharkDissectFieldRefNameInternal()},
        {"DESC", strings::genNilStr()},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
    };

    if (!parseObj.parseDescription().empty()) {
        repl["DESC"] = parseObj.parseDescription();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkPayloadLayer::wiresharkDissectFieldNameInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkFuncNameFor(*this, "_field");
}

std::string WiresharkPayloadLayer::wiresharkDissectFieldRefNameInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto scope = comms::genScopeFor(*this, wiresharkGenerator, false);
    return Wireshark::wiresharkProtocolObjName(wiresharkGenerator) + '.' + util::genStrReplace(scope, "::", ".") + ".field";
}

} // namespace commsdsl2wireshark
