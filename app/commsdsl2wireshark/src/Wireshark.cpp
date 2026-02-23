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

#include <cassert>
#include <fstream>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

bool Wireshark::wiresharkWrite(WiresharkGenerator& generator)
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
    return wiresharkProtocolObjName(generator) + "_createField";
}

std::string Wireshark::wiresharkFieldsListName(const WiresharkGenerator& generator)
{
    return wiresharkProtocolObjName(generator) + "_fields_list";
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
            "#^#FIELDS_REG#$#\n"
            "#^#CODE#$#\n"
            "#^#DISSECT_FUNC#$#\n"
            "#^#NAME#$#.fields = #^#FIELDS_LIST#$#\n"
            "\n"
            "return #^#NAME#$#\n"
            ;

        util::GenReplacementMap repl = {
            {"GEN_COMMENT", m_wiresharkGenerator.wiresharkFileGeneratedComment()},
            {"PROTOCOL", wiresharkProtocolDefInternal()},
            {"FIELDS_REG", wiresharkFieldsRegistrationInternal()},
            {"DISSECT_FUNC", wiresharkDissectFuncInternal()},
            {"NAME", wiresharkProtocolObjName(m_wiresharkGenerator)},
            {"FIELDS_LIST", wiresharkFieldsListName(m_wiresharkGenerator)},
            {"CODE", wiresharkCodeInternal()},
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

std::string Wireshark::wiresharkDissectFuncInternal() const
{
    const std::string Templ =
        "-- Main Dissector Entry Point\n"
        "function #^#NAME#$#.dissector(tvb, pinfo, tree)\n"
        "    #^#BODY#$#\n"
        "end\n"
        ;

    bool bodyReplaced = false;
    auto replacePath = m_wiresharkGenerator.wiresharkInputDissectRelPathFor(wiresharkProtocolObjName(m_wiresharkGenerator) + ".dissector") + strings::genReplaceFileSuffixStr();
    auto replaceCode = m_wiresharkGenerator.genReadCodeInjectCode(replacePath, "Replace body", &bodyReplaced);

    util::GenReplacementMap repl = {
        {"NAME", wiresharkProtocolObjName(m_wiresharkGenerator)},
        {"BODY", std::move(replaceCode)},
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
        "local #^#LIST#$# = {}\n"
        "\n"
        "-- Invoke this function every time the field is created\n"
        "local function #^#NAME#$#(obj)\n"
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
        static const std::string FrameTempl =
            "result, next_offset = #^#NAME#$#(tvb, tree)\n"
            "if result then\n"
            "    break\n"
            "end\n"
        ;

        util::GenReplacementMap frameRepl = {
            {"NAME", wiresharkFrame.wiresharkDissectName()}
        };

        elems.push_back(util::genProcessTemplate(FrameTempl, frameRepl));
    }

    const std::string Templ =
        "if tvb:len() == 0 then\n"
        "    return\n"
        "end\n"
        "\n"
        "pinfo.cols.protocol = #^#NAME#$#.name\n"
        "\n"
        "local result = false\n"
        "local next_offset = 0\n"
        "repeat\n"
        "    #^#FRAMES#$#\n"
        "until true\n"
        "\n"
        "if not result then\n"
        "   return\n"
        "end\n"
        "\n"
        "if next_offset < tvb:len() then\n"
        "    pinfo.desegment_offset = next_offset\n"
        "    pinfo.desegment_len = DESEGMENT_ONE_MORE_SEGMENT\n"
        "end\n"
    ;

    util:: GenReplacementMap repl = {
        {"FRAMES", util::genStrListToString(elems, "\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark

