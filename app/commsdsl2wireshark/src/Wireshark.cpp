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

#include "Wireshark.h"

#include "WiresharkFrame.h"
#include "WiresharkGenerator.h"
#include "WiresharkSchema.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <type_traits>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

bool Wireshark::wiresharkWrite(const WiresharkGenerator& generator)
{
    Wireshark obj(generator);
    return obj.wiresharkWriteInternal();
}

std::string Wireshark::wiresharkFileName(const WiresharkGenerator& generator)
{
    return generator.genProtocolSchema().genMainNamespace() + ".lua";
}

const std::string& Wireshark::wiresharkProtocolObjName(const WiresharkGenerator& generator)
{
    return generator.genProtocolSchema().genMainNamespace();
}

std::string Wireshark::wiresharkCreateFieldFuncName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".createField";
}

std::string Wireshark::wiresharkCreateExtractorFuncName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".createExtractor";
}

std::string Wireshark::wiresharkFieldsListName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".fields_list";
}

std::string Wireshark::wiresharkExtractorsMapName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".extractors_map";
}

std::string Wireshark::wiresharkStatusCodeStr(const WiresharkGenerator& generator, WiresharkStatusCode code)
{
    Wireshark obj(generator);
    return obj.wiresharkStatusCodeNameInternal() + "." + wiresharkStatusCodeStrInternal(code);
}

std::string Wireshark::wiresharkOptModeStr(const WiresharkGenerator& generator, WiresharkOptMode code)
{
    Wireshark obj(generator);
    return obj.wiresharkOptModeNameInternal() + "." + wiresharkOptModeStrInternal(code);
}

std::string Wireshark::wiresharkOptModeValsName(const WiresharkGenerator& generator)
{
    Wireshark obj(generator);
    return obj.wiresharkOptModeNameInternal() + strings::genValsSuffixStr();
}

std::string Wireshark::wiresharkFieldValueFuncName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".field_value";
}

std::string Wireshark::wiresharkLocalNamespaceName(const WiresharkGenerator& generator)
{
    return wiresharkProtocolObjName(generator) + "_local";
}

std::string Wireshark::wiresharkProtVersionGetFuncName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".prot_version_get";
}

std::string Wireshark::wiresharkProtVersionSetFuncName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".prot_version_set";
}

std::string Wireshark::wiresharkPinfoName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".last_pinfo";
}

std::string Wireshark::wiresharkPacketIdFuncName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".packet_id";
}

std::string Wireshark::wiresharkCreateCrcFuncName(const WiresharkGenerator& generator)
{
    return wiresharkLocalNamespaceName(generator) + ".create_crc_calc";
}

bool Wireshark::wiresharkWriteInternal() const
{
    auto fileName = wiresharkFileName(m_wiresharkGenerator);
    auto filePath = util::genPathAddElem(m_wiresharkGenerator.genGetOutputDir(), fileName);

    m_wiresharkGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_wiresharkGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    do {
        const std::string Templ =
            "#^#GEN_COMMENT#$#\n"
            "#^#PROTOCOL#$#\n"
            "#^#LOCAL#$#\n"
            "#^#PINFO#$#\n"
            "#^#PROT_VERSION#$#\n"
            "#^#STATUS_CODE#$#\n"
            "#^#OPT_MODE#$#\n"
            "#^#CRC#$#\n"
            "#^#FIELDS_REG#$#\n"
            "#^#EXTRACTORS_DECL#$#\n"
            "#^#FIELD_VALUE_FUNC#$#\n"
            "#^#CODE#$#\n"
            "#^#DISSECT_FUNC#$#\n"
            "#^#NAME#$#.fields = #^#FIELDS_LIST#$#\n"
            "#^#EXTRACTORS_REG#$#\n"
            "\n"
            "return #^#NAME#$#\n"
            ;

        util::GenReplacementMap repl = {
            {"GEN_COMMENT", m_wiresharkGenerator.wiresharkFileGeneratedComment()},
            {"PROTOCOL", wiresharkProtocolDefInternal()},
            {"LOCAL", wiresharkLocalInternal()},
            {"FIELDS_REG", wiresharkFieldsRegistrationInternal()},
            {"DISSECT_FUNC", wiresharkDissectFuncInternal()},
            {"NAME", wiresharkProtocolObjName(m_wiresharkGenerator)},
            {"FIELDS_LIST", wiresharkFieldsListName(m_wiresharkGenerator)},
            {"CODE", wiresharkCodeInternal()},
            {"STATUS_CODE", wiresharkStatusCodeDefInternal()},
            {"OPT_MODE", wiresharkOptionalModeDefInternal()},
            {"EXTRACTORS_DECL", wiresharkExtractorsDeclInternal()},
            {"EXTRACTORS_REG", wiresharkExtractorsRegCodeInternal()},
            {"FIELD_VALUE_FUNC", wiresharkFieldValueFuncInternal()},
            {"PROT_VERSION", wiresharkProtocolVersionDefInternal()},
            {"PINFO", wiresharkPinfoDefInternal()},
            {"CRC", wiresharkCrcCodeDefInternal()},
        };

        auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
        stream << str;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        m_wiresharkGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Wireshark::wiresharkProtocolDefInternal() const
{
    const std::string Templ =
        "local #^#NAME#$# = Proto(\"#^#NAME#$#\", \"#^#DISP_NAME#$#\")\n"
        ;

    auto& nsName = wiresharkProtocolObjName(m_wiresharkGenerator);
    util::GenReplacementMap repl = {
        {"NAME", nsName},
        {"DISP_NAME", util::genDisplayName(m_wiresharkGenerator.genProtocolSchema().genParseObj().parseDisplayName(), nsName)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkLocalInternal() const
{
    const std::string Templ =
        "local #^#NAME#$# = {}\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkLocalNamespaceName(m_wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkDissectFuncInternal() const
{
    const std::string Templ =
        "-- Main Dissector Entry Point\n"
        "function #^#NAME#$#.dissector(tvb, pinfo, tree)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        ;

    bool bodyReplaced = false;
    auto replacePath = m_wiresharkGenerator.wiresharkInputDissectRelPathFor(wiresharkProtocolObjName(m_wiresharkGenerator) + ".dissector") + strings::genReplaceFileSuffixStr();
    auto replaceCode = m_wiresharkGenerator.genReadCodeInjectCode(replacePath, "Replace body", &bodyReplaced);

    util::GenReplacementMap repl = {
        {"NAME", wiresharkProtocolObjName(m_wiresharkGenerator)},
        {"REPLACE", std::move(replaceCode)},
    };

    if (!bodyReplaced) {
        repl["BODY"] = wiresharkDissectFuncBodyInternal();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkFieldsRegistrationInternal() const
{
    const std::string Templ =
        "-- Field Management\n"
        "#^#LIST#$# = {}\n"
        "\n"
        "-- Invoke this function every time the field is created\n"
        "function #^#NAME#$#(obj)\n"
        "    table.insert(#^#LIST#$#, obj)\n"
        "    return obj\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"LIST", wiresharkFieldsListName(m_wiresharkGenerator)},
        {"NAME", wiresharkCreateFieldFuncName(m_wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkCodeInternal() const
{
    util::GenStringsList elems;
    for (auto& sPtr : m_wiresharkGenerator.genSchemas()) {
        auto str = WiresharkSchema::wiresharkCast(*sPtr).wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "\n", "");
}

std::string Wireshark::wiresharkDissectFuncBodyInternal() const
{
    util::GenStringsList elems;
    auto frames = m_wiresharkGenerator.genProtocolSchema().genGetAllFrames();
    for (auto* f : frames) {
        assert(f != nullptr);
        auto& wiresharkFrame = WiresharkFrame::wiresharkCast(*f);
        if (!wiresharkFrame.wiresharkValidFrame()) {
            continue;
        }

        static const std::string FrameTempl =
            "result, next_offset = #^#NAME#$#(tvb, tree)\n"
            "if result == #^#SUCCESS#$# then\n"
            "    break\n"
            "end\n"
        ;

        util::GenReplacementMap frameRepl = {
            {"NAME", wiresharkFrame.wiresharkDissectName()},
            {"SUCCESS", wiresharkStatusCodeStr(m_wiresharkGenerator, WiresharkStatusCode::Success)},
        };

        elems.push_back(util::genProcessTemplate(FrameTempl, frameRepl));
    }

    const std::string Templ =
        "if tvb:len() == 0 then\n"
        "    return\n"
        "end\n"
        "\n"
        "pinfo.cols.protocol = #^#NAME#$#.name\n"
        "#^#PINFO#$# = pinfo\n"
        "\n"
        "local result = #^#SUCCESS#$#\n"
        "local next_offset = 0\n"
        "repeat\n"
        "    #^#FRAMES#$#\n"
        "until true\n"
        "\n"
        "if (result ~= #^#NOT_ENOUGH_DATA#$#) and (result ~= #^#SUCCESS#$#) then\n"
        "    -- Consume everything\n"
        "    tree:add_expert_info(PI_MALFORMED, PI_WARN, \"Invalid protocol data\")\n"
        "    return\n"
        "end\n"
        "\n"
        "if next_offset < tvb:len() then\n"
        "    pinfo.desegment_offset = next_offset\n"
        "    pinfo.desegment_len = DESEGMENT_ONE_MORE_SEGMENT\n"
        "end\n"
    ;

    util:: GenReplacementMap repl = {
        {"NAME", wiresharkProtocolObjName(m_wiresharkGenerator)},
        {"FRAMES", util::genStrListToString(elems, "\n", "")},
        {"SUCCESS", wiresharkStatusCodeStr(m_wiresharkGenerator, WiresharkStatusCode::Success)},
        {"NOT_ENOUGH_DATA", wiresharkStatusCodeStr(m_wiresharkGenerator, WiresharkStatusCode::NotEnoughData)},
        {"PINFO", wiresharkPinfoName(m_wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkStatusCodeNameInternal() const
{
    return wiresharkLocalNamespaceName(m_wiresharkGenerator) + ".StatusCode";
}

std::string Wireshark::wiresharkStatusCodeDefInternal() const
{
    util::GenStringsList vals;

    for (auto idx = 0U; idx < static_cast<unsigned>(WiresharkStatusCode::ValuesLimit); ++idx) {
        vals.push_back(wiresharkStatusCodeStrInternal(static_cast<WiresharkStatusCode>(idx)) + " = " + std::to_string(idx));
    }

    const std::string Templ =
        "#^#NAME#$# = {\n"
        "    #^#VALS#$#\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkStatusCodeNameInternal()},
        {"VALS", util::genStrListToString(vals, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkOptModeNameInternal() const
{
    return wiresharkLocalNamespaceName(m_wiresharkGenerator) + ".OptMode";
}

std::string Wireshark::wiresharkOptionalModeDefInternal() const
{
    auto& schemas = m_wiresharkGenerator.genSchemas();
    bool requiresOptMode =
        std::any_of(
            schemas.begin(), schemas.end(),
            [](auto& sPtr)
            {
                assert(sPtr);
                return WiresharkSchema::wiresharkCast(*sPtr).wiresharkNeedsOptionalModeDefinition();
            });

    if (!requiresOptMode) {
        return strings::genEmptyString();
    }

    util::GenStringsList vals;
    util::GenStringsList valNames;

    for (auto idx = 0U; idx < static_cast<unsigned>(WiresharkOptMode::ValuesLimit); ++idx) {
        auto& optModeStr = wiresharkOptModeStrInternal(static_cast<WiresharkOptMode>(idx));
        vals.push_back(optModeStr + " = " + std::to_string(idx));
        valNames.push_back("[" + std::to_string(idx) + "] = \"" + optModeStr + "\"");
    }

    const std::string Templ =
        "#^#NAME#$# = {\n"
        "    #^#VALS#$#\n"
        "}\n"
        "\n"
        "#^#VALS_NAME#$# = {\n"
        "    #^#VAL_NAMES#$#\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkOptModeNameInternal()},
        {"VALS", util::genStrListToString(vals, ",\n", "")},
        {"VALS_NAME", wiresharkOptModeValsName(m_wiresharkGenerator)},
        {"VAL_NAMES", util::genStrListToString(valNames, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkExtractorsDeclInternal() const
{
    const std::string Templ =
        "-- Extractors Management\n"
        "#^#MAP#$# = {}\n"
        "\n"
        "-- Invoke this function every time the extractor needs to be created\n"
        "function #^#NAME#$#(name, field)\n"
        "    #^#MAP#$#[field] = Field.new(name)\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"MAP", wiresharkExtractorsMapName(m_wiresharkGenerator)},
        {"NAME", wiresharkCreateExtractorFuncName(m_wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkExtractorsRegCodeInternal() const
{
    util::GenStringsList elems;
    for (auto& sPtr : m_wiresharkGenerator.genSchemas()) {
        auto str = WiresharkSchema::wiresharkCast(*sPtr).wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "", "");
}

std::string Wireshark::wiresharkFieldValueFuncInternal() const
{
    const std::string Templ =
        "function #^#NAME#$#(field)\n"
        "    local extractor = #^#MAP#$#[field]\n"
        "    local info = {extractor()}\n"
        "    local last = info[#info]\n"
        "    return last.value\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldValueFuncName(m_wiresharkGenerator)},
        {"MAP", wiresharkExtractorsMapName(m_wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkProtocolVersionDefInternal() const
{
    const std::string Templ =
        "#^#LOCAL#$#.spec_version = #^#SPEC#$#\n"
        "#^#LOCAL#$#.prot_version = {}\n"
        "\n"
        "function #^#GET_NAME#$#()\n"
        "    local pkt_id = #^#PKT_ID#$#()\n"
        "    local last_ver = #^#LOCAL#$#.prot_version[pkt_id]\n"
        "    if last_ver ~= #^#NIL#$# then\n"
        "        return last_ver\n"
        "    end\n"
        "\n"
        "    return #^#LOCAL#$#.spec_version\n"
        "end\n"
        "\n"
        "function #^#SET_NAME#$#(value)\n"
        "    local pkt_id = #^#PKT_ID#$#()\n"
        "    #^#LOCAL#$#.prot_version[pkt_id] = value\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"LOCAL", wiresharkLocalNamespaceName(m_wiresharkGenerator)},
        {"SPEC", std::to_string(m_wiresharkGenerator.genProtocolSchema().genSchemaVersion())},
        {"GET_NAME", wiresharkProtVersionGetFuncName(m_wiresharkGenerator)},
        {"SET_NAME", wiresharkProtVersionSetFuncName(m_wiresharkGenerator)},
        {"PKT_ID", wiresharkPacketIdFuncName(m_wiresharkGenerator)},
        {"NIL", strings::genNilStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkPinfoDefInternal() const
{
    const std::string Templ =
        "#^#NAME#$# = #^#NIL#$#\n"
        "\n"
        "function #^#PKT_ID#$#()\n"
        "    return tostring(#^#NAME#$#.src) .. \":\" .. tostring(#^#NAME#$#.src_port) .. \" -> \" .. tostring(#^#NAME#$#.dst) .. \":\" .. tostring(#^#NAME#$#.dst_port)\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkPinfoName(m_wiresharkGenerator)},
        {"NIL", strings::genNilStr()},
        {"PKT_ID", wiresharkPacketIdFuncName(m_wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Wireshark::wiresharkCrcCodeDefInternal() const
{
    auto& schemas = m_wiresharkGenerator.genSchemas();
    bool hasCrc =
        std::any_of(
            schemas.begin(), schemas.end(),
            [](auto& sPtr)
            {
                return WiresharkSchema::wiresharkCast(*sPtr).wiresharkNeedsCrcCalc();
            });

    if (!hasCrc) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "function #^#NAME#$#(width, poly, init, ref_in, ref_out, xor_out)\n"
        "    local function reflect(val, w)\n"
        "        local res = 0\n"
        "        for i = 0, w - 1 do\n"
        "            if bit32.band(bit32.rshift(val, i), 1) == 1 then\n"
        "                res = bit32.bor(res, bit32.lshift(1, w - 1 - i))\n"
        "            end\n"
        "        end\n"
        "        return res\n"
        "    end\n"
        "\n"
        "    local tbl = {}\n"
        "    local mask = (width == 32) and 0xFFFFFFFF or (bit32.lshift(1, width) - 1)\n"
        "    local msb_mask = bit32.lshift(1, width - 1)\n"
        "\n"
        "    -- Precompute the 256-value lookup table for this specific CRC profile\n"
        "    for i = 0, 255 do\n"
        "        local crc\n"
        "        if ref_in then\n"
        "            crc = i\n"
        "            local ref_poly = reflect(poly, width)\n"
        "            for j = 1, 8 do\n"
        "                if bit32.band(crc, 1) == 1 then\n"
        "                    crc = bit32.bxor(bit32.rshift(crc, 1), ref_poly)\n"
        "                else\n"
        "                    crc = bit32.rshift(crc, 1)\n"
        "                end\n"
        "            end\n"
        "        else\n"
        "            crc = bit32.lshift(i, width - 8)\n"
        "            for j = 1, 8 do\n"
        "                if bit32.band(crc, msb_mask) ~= 0 then\n"
        "                    crc = bit32.bxor(bit32.lshift(crc, 1), poly)\n"
        "                else\n"
        "                    crc = bit32.lshift(crc, 1)\n"
        "                end\n"
        "            end\n"
        "        end\n"
        "        tbl[i] = bit32.band(crc, mask)\n"
        "    end\n"
        "\n"
        "    -- Return the actual high-speed checksum function\n"
        "    -- Accepts tvb, offset, and offset_limit (exclusive end index)\n"
        "    return function(tvb, offset, offset_limit)\n"
        "        local crc = init\n"
        "        local length = offset_limit - offset\n"
        "        if length <= 0 then\n"
        "            return init\n"
        "        end\n"
        "\n"
        "        -- Extracting raw bytes once is massively faster than calling tvb:range(i, 1) in a loop\n"
        "        local data = tvb:range(offset, length):raw()\n"
        "\n"
        "        for i = 1, #data do\n"
        "            local byte = data:byte(i)\n"
        "            if ref_in then\n"
        "                local idx = bit32.band(bit32.bxor(crc, byte), 0xFF)\n"
        "                crc = bit32.bxor(bit32.rshift(crc, 8), tbl[idx])\n"
        "            else\n"
        "                local idx = bit32.band(bit32.bxor(bit32.rshift(crc, width - 8), byte), 0xFF)\n"
        "                crc = bit32.bxor(bit32.lshift(crc, 8), tbl[idx])\n"
        "            end\n"
        "            crc = bit32.band(crc, mask)\n"
        "        end\n"
        "\n"
        "        -- If output reflection differs from input reflection, we must reflect it\n"
        "        if ref_out ~= ref_in then\n"
        "            crc = reflect(crc, width)\n"
        "        end\n"
        "\n"
        "        crc = bit32.bxor(crc, xor_out)\n"
        "\n"
        "        -- Force unsigned 32-bit (for environments where bitwise ops return signed 32-bit)\n"
        "        if crc < 0 then\n"
        "            crc = crc + 4294967296\n"
        "        end\n"
        "\n"
        "        return bit32.band(crc, mask)\n"
        "    end\n"
        "end\n";

    util::GenReplacementMap repl = {
        {"NAME", wiresharkCreateCrcFuncName(m_wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

const std::string& Wireshark::wiresharkStatusCodeStrInternal(WiresharkStatusCode code)
{
    static const std::string Map[] = {
        /* Success */ "SUCCESS",
        /* NotEnoughData */ "NOT_ENOUGH_DATA",
        /* MalformedData */ "MALFORMED_PACKET",
        /* InvalidMsgId */ "INVALID_MSG_ID",
        /* InvalidMsgData */ "INVALID_MSG_DATA",
        /* ChecksumError */ "CHECKSUM_ERROR",
        /* CodegenError */ "CODEGEN_ERROR",
        /* InvalidFrame */ "INVALID_FRAME",
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(WiresharkStatusCode::ValuesLimit));

    auto idx = static_cast<unsigned>(code);
    assert(idx < MapSize);
    return Map[idx];
}

const std::string& Wireshark::wiresharkOptModeStrInternal(WiresharkOptMode code)
{
    static const std::string Map[] = {
        /* Tentative */ "TENTATIVE",
        /* Exists */ "EXISTS",
        /* Missing */ "MISSING",
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(WiresharkOptMode::ValuesLimit));

    auto idx = static_cast<unsigned>(code);
    assert(idx < MapSize);
    return Map[idx];
}

} // namespace commsdsl2wireshark

