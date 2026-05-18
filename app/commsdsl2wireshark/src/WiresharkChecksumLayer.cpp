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

#include "WiresharkChecksumLayer.h"

#include "Wireshark.h"
#include "WiresharkField.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <sstream>
#include <type_traits>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

namespace
{

const std::string WiresharkChecksumCalcSuffix = "_checksum_calc_func";

} // namespace

WiresharkChecksumLayer::WiresharkChecksumLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkChecksumLayer::wiresharkDissectBodyImpl() const
{
    if (!genChecksumParseObj().parseUntilLayer().empty()) {
        return wiresharkPrefixDissectBodyInternal();
    }

    return wiresharkSuffixDissectBodyInternal();
}

std::string WiresharkChecksumLayer::wiresharkExtraDissectCodeImpl() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(*this, WiresharkChecksumCalcSuffix);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    static const std::string Templ =
        "#^#NAME#$##^#SUFFIX#$# =\n"
        "    #^#REPLACE#$#\n"
        "    #^#VALUE#$#\n"
        "#^#EXTEND#$#\n"
        ;

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"NAME", wiresharkChecksumFuncNameInternal()},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace checksum calculation algorithm", &replaced)},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend checksum calculation algorithm", &replaced)},
    };

    if (!replaced) {
        auto parseObj = genChecksumParseObj();
        auto alg = parseObj.parseAlg();

        using Func = std::string (WiresharkChecksumLayer::*)() const;
        static const Func Map[] = {
            /* Custom */ &WiresharkChecksumLayer::wiresharkCustomChecksumExtraCodeInternal,
            /* Sum */ &WiresharkChecksumLayer::wiresharkSumChecksumExtraCodeInternal,
            /* Crc_CCITT */ &WiresharkChecksumLayer::wiresharkCrcCcittChecksumExtraCodeInternal,
            /* Crc_16 */ &WiresharkChecksumLayer::wiresharkCrc16ChecksumExtraCodeInternal,
            /* Crc_32 */ &WiresharkChecksumLayer::wiresharkCrc32ChecksumExtraCodeInternal,
            /* Xor */ &WiresharkChecksumLayer::wiresharkXorChecksumExtraCodeInternal,
        };
        static const std::size_t MapSize = std::extent_v<decltype(Map)>;
        static_assert(MapSize == static_cast<std::size_t>(ParseChecksumLayer::ParseAlg::NumOfValues));

        auto idx = static_cast<unsigned>(alg);
        if (MapSize <= idx) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return strings::genEmptyString();
        }

        auto func = Map[idx];
        repl["VALUE"] = (this->*func)();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkChecksumLayer::wiresharkNeedsCrcCalcImpl() const
{
    auto parseObj = genChecksumParseObj();
    auto alg = parseObj.parseAlg();

    using ParseAlg = ParseChecksumLayer::ParseAlg;
    static const ParseAlg CrcAlgs[] = {
        ParseAlg::Custom,
        ParseAlg::Crc_CCITT,
        ParseAlg::Crc_16,
        ParseAlg::Crc_32,
    };

    auto iter = std::find(std::begin(CrcAlgs), std::end(CrcAlgs), alg);
    return iter != std::end(CrcAlgs);
}

std::string WiresharkChecksumLayer::wiresharkCustomChecksumExtraCodeInternal() const
{
    return strings::genNilStr() + " -- !!! TODO: Provide custom algorithm via code injection here !!!";
}

std::string WiresharkChecksumLayer::wiresharkSumChecksumExtraCodeInternal() const
{
    static const std::string Templ =
        "function(#^#TVB#$#, #^#OFFSET#$#, #^#LIMIT#$#)\n"
        "    local sum = 0\n"
        "    local length = #^#LIMIT#$# - #^#OFFSET#$#\n"
        "    if length <= 0 then\n"
        "        return 0\n"
        "    end\n"
        "\n"
        "    local data = #^#TVB#$#:range(#^#OFFSET#$#, length):raw()\n"
        "    for i = 1, #data do\n"
        "        sum = sum + data:byte(i)\n"
        "    end\n"
        "    return bit32.band(sum, #^#MASK#$#)\n"
        "end\n"
        ;

    auto* field = wiresharkField();
    assert(field != nullptr);
    auto shift = field->wiresharkMaxFieldLength() * std::numeric_limits<std::uint8_t>::digits;
    std::string maskStr = "0xffffffffffff";
    if (shift < std::numeric_limits<std::uintmax_t>::digits) {
        auto mask = static_cast<std::uintmax_t>((std::uintmax_t(1) << shift) - 1U);
        std::stringstream stream;
        stream << "0x" << std::hex << mask;
        maskStr = stream.str();
    }

    util::GenReplacementMap repl = {
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
        {"MASK", std::move(maskStr)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkChecksumLayer::wiresharkCrcCcittChecksumExtraCodeInternal() const
{
    static const std::string Templ =
        "#^#FUNC#$#(16, 0x1021, 0xFFFF, false, false, 0x0000)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"FUNC", Wireshark::wiresharkCreateCrcFuncName(wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkChecksumLayer::wiresharkCrc16ChecksumExtraCodeInternal() const
{
    static const std::string Templ =
        "#^#FUNC#$#(16, 0x8005, 0x0000, true, true, 0x0000)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"FUNC", Wireshark::wiresharkCreateCrcFuncName(wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkChecksumLayer::wiresharkCrc32ChecksumExtraCodeInternal() const
{
    static const std::string Templ =
        "#^#FUNC#$#(32, 0x04C11DB7, 0xFFFFFFFF, true, true, 0xFFFFFFFF)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"FUNC", Wireshark::wiresharkCreateCrcFuncName(wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkChecksumLayer::wiresharkXorChecksumExtraCodeInternal() const
{
    static const std::string Templ =
        "function (#^#TVB#$#, #^#OFFSET#$#, #^#LIMIT#$#)\n"
        "    local xor_val = 0\n"
        "    local length = #^#LIMIT#$# - #^#OFFSET#$#\n"
        "    if length <= 0 then\n"
        "        return 0\n"
        "    end\n"
        "\n"
        "    local data = #^#TVB#$#:range(#^#OFFSET#$#, length):raw()\n"
        "    for i = 1, #data do\n"
        "        xor_val = bit32.bxor(xor_val, data:byte(i))\n"
        "    end\n"
        "    return xor_val\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkChecksumLayer::wiresharkChecksumFuncNameInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkFuncNameFor(*this, WiresharkChecksumCalcSuffix);
}

std::string WiresharkChecksumLayer::wiresharkPrefixDissectBodyInternal() const
{
    // TODO:
    return "-- TODO: implement checksum prefix";
}

std::string WiresharkChecksumLayer::wiresharkSuffixDissectBodyInternal() const
{
    auto parseObj = genChecksumParseObj();
    if (parseObj.parseVerifyBeforeRead()) {
        return wiresharkSuffixVerifyFirstDissectBodyInternal();
    }

    static const std::string Templ =
        "local from_offset = #^#OFFSET#$#\n"
        "local orig_limit = #^#LIMIT#$#\n"
        "#^#LIMIT#$# = #^#LIMIT#$# - #^#FIELD_LEN#$#\n"
        "#^#NEXT#$#\n"
        "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
        "    return #^#RESULT#$#\n"
        "end\n"
        "\n"
        "local until_offset = #^#NEXT_OFFSET#$#\n"
        "#^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
        "#^#LIMIT#$# = orig_limit\n"
        "#^#FIELD#$#\n"
        "local checksum = #^#CALC#$#(#^#TVB#$#, from_offset, until_offset)\n"
        "if #^#VALUE_FUNC#$#() ~= checksum then\n"
        "   #^#TREE#$#:add_expert_info(PI_CHECKSUM, PI_WARN, \"Checksum Error\")\n"
        "   return #^#ERROR#$#, #^#NEXT_OFFSET#$#\n"
        "end\n"
    ;

    auto* field = wiresharkField();
    assert(field != nullptr);
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"NEXT", wiresharkNextFuncCode()},
        {"RESULT", WiresharkField::wiresharkResultStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::ChecksumError)},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"FIELD", wiresharkDissectFieldCode()},
        {"VALUE_FUNC", field->wiresharkValueFuncName()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
        {"CALC", wiresharkChecksumFuncNameInternal()},
        {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
        {"FIELD_LEN", std::to_string(field->wiresharkMinFieldLength())},
        {"TREE", WiresharkField::wiresharkTreeStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkChecksumLayer::wiresharkSuffixVerifyFirstDissectBodyInternal() const
{
    // TODO:
    return "-- TODO: implement checksum suffix with verification first";
}

} // namespace commsdsl2wireshark
