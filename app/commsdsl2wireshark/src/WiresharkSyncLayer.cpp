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

#include "WiresharkSyncLayer.h"

#include "Wireshark.h"
#include "WiresharkField.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

namespace
{

const std::string& wiresharkEscDataSuffixInternal()
{
    static const std::string Str("_esc_data");
    return Str;
}

const std::string& wiresharkUnescDataSuffixInternal()
{
    static const std::string Str("_unesc_data");
    return Str;
}

} // namespace

WiresharkSyncLayer::WiresharkSyncLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkSyncLayer::wiresharkDissectBodyImpl() const
{
    auto parseObj = genSyncLayerParseObj();
    if (parseObj.parseIsAfterPayload()) {
        return wiresharkSuffixDissectCodeInternal();
    }

    return wiresharkPrefixDissectCodeInternal();
}

std::string WiresharkSyncLayer::wiresharkExtraDissectCodeImpl() const
{
    util::GenStringsList elems;
    auto parseObj = genSyncLayerParseObj();
    do {
        if (!parseObj.parseHasEscField()) {
            break;
        }

        auto escField = WiresharkField::wiresharkCast(genExternalEscField());
        if (escField == nullptr) {
            escField = WiresharkField::wiresharkCast(genMemberEscField());
        }

        assert(escField != nullptr);
        elems.push_back(escField->wiresharkDissectCode());

        if (parseObj.parseIsAfterPayload()) {
            static const std::string Templ =
                "#^#NAME#$##^#ESC_SUFFIX#$# = #^#CREATE_FUNC#$#(ProtoField.bytes(\"#^#NAME#$##^#ESC_SUFFIX#$#\", \"Escaped Data\", base.SPACE))\n"
                "#^#NAME#$##^#UNESC_SUFFIX#$# = #^#CREATE_FUNC#$#(ProtoField.bytes(\"#^#NAME#$##^#UNESC_SUFFIX#$#\", \"Unescaped Data\", base.SPACE))\n"
                ;

            auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
            util::GenReplacementMap repl = {
                {"NAME", wiresharkDissectName()},
                {"ESC_SUFFIX", wiresharkEscDataSuffixInternal()},
                {"UNESC_SUFFIX", wiresharkUnescDataSuffixInternal()},
                {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(wiresharkGenerator)},
            };

            elems.push_back(util::genProcessTemplate(Templ, repl));
        }
    } while (false);

    return util::genStrListToString(elems, "\n", "");
}

std::string WiresharkSyncLayer::wiresharkExtractorsRegCodeImpl() const
{
    auto parseObj = genSyncLayerParseObj();
    if (!parseObj.parseHasEscField()) {
        return strings::genEmptyString();
    }

    auto escField = WiresharkField::wiresharkCast(genExternalEscField());
    if (escField == nullptr) {
        escField = WiresharkField::wiresharkCast(genMemberEscField());
    }

    assert(escField != nullptr);
    return escField->wiresharkExtractorsRegCode();
}

std::string WiresharkSyncLayer::wiresharkPrefixDissectCodeInternal() const
{
    static const std::string Templ =
        "#^#FIELD#$#\n"
        "#^#CHECK#$#\n"
        "#^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
        "#^#NEXT#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"FIELD", WiresharkLayer::wiresharkDissectFieldCode()},
        {"NEXT", wiresharkNextFuncCode()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"CHECK", wiresharkSyncValueCheckCodeInternal()},
    };

    auto parseObj = genSyncLayerParseObj();
    if (parseObj.parseSeekField()) {
        repl["FIELD"] = wiresharkSeekPrefixFieldCodeInternal();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSyncLayer::wiresharkSuffixDissectCodeInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genSyncLayerParseObj();
    if (parseObj.parseSeekField()) {
        static const std::string Templ =
            "local orig_tvb = #^#TVB#$#\n"
            "local orig_tree = #^#TREE#$#\n"
            "local orig_limit = #^#LIMIT#$#\n"
            "#^#FIELD#$#\n"
            "#^#NEXT#$#\n"
            "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
            "    return #^#RESULT#$#, #^#NEXT_OFFSET#$#\n"
            "end\n"
            "\n"
            "#^#TVB#$# = orig_tvb\n"
            "#^#TREE#$# = orig_tree\n"
            "#^#LIMIT#$# = orig_limit\n"
            "#^#ADJ_OFFSET#$#\n"
            "#^#FIELD_AGAIN#$#\n"
            "#^#CHECK#$#\n"
            ;

        util::GenReplacementMap repl = {
            {"FIELD", wiresharkSeekSuffixFieldCodeInternal()},
            {"NEXT", wiresharkNextFuncCode()},
            {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
            {"FIELD_AGAIN", WiresharkLayer::wiresharkDissectFieldCode()},
            {"OFFSET", WiresharkField::wiresharkOffsetStr()},
            {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
            {"CHECK", wiresharkSyncValueCheckCodeInternal()},
            {"TREE", WiresharkField::wiresharkTreeStr()},
            {"TVB", WiresharkField::wiresharkTvbStr()},
            {"RESULT", WiresharkField::wiresharkResultStr()},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
            {"ADJ_OFFSET", wiresharkAdjustSuffixOffsetInternal()},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    auto* field = wiresharkField();
    assert(field != nullptr);
    auto minLen = field->wiresharkMinFieldLength();
    auto maxLen = field->wiresharkMaxFieldLength();
    if (parseObj.parseVerifyBeforeRead() && (minLen == maxLen)) {
        static const std::string Templ =
            "local orig_tree = #^#TREE#$#\n"
            "local orig_offset = #^#OFFSET#$#\n"
            "local sync_field_offset = #^#LIMIT#$# - #^#LEN#$#\n"
            "#^#OFFSET#$# = sync_field_offset\n"
            "#^#TREE#$# = #^#TREE#$#:add(#^#PROTO_NAME#$#, #^#TVB#$#(#^#OFFSET#$#, -1))\n"
            "#^#TREE#$#:set_hidden(true)\n"
            "#^#FIELD#$#\n"
            "#^#CHECK#$#\n"
            "#^#TREE#$# = orig_tree\n"
            "#^#OFFSET#$# = orig_offset\n"
            "#^#LIMIT#$# = sync_field_offset\n"
            "#^#NEXT#$#\n"
            "#^#OFFSET#$# = sync_field_offset\n"
            "#^#LIMIT#$# = sync_field_offset + #^#LEN#$#\n"
            "#^#FIELD#$#\n"
            "#^#CHECK#$#\n"
            ;

        util::GenReplacementMap repl = {
            {"OFFSET", WiresharkField::wiresharkOffsetStr()},
            {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
            {"LEN", std::to_string(minLen)},
            {"FIELD", WiresharkLayer::wiresharkDissectFieldCode()},
            {"CHECK", wiresharkSyncValueCheckCodeInternal()},
            {"NEXT", wiresharkNextFuncCode()},
            {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
            {"TREE", WiresharkField::wiresharkTreeStr()},
            {"PROTO_NAME", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
            {"TVB", WiresharkField::wiresharkTvbStr()},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string Templ =
        "#^#NEXT#$#\n"
        "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
        "    return #^#RESULT#$#, #^#OFFSET#$#\n"
        "end"
        "\n"
        "#^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
        "#^#FIELD#$#\n"
        "#^#CHECK#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"FIELD", WiresharkLayer::wiresharkDissectFieldCode()},
        {"NEXT", wiresharkNextFuncCode()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"CHECK", wiresharkSyncValueCheckCodeInternal()},
        {"RESULT", WiresharkField::wiresharkResultStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSyncLayer::wiresharkSeekPrefixFieldCodeInternal() const
{
    auto parseObj = genSyncLayerParseObj();
    static const std::string Templ =
        "local seek_subtree = #^#TREE#$#:add(#^#PROTO_NAME#$#, #^#TVB#$#(#^#NEXT_OFFSET#$#, -1))\n"
        "seek_subtree:set_hidden(true)\n"
        "local escaped = false\n"
        "local found = false\n"
        "while #^#NEXT_OFFSET#$# < #^#LIMIT#$# do\n"
        "    local from_offset = #^#NEXT_OFFSET#$#\n"
        "    #^#ESC_FIELD#$#\n"
        "    if (not escaped) then\n"
        "        #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, seek_subtree, from_offset, #^#LIMIT#$#)\n"
        "        if (#^#RESULT#$# == #^#SUCCESS#$#) and (#^#VALUE_FUNC#$#() == #^#VAL#$#) then\n"
        "            found = true\n"
        "            #^#OFFSET#$# = from_offset\n"
        "            #^#NEXT_OFFSET#$# = from_offset\n"
        "            break\n"
        "        end\n"
        "    end\n"
        "    #^#NEXT_OFFSET#$# = from_offset + 1\n"
        "    escaped = false\n"
        "    ::continue::\n"
        "end\n"
        "\n"
        "if not found then\n"
        "    return #^#NOT_ENOUGH_DATA#$#, #^#OFFSET#$#\n"
        "end\n"
        "\n"
        "#^#FIELD#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto* field = wiresharkField();
    assert(field != nullptr);
    util::GenReplacementMap repl = {
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
        {"RESULT", WiresharkField::wiresharkResultStr()},
        {"DISSECT", field->wiresharkDissectName()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
        {"TREE", WiresharkField::wiresharkTreeStr()},
        {"PROTO_NAME", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
        {"FIELD", WiresharkLayer::wiresharkDissectFieldCode()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"VALUE_FUNC", field->wiresharkValueFuncName()},
        {"VAL", field->wiresharkDefaultValueStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
    };

    if (parseObj.parseHasEscField()) {
        auto escField = WiresharkField::wiresharkCast(genExternalEscField());
        if (escField == nullptr) {
            escField = WiresharkField::wiresharkCast(genMemberEscField());
        }

        assert(escField != nullptr);
        assert(escField->wiresharkGenField().genIsReferenced());
        const std::string EscTempl =
            "if not escaped then\n"
            "    #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, seek_subtree, from_offset, #^#LIMIT#$#)\n"
            "    if (result == #^#SUCCESS#$#) and (#^#VALUE_FUNC#$#() == #^#VAL#$#) then\n"
            "        escaped = true\n"
            "        goto continue\n"
            "    end\n"
            "end\n"
            ;

        util::GenReplacementMap escRepl = {
            {"RESULT", WiresharkField::wiresharkResultStr()},
            {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
            {"DISSECT", escField->wiresharkDissectName()},
            {"TVB", WiresharkField::wiresharkTvbStr()},
            {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
            {"VALUE_FUNC", escField->wiresharkValueFuncName()},
            {"VAL", escField->wiresharkDefaultValueStr()},
        };

        repl["ESC_FIELD"] = util::genProcessTemplate(EscTempl, escRepl);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSyncLayer::wiresharkSeekSuffixFieldCodeInternal() const
{
    static const std::string Templ =
        "local seek_subtree = #^#TREE#$#:add(#^#PROTO_NAME#$#, #^#TVB#$#(#^#NEXT_OFFSET#$#, -1))\n"
        "seek_subtree:set_hidden(true)\n"
        "local escaped = false\n"
        "local found = false\n"
        "local until_offset = #^#NEXT_OFFSET#$#\n"
        "local raw_bytes = {}\n"
        "while #^#NEXT_OFFSET#$# < #^#LIMIT#$# do\n"
        "    until_offset = #^#NEXT_OFFSET#$#\n"
        "    #^#ESC_FIELD#$#\n"
        "    if (not escaped) then\n"
        "        #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, seek_subtree, until_offset, #^#LIMIT#$#)\n"
        "        if (#^#RESULT#$# == #^#SUCCESS#$#) and (#^#VALUE_FUNC#$#() == #^#VAL#$#) then\n"
        "            found = true\n"
        "            #^#LIMIT#$# = until_offset\n"
        "            break\n"
        "        end\n"
        "    end\n"
        "    local byte = #^#TVB#$#(until_offset, 1):uint()\n"
        "    table.insert(raw_bytes, string.format(\"%02x\", byte))\n"
        "    #^#NEXT_OFFSET#$# = until_offset + 1\n"
        "    escaped = false\n"
        "    ::continue::\n"
        "end\n"
        "\n"
        "if not found then\n"
        "    return #^#NOT_ENOUGH_DATA#$#, #^#OFFSET#$#\n"
        "end\n"
        "\n"
        "#^#LIMIT#$# = until_offset\n"
        "#^#ESC_TREE#$#\n"
        ;

    auto parseObj = genSyncLayerParseObj();
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto* field = wiresharkField();
    assert(field != nullptr);
    util::GenReplacementMap repl = {
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
        {"RESULT", WiresharkField::wiresharkResultStr()},
        {"DISSECT", field->wiresharkDissectName()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
        {"TREE", WiresharkField::wiresharkTreeStr()},
        {"PROTO_NAME", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
        {"FIELD", WiresharkLayer::wiresharkDissectFieldCode()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"VALUE_FUNC", field->wiresharkValueFuncName()},
        {"VAL", field->wiresharkDefaultValueStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
    };

    if (parseObj.parseHasEscField()) {
        auto escField = WiresharkField::wiresharkCast(genExternalEscField());
        if (escField == nullptr) {
            escField = WiresharkField::wiresharkCast(genMemberEscField());
        }

        assert(escField != nullptr);
        assert(escField->wiresharkGenField().genIsReferenced());
        const std::string EscTempl =
            "if not escaped then\n"
            "    #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, seek_subtree, until_offset, #^#LIMIT#$#)\n"
            "    if (result == #^#SUCCESS#$#) and (#^#VALUE_FUNC#$#() == #^#VAL#$#) then\n"
            "        escaped = true\n"
            "        goto continue\n"
            "    end\n"
            "end\n"
            ;

        util::GenReplacementMap escRepl = {
            {"RESULT", WiresharkField::wiresharkResultStr()},
            {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
            {"DISSECT", escField->wiresharkDissectName()},
            {"TVB", WiresharkField::wiresharkTvbStr()},
            {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
            {"VALUE_FUNC", escField->wiresharkValueFuncName()},
            {"VAL", escField->wiresharkDefaultValueStr()},
        };

        repl["ESC_FIELD"] = util::genProcessTemplate(EscTempl, escRepl);

        static const std::string EscTreeTempl =
            "#^#TREE#$# = #^#TREE#$#:add(#^#NAME#$##^#ESC_SUFFIX#$#, #^#TVB#$#(#^#OFFSET#$#, #^#LIMIT#$# - #^#OFFSET#$#))\n"
            "local esc_data = ByteArray.new(table.concat(raw_bytes, \"\"))\n"
            "local esc_tvb = ByteArray.tvb(esc_data, \"Unescaped Data\")\n"
            "#^#TVB#$# = esc_tvb\n"
            "#^#OFFSET#$# = 0\n"
            "#^#LIMIT#$# = esc_data:len()\n"
            "#^#TREE#$# = #^#TREE#$#:add(#^#NAME#$##^#UNESC_SUFFIX#$#, #^#TVB#$#(#^#OFFSET#$#, #^#LIMIT#$# - #^#OFFSET#$#))\n"
            ;

        util::GenReplacementMap escTreeRepl = {
            {"TREE", WiresharkField::wiresharkTreeStr()},
            {"TVB", WiresharkField::wiresharkTvbStr()},
            {"NAME", wiresharkDissectName()},
            {"ESC_SUFFIX", wiresharkEscDataSuffixInternal()},
            {"UNESC_SUFFIX", wiresharkUnescDataSuffixInternal()},
            {"OFFSET", WiresharkField::wiresharkOffsetStr()},
            {"LIMIT", WiresharkField::wiresharkOffsetLimitStr()},
        };

        repl["ESC_TREE"] = util::genProcessTemplate(EscTreeTempl, escTreeRepl);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSyncLayer::wiresharkSyncValueCheckCodeInternal() const
{
    static const std::string Templ =
        "if #^#VALUE_FUNC#$#() ~= #^#VAL#$# then\n"
        "    return #^#ERROR#$#\n"
        "end\n"
        ;

    auto* field = wiresharkField();
    assert(field != nullptr);
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"VALUE_FUNC", field->wiresharkValueFuncName()},
        {"VAL", field->wiresharkDefaultValueStr()},
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::InvalidMsgData)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSyncLayer::wiresharkAdjustSuffixOffsetInternal() const
{
    auto parseObj = genSyncLayerParseObj();
    if (parseObj.parseIsAfterPayload() && parseObj.parseHasEscField()) {
        static const std::string Templ =
            "#^#OFFSET#$# = until_offset\n"
        ;

        util::GenReplacementMap repl = {
            {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string Templ =
        "#^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
