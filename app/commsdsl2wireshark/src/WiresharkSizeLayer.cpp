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

#include "WiresharkSizeLayer.h"

#include "Wireshark.h"
#include "WiresharkField.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"

#include <cassert>

namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkSizeLayer::WiresharkSizeLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkSizeLayer::wiresharkDissectBodyImpl() const
{
    static const std::string Templ =
        "#^#FIELD#$#\n"
        "local next_limit = #^#NEXT_OFFSET#$# + #^#VALUE_FUNC#$#()\n"
        "if #^#LIMIT#$# < next_limit then\n"
        "    return #^#NOT_ENOUGH_DATA#$#, #^#OFFSET#$#\n"
        "end\n"
        "\n"
        "#^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
        "#^#LIMIT#$# = next_limit\n"
        "#^#NEXT#$#\n"
        "if #^#RESULT#$# == #^#NOT_ENOUGH_DATA#$# then\n"
        "    return #^#MALFORMED#$#, #^#LIMIT#$#\n"
        "end\n"
        ;

    auto* field = wiresharkField();
    assert(field != nullptr);

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"FIELD", wiresharkDissectFieldCode()},
        {"NEXT", wiresharkNextFuncCode()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"VALUE_FUNC", field->wiresharkValueFuncName()},
        {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
        {"MALFORMED", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::MalformedPacket)},
        {"RESULT", WiresharkField::wiresharkResultStr()},
        {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
