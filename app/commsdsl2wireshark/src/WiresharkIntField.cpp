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

std::string WiresharkIntField::wiresharkTvbRangeAccessIntegralValue(ParseIntField::ParseType type, ParseEndian endian, std::size_t len)
{
    std::string prefix;
    if (endian == ParseEndian::ParseEndian_Little) {
        prefix = "le_";
    }

    if (GenIntField::genIsUnsignedType(type)) {
        prefix += 'u';
    }

    if (len <= 4) {
        return prefix + "int()";
    }

    return prefix + "int64()";
}

bool WiresharkIntField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }
    return true;
}

std::string WiresharkIntField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#SPECIALS#$#\n"
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, #^#BASE#$#\n, #^#SPECIALS_NAME#$#, #^#MASK#$#, #^#DESC#$#))\n"
    ;

    auto obj = genIntFieldParseObj();
    util::GenReplacementMap repl = {
        {"SPECIALS", wiresharkSpecialsInternal(refField)},
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkForcedIntegralFieldType(refField)},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"SPECIALS_NAME", strings::genNilStr()},
        {"MASK", wiresharkForcedIntegralFieldMask(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
        {"BASE", "base.DEC_HEX"},
    };

    if (!repl["SPECIALS"].empty()) {
        repl["SPECIALS_NAME"] = repl["OBJ_NAME"] + strings::genValsSuffixStr();
    }

    if (repl["TYPE"].empty()) {
        repl["TYPE"] = wiresharkIntegralType(obj.parseType(), obj.parseMaxLength());
    }
    else if (!genIsUnsignedType() && (repl["TYPE"].front() == 'u')) {
        repl["TYPE"] = repl["TYPE"].substr(1U);
    }

    if (obj.parseMinLength() == 3U) {
        // Cannot display hex value
        repl["BASE"] = "base.DEC";
    }

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkIntField::wiresharkTvbRangeAccessImpl() const
{
    auto obj = genIntFieldParseObj();
    return wiresharkTvbRangeAccessIntegralValue(obj.parseType(), obj.parseEndian(), obj.parseMaxLength());
}

std::string WiresharkIntField::wiresharkDissectLengthCheckImpl() const
{
    auto parseObj = genIntFieldParseObj();

    if (parseObj.parseAvailableLengthLimit()) {
        static const std::string Templ =
            "if offset == offset_limit then"
            "    return #^#ERROR#$#, offset\n"
            "end"
            ;

        auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
        util::GenReplacementMap repl = {
            {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::NotEnoughData)},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    return WiresharkBase::wiresharkDissectLengthCheckImpl();
}

std::string WiresharkIntField::wiresharkDissectBodyImpl() const
{
    static const std::string Templ =
        "local len = math.min(#^#LEN#$#, offset_limit - offset)\n"
        "#^#VAL_DECL#$#\n"
        "#^#VAR_LEN#$#\n"
        "#^#SCALING#$#\n"
        "tree:add#^#SUFFIX#$#(field, tvb(offset, len)#^#VAL#$#)\n"
        "result = #^#SUCCESS#$#\n"
        "next_offset = offset + len\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genIntFieldParseObj();
    util::GenReplacementMap repl = {
        {"LEN", std::to_string(parseObj.parseMaxLength())},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
        {"VAR_LEN", wiresharkVarLengthCodeInternal()}
    };

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        repl["SUFFIX"] = "_le";
    }

    if ((!repl["VAR_LEN"].empty()) || (!repl["SCALING"].empty())) {
        repl["VAL_DECL"] = "local val = 0";
        repl["VAL"] = ", val";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkIntField::wiresharkSpecialsInternal(const WiresharkField* refField) const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList elems;
    for (auto& s : specials) {
        if (!genGenerator().genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "[#^#VAL#$#] = \"#^#NAME#$#\"";

        util::GenReplacementMap repl = {
            {"NAME", util::genDisplayName(s.second.m_displayName, s.first)},
            {"VAL", std::to_string(s.second.m_value)},
        };

        if ((s.second.m_value < 0) && genIsUnsignedType()) {
            repl["VAL"] = std::to_string(static_cast<std::uintmax_t>(s.second.m_value));
        }

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

std::string WiresharkIntField::wiresharkVarLengthCodeInternal() const
{
    auto parseObj = genIntFieldParseObj();
    auto type = parseObj.parseType();
    if (!genIsVarLengthType(type)) {
        return strings::genEmptyString();
    }

    if (sizeof(std::uint32_t) < parseObj.parseMaxLength()) {
        return wiresharkVarLengthCodeLargeNumInternal();
    }

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        return wiresharkVarLengthCodeLittleEndianInternal();
    }

    return wiresharkVarLengthCodeBigEndianInternal();
}

std::string WiresharkIntField::wiresharkVarLengthCodeLargeNumInternal() const
{
    // TODO: Implement
    assert(false);
    return strings::genEmptyString();
}

std::string WiresharkIntField::wiresharkVarLengthCodeLittleEndianInternal() const
{
    // TODO: Implement
    assert(false);
    return strings::genEmptyString();
}

std::string WiresharkIntField::wiresharkVarLengthCodeBigEndianInternal() const
{
    static const std::string Templ =
        "local has_more = true\n"
        "while has_more and (next_offset < (offset + len)) do\n"
        "    local b = tvb(next_offset, 1):uint()\n"
        "    local data = bit32.band(b, 0x7F)\n"
        "    has_more = (bit32.band(b, 0x80) ~= 0)\n"
        "    val = bit32.bor(bit32.lshift(val, 7), data)\n"
        "    next_offset = next_offset + 1\n"
        "end\n"
        "\n"
        "if has_more then\n"
        "    return #^#ERROR#$#, offset\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::MalformedPacket)},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
