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

#include "WiresharkValueLayer.h"

#include "Wireshark.h"
#include "WiresharkField.h"
#include "WiresharkFrame.h"
#include "WiresharkGenerator.h"
#include "WiresharkInterface.h"
#include "WiresharkNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkValueLayer::WiresharkValueLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkValueLayer::wiresharkDissectBodyImpl() const
{
    auto parseObj = genValueLayerParseObj();
    if (parseObj.parsePseudo()) {
        return wiresharkNextFuncCode();
    }

    static const std::string Templ =
        "#^#FIELD#$#\n"
        "#^#INTERFACE_READ#$#\n"
        "#^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
        "#^#NEXT#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"FIELD", wiresharkDissectFieldCode()},
        {"INTERFACE_READ", wiresharkInterfaceReadCodeInternal()},
        {"NEXT", wiresharkNextFuncCode()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkValueLayer::wiresharkIsInterfaceSupportedImpl(const WiresharkInterface& iFace) const
{
    return genIsInterfaceSupported(&iFace);
}

std::string WiresharkValueLayer::wiresharkInterfaceReadCodeInternal() const
{
    static const std::string Templ =
        "local interface_tree = #^#TREE#$#:add(#^#PROTO#$#, #^#TVB#$#(#^#OFFSET#$#, #^#LIMIT#$# - #^#OFFSET#$#))\n"
        "interface_tree:set_hidden(true)\n"
        "#^#RESULT#$#, _ = #^#DISSECT#$#(#^#TVB#$#, interface_tree, #^#OFFSET#$#, #^#NEXT_OFFSET#$#)\n"
        "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
        "    return #^#RESULT#$#, #^#NEXT_OFFSET#$#\n"
        "end\n"
        ;

    auto* parentFrame = genParentFrame();
    assert(parentFrame != nullptr);
    auto* parentNs = parentFrame->genParentNamespace();
    assert(parentNs != nullptr);
    auto* iFace = WiresharkNamespace::wiresharkCast(parentNs)->wiresharkInterface();
    assert(iFace != nullptr);
    auto* iFaceField = iFace->wiresharkFindField(genValueLayerParseObj().parseFieldName());
    assert(iFaceField != nullptr);

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"PROTO", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
        {"TREE", WiresharkField::wiresharkTreeStr()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
        {"RESULT", WiresharkField::wiresharkResultStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"DISSECT", iFaceField->wiresharkDissectName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
