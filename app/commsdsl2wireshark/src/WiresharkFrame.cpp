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

#include "WiresharkFrame.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"
#include "WiresharkInterface.h"
#include "WiresharkNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkFrame::WiresharkFrame(WiresharkGenerator& generator, ParseFrame parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

WiresharkFrame::~WiresharkFrame() = default;

std::string WiresharkFrame::wiresharkDissectName() const
{
    if (!m_validFrame) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkDissectNameFor(*this);
}

std::string WiresharkFrame::wiresharkDissectCode() const
{
    if (!m_validFrame) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "#^#LAYERS#$#\n"
        "#^#FUNC_LIST#$#\n"
        "#^#PREPEND#$#\n"
        "#^#RETVAL_MAP#$#\n"
        "function #^#NAME#$##^#SUFFIX#$#(#^#TVB#$#, #^#TREE#$#)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(*this);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto prependFileName = relPath + strings::genPrependFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"LAYERS", wiresharkLayersDissectCodeInternal()},
        {"NAME", wiresharkDissectName()},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"PREPEND", wiresharkGenerator.genReadCodeInjectCode(prependFileName, "Prepend here")},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
        {"FUNC_LIST", wiresharkLayerFuncsListInternal()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
        {"TREE", WiresharkField::wiresharkTreeStr()},
        {"RETVAL_MAP", wiresharkRetvalMapDefInternal()},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyInternal();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFrame::wiresharkExtractorsRegCode() const
{
    util::GenStringsList layers;
    for (auto* l : m_wiresharkLayers) {
        auto str = l->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        layers.push_back(std::move(str));
    }

    return util::genStrListToString(layers, "", "");
}

bool WiresharkFrame::wiresharkNeedsOptionalModeDefinition() const
{
    if (!m_validFrame) {
        return false;
    }

    return
        std::any_of(
            m_wiresharkLayers.begin(), m_wiresharkLayers.end(),
            [](auto* l)
            {
                return l->wiresharkNeedsOptionalModeDefinition();
            });
}

bool WiresharkFrame::wiresharkNeedsCrcCalc() const
{
    if (!m_validFrame) {
        return false;
    }

    return
        std::any_of(
            m_wiresharkLayers.begin(), m_wiresharkLayers.end(),
            [](auto* l)
            {
                return l->wiresharkNeedsCrcCalc();
            });
}

bool WiresharkFrame::wiresharkValidFrame() const
{
    return m_validFrame;
}

bool WiresharkFrame::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    bool success = true;
    auto reorderedLayers = genCommsOrderOfLayers(success);
    if (!success) {
        return false;
    }

    for (auto* l : reorderedLayers) {
        m_wiresharkLayers.push_back(WiresharkLayer::wiresharkCast(l));
    }

    auto* iFace = wiresharkInterfaceInternal();
    do {
        if (iFace == nullptr) {
            genGenerator().genLogger().genDebug("No valid interface for frame " + genName());
            break;
        }

        m_validFrame =
            std::all_of(
                m_wiresharkLayers.begin(), m_wiresharkLayers.end(),
                [iFace](auto* l)
                {
                    return l->wiresharkIsInterfaceSupported(*iFace);
                });
    } while (false);
    return true;
}

std::string WiresharkFrame::wiresharkDissectBodyInternal() const
{
    static const std::string Templ =
        "#^#INTERFACE#$#\n"
        "local #^#RESULT#$# = #^#SUCCESS#$#\n"
        "local #^#OFFSET#$# = 0\n"
        "local len = #^#TVB#$#:len()\n"
        "while (#^#RESULT#$# == #^#SUCCESS#$#) and (#^#OFFSET#$# < len) do\n"
        "    local frame_subtree = #^#TREE#$#:add(#^#PROTO_NAME#$#, #^#TVB#$#(#^#OFFSET#$#, -1), \"#^#FRAME_NAME#$#\")\n"
        "    local layer_func = #^#LIST_NAME#$#[1]\n"
        "    local #^#NEXT_OFFSET#$# = #^#OFFSET#$#\n"
        "    #^#RESULT#$#, #^#NEXT_OFFSET#$# = layer_func(#^#TVB#$#, frame_subtree, #^#OFFSET#$#, len, #^#LIST_NAME#$#, 2, #^#NIL#$#)\n"
        "    if #^#RESULT#$# == #^#NOT_ENOUGH_DATA#$# then\n"
        "        frame_subtree:set_hidden(true)\n"
        "        return #^#RESULT#$#, #^#OFFSET#$#\n"
        "    end\n"
        "\n"
        "    if #^#RESULT#$# == #^#CODEGEN_ERROR#$# then\n"
        "        frame_subtree:set_hidden(true)\n"
        "        #^#TREE#$#:add_expert_info(PI_DISSECTOR_BUG, PI_ERROR, \"!!!CODE GENERATOR BUG!!!\")\n"
        "        return #^#RESULT#$#, len\n"
        "    end\n"
        "\n"
        "    #^#RESULT#$# = #^#RETVAL_MAP#$#[#^#RESULT#$#] or #^#RESULT#$#\n"
        "    if #^#RESULT#$# == #^#INVALID_FRAME#$# then\n"
        "        frame_subtree:set_hidden(true)\n"
        "        return #^#RESULT#$#, #^#OFFSET#$#\n"
        "    end\n"
        "\n"
        "    if #^#RESULT#$# == #^#SUCCESS#$# then\n"
        "        goto continue\n"
        "    end\n"
        "\n"
        "    -- #^#MALFORMED#$# or other unexpected error\n"
        "    frame_subtree:set_hidden(true)\n"
        "    #^#NEXT_OFFSET#$# = #^#OFFSET#$# + 1\n"
        "\n"
        "    ::continue::\n"
        "    frame_subtree:set_len(#^#NEXT_OFFSET#$# - #^#OFFSET#$#)\n"
        "    #^#OFFSET#$# = #^#NEXT_OFFSET#$#\n"
        "end\n"
        "return #^#RESULT#$#, #^#OFFSET#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto* ns = WiresharkNamespace::wiresharkCast(genParentNamespace());
    assert(ns != nullptr);
    auto* iFace = ns->wiresharkInterface();
    assert(ns != nullptr);

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"PROTO_NAME", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
        {"FRAME_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"LIST_NAME", wiresharkLayerFuncsListNameInternal()},
        {"NIL", strings::genNilStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
        {"CODEGEN_ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::CodegenError)},
        {"MALFORMED", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::MalformedPacket)},
        {"INVALID_FRAME", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::InvalidFrame)},
        {"INTERFACE", iFace->wiresharkDefaultAssignments()},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
        {"RESULT", WiresharkField::wiresharkResultStr()},
        {"NEXT_OFFSET", WiresharkField::wiresharkNextOffsetStr()},
        {"TVB", WiresharkField::wiresharkTvbStr()},
        {"TREE", WiresharkField::wiresharkTreeStr()},
        {"RETVAL_MAP", wiresharkRetvalMapNameInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFrame::wiresharkLayersDissectCodeInternal() const
{
    util::GenStringsList layers;
    for (auto* l : m_wiresharkLayers) {
        auto str = l->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        layers.push_back(std::move(str));
    }

    return util::genStrListToString(layers, "\n", "\n");
}

const WiresharkInterface* WiresharkFrame::wiresharkInterfaceInternal() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto* parentNs = WiresharkNamespace::wiresharkCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    return parentNs->wiresharkInterface();
}

std::string WiresharkFrame::wiresharkLayerFuncsListInternal() const
{
    static const std::string Templ =
        "#^#NAME#$# = {\n"
        "    #^#LAYERS#$#\n"
        "}\n"
        ;

    util::GenStringsList layers;
    for (auto* l : m_wiresharkLayers) {
        layers.push_back(l->wiresharkDissectName());
    }

    util::GenReplacementMap repl = {
        {"NAME", wiresharkLayerFuncsListNameInternal()},
        {"LAYERS", util::genStrListToString(layers, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFrame::wiresharkLayerFuncsListNameInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkFuncNameFor(*this, "_layers");
}

std::string WiresharkFrame::wiresharkRetvalMapDefInternal() const
{
    static const std::string Templ =
        "#^#NAME#$# = {\n"
        "    #^#REPLACE#$#\n"
        "    #^#VALUES#$#\n"
        "}\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathPrefix() + wiresharkRetvalMapNameInternal();
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();

    bool replaced = false;
    util::GenReplacementMap repl = {
        {"NAME", wiresharkRetvalMapNameInternal()},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this map contents", &replaced)},
    };

    if (!replaced) {
        static const Wireshark::WiresharkStatusCode SuccessCodes[] = {
            Wireshark::WiresharkStatusCode::InvalidMsgData,
            Wireshark::WiresharkStatusCode::ChecksumError,
        };

        util::GenStringsList values;
        for (auto c : SuccessCodes) {
            static const std::string ValTempl =
                "[#^#VAL#$#] = #^#SUCCESS#$#"
                ;

            util::GenReplacementMap valRepl = {
                {"VAL", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, c)},
                {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
            };

            values.push_back(util::genProcessTemplate(ValTempl, valRepl));
        }

        repl["VALUES"] = util::genStrListToString(values, ",\n", "");
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFrame::wiresharkRetvalMapNameInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkFuncNameFor(*this, "_retval_map");
}

} // namespace commsdsl2wireshark
