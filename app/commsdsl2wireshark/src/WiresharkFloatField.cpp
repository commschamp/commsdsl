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

#include "WiresharkFloatField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <cstddef>
#include <cmath>
#include <type_traits>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkFloatField::WiresharkFloatField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkFloatField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }
    return true;
}

std::string WiresharkFloatField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#SPECIALS#$#\n"
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, #^#SPECIALS_NAME#$#, #^#DESC#$#))\n"
    ;

    util::GenReplacementMap repl = {
        {"SPECIALS", wiresharkSpecialsInternal(refField)},
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkFloatTypeInternal()},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"SPECIALS_NAME", strings::genNilStr()},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    if (!repl["SPECIALS"].empty()) {
        repl["SPECIALS_NAME"] = repl["OBJ_NAME"] + strings::genValsSuffixStr();
    }

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFloatField::wiresharkSpecialsInternal(const WiresharkField* refField) const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList elems;
    for (auto& s : specials) {
        if (std::isnan(s.second.m_value)) {
            // Known limitation: the NaN comparing to NaN will return false, so it cannot really be found in map
            continue;
        }

        if (!genGenerator().genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "[#^#VAL#$#] = \"#^#NAME#$#\"";

        util::GenReplacementMap repl = {
            {"NAME", util::genDisplayName(s.second.m_displayName, s.first)},
            {"VAL", wiresharkSpecialValStrInternal(s.second.m_value)},
        };

        elems.push_back(util::genProcessTemplate(Templ, repl));
    }

    if (elems.empty()) {
        return strings::genEmptyString();
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

std::string WiresharkFloatField::wiresharkSpecialValStrInternal(double val) const
{
    assert(!std::isnan(val)); // Should be filtered out earlier

    if (!std::isinf(val)) {
        return std::to_string(val);
    }

    if (val < 0) {
        return "1/0";
    }

    return "-1/0";
}

const std::string& WiresharkFloatField::wiresharkFloatTypeInternal() const
{
    static const std::string Map[] = {
        /* Float */ "float",
        /* Double */ "double",
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(ParseFloatField::ParseType::NumOfValues));

    auto parseObj = genFloatFieldParseObj();
    auto idx = static_cast<unsigned>(parseObj.parseType());
    if (MapSize <= idx) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        idx = static_cast<decltype(idx)>(ParseFloatField::ParseType::Double);
    }

    return Map[idx];
}

} // namespace commsdsl2wireshark
