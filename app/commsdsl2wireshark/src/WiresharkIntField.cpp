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

#include "WiresharkIntField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <map>
#include <utility>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkIntField::WiresharkIntField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

const std::string& WiresharkIntField::wiresharkIntegralType(ParseIntField::ParseType type, std::size_t len)
{
    using ParseType = ParseIntField::ParseType;
    using Key = std::pair<ParseIntField::ParseType, std::size_t>;
    static const std::map<Key, std::string> Map = {
        {{ParseType::Int8, 1U}, "int8"},
        {{ParseType::Uint8, 1U}, "uint8"},
        {{ParseType::Int16, 1U}, "int8"},
        {{ParseType::Int16, 2U}, "int16"},
        {{ParseType::Uint16, 1U}, "uint8"},
        {{ParseType::Uint16, 2U}, "uint16"},
        {{ParseType::Int32, 1U}, "int8"},
        {{ParseType::Int32, 2U}, "int16"},
        {{ParseType::Int32, 3U}, "int24"},
        {{ParseType::Int32, 4U}, "int32"},
        {{ParseType::Uint32, 1U}, "uint8"},
        {{ParseType::Uint32, 2U}, "uint16"},
        {{ParseType::Uint32, 3U}, "uint24"},
        {{ParseType::Uint32, 4U}, "uint32"},
        {{ParseType::Int64, 1U}, "int8"},
        {{ParseType::Int64, 2U}, "int16"},
        {{ParseType::Int64, 3U}, "int24"},
        {{ParseType::Int64, 4U}, "int32"},
        {{ParseType::Int64, 5U}, "int64"},
        {{ParseType::Int64, 6U}, "int64"},
        {{ParseType::Int64, 7U}, "int64"},
        {{ParseType::Int64, 8U}, "int64"},
        {{ParseType::Uint64, 1U}, "uint8"},
        {{ParseType::Uint64, 2U}, "uint16"},
        {{ParseType::Uint64, 3U}, "uint24"},
        {{ParseType::Uint64, 4U}, "uint32"},
        {{ParseType::Uint64, 5U}, "uint64"},
        {{ParseType::Uint64, 6U}, "uint64"},
        {{ParseType::Uint64, 7U}, "uint64"},
        {{ParseType::Uint64, 8U}, "uint64"},
        {{ParseType::Intvar, 1U}, "int8"},
        {{ParseType::Intvar, 2U}, "int16"},
        {{ParseType::Intvar, 3U}, "int24"},
        {{ParseType::Intvar, 4U}, "int32"},
        {{ParseType::Intvar, 5U}, "int64"},
        {{ParseType::Intvar, 6U}, "int64"},
        {{ParseType::Intvar, 7U}, "int64"},
        {{ParseType::Intvar, 8U}, "int64"},
        {{ParseType::Uintvar, 1U}, "uint8"},
        {{ParseType::Uintvar, 2U}, "uint16"},
        {{ParseType::Uintvar, 3U}, "uint24"},
        {{ParseType::Uintvar, 4U}, "uint32"},
        {{ParseType::Uintvar, 5U}, "uint64"},
        {{ParseType::Uintvar, 6U}, "uint64"},
        {{ParseType::Uintvar, 7U}, "uint64"},
        {{ParseType::Uintvar, 8U}, "uint64"},
    };

    auto iter = Map.find(std::make_pair(type, len));
    if (iter == Map.end()) {
        return strings::genEmptyString();
    }

    return iter->second;
}

std::string WiresharkIntField::wiresharkFieldRegistrationImpl() const
{
    static const std::string Templ =
        "#^#SPECIALS#$#\n"
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", \"#^#DISP_NAME#$#\", base.DEC_HEX#^#SUFFIX#$#))\n"
    ;

    auto obj = genIntFieldParseObj();
    util::GenReplacementMap repl = {
        {"SPECIALS", wiresharkSpecialsInternal()},
        {"OBJ_NAME", wiresharkFieldObjName()},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkIntegralType(obj.parseType(), obj.parseMaxLength())},
        {"REF_NAME", wiresharkFieldRefName()},
        {"DISP_NAME", util::genDisplayName(obj.parseDisplayName(), obj.parseName())},
    };

    if (!repl["SPECIALS"].empty()) {
        repl["SUFFIX"] += ", " + wiresharkFieldObjName() + strings::genSpecialsSuffixStr();
    }

    if (!obj.parseDescription().empty()) {
        repl["SUFFIX"] += ", \"" + obj.parseDescription() + "\"";
    }

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkIntField::wiresharkSpecialsInternal() const
{
    // TODO
    return std::string();
}

} // namespace commsdsl2wireshark
