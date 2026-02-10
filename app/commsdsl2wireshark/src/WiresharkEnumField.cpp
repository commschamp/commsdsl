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

#include "WiresharkEnumField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"
#include "WiresharkIntField.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkEnumField::WiresharkEnumField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkEnumField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#VALS#$#\n"
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", \"#^#DISP_NAME#$#\", #^#BASE#$#, #^#VALS_NAME#$#, #^#MASK#$#, #^#DESC#$#))\n"
    ;

    auto obj = genEnumFieldParseObj();
    util::GenReplacementMap repl = {
        {"VALS", wiresharkValsInternal(refField)},
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkForcedIntegralFieldType(refField)},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldDisplayNameStr(refField)},
        {"VALS_NAME", wiresharkFieldObjName(refField) + strings::genValsSuffixStr()},
        {"BASE", "base.DEC_HEX"},
        {"MASK", wiresharkForcedIntegralFieldMask(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    if (obj.parseHexAssign()) {
        repl["BASE"] = "base.HEX_DEC";
    }

    if (repl["TYPE"].empty()) {
        repl["TYPE"] = WiresharkIntField::wiresharkIntegralType(obj.parseType(), obj.parseMaxLength());
    }
    else if (!genIsUnsignedType() && (repl["TYPE"].front() == 'u')) {
        repl["TYPE"] = repl["TYPE"].substr(1U);
    }

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkEnumField::wiresharkValsInternal(const WiresharkField* refField) const
{
    auto& values = genSortedRevValues();
    assert(!values.empty());

    auto actValues = genEnumFieldParseObj().parseValues();
    util::GenStringsList elems;
    for (auto& v : values) {
        assert(v.second != nullptr);
        auto iter = actValues.find(*v.second);
        if (iter == actValues.end()) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        static const std::string Templ =
            "[#^#VAL#$#] = \"#^#NAME#$#\"";

        util::GenReplacementMap repl = {
            {"NAME", util::genDisplayName(iter->second.m_displayName, iter->first)},
            {"VAL", std::to_string(v.first)},
        };

        if ((v.first < 0) && genIsUnsignedUnderlyingType()) {
            repl["VAL"] = std::to_string(static_cast<std::uintmax_t>(v.first));
        }

        elems.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "local #^#NAME#$##^#SUFFIX#$# = {\n"
        "    #^#ELEMS#$#\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldObjName(refField)},
        {"SUFFIX",  strings::genValsSuffixStr()},
        {"ELEMS", util::genStrListToString(elems, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
