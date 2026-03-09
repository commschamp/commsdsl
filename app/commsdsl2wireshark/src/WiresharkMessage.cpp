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

#include "WiresharkMessage.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkMessage::WiresharkMessage(WiresharkGenerator& generator, ParseMessage parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

WiresharkMessage::~WiresharkMessage() = default;

std::string WiresharkMessage::wiresharkDissectName() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkDissectNameFor(*this);
}

std::string WiresharkMessage::wiresharkDissectCode() const
{
    if (!genIsReferenced()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "#^#FIELDS#$#\n"
        "#^#NAME_VAR#$#\n"
        "#^#PREPEND#$#\n"
        "local function #^#NAME#$##^#SUFFIX#$#(tvb, tree, offset, offset_limit)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#"
        ;

    util::GenStringsList fields;
    for (auto* fPtr : m_wiresharkFields) {
        auto str = fPtr->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        fields.push_back(std::move(str));
    }

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(*this);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto prependFileName = relPath + strings::genPrependFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"FIELDS", util::genStrListToString(fields, "\n", "")},
        {"NAME", wiresharkDissectName()},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"PREPEND", wiresharkGenerator.genReadCodeInjectCode(prependFileName, "Prepend here")},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
        {"NAME_VAR", wiresharkNameDefInternal()},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyInternal();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkMessage::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_wiresharkFields = WiresharkField::wiresharkTransformFieldsList(genFields());
    return true;
}

std::string WiresharkMessage::wiresharkDissectBodyInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenStringsList fields;
    for (auto* f : m_wiresharkFields) {
        static const std::string FieldTempl =
            "result, next_offset = #^#DISSECT#$#(tvb, tree, next_offset, offset_limit)\n"
            "if result ~= #^#SUCCESS#$# then\n"
            "    return result, offset\n"
            "end\n"
        ;

        util::GenReplacementMap fieldRepl = {
            {"DISSECT", f->wiresharkDissectName()},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
        };

        fields.push_back(util::genProcessTemplate(FieldTempl, fieldRepl));
    }

    static const std::string Templ =
        "local result = #^#SUCCESS#$#\n"
        "local next_offset = offset\n"
        "tree = tree:add(#^#PROTO#$#, #^#NAME#$#)\n"
        "#^#FIELDS#$#\n"
        "return result, next_offset\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkMessageNameVarNameStr()},
        {"FIELDS", util::genStrListToString(fields, "\n", "")},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
        {"PROTO", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkMessage::wiresharkMessageNameVarNameStr() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkFuncNameFor(*this, strings::genNameSuffixStr());
}

std::string WiresharkMessage::wiresharkNameDefInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(*this, strings::genNameSuffixStr());
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();

    static const std::string Templ =
        "#^#COMMENT#$#"
        "local #^#VAR_NAME#$# = \"#^#NAME#$#\"\n"
    ;

    bool hasName = false;
    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"COMMENT", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace name value", &hasName)},
        {"VAR_NAME", wiresharkMessageNameVarNameStr()},
        {"NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
    };

    if (hasName) {
        repl["NAME"] = std::move(repl["COMMENT"]);
    }

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
