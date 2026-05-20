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
#include "WiresharkNamespace.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

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
        "#^#VALID_FUNC#$#\n"
        "#^#PREPEND#$#\n"
        "function #^#NAME#$##^#SUFFIX#$#(tvb, tree, offset, offset_limit)\n"
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
        {"VALID_FUNC", wiresharkValidFuncCodeInternal()},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyInternal();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkMessage::wiresharkExtractorsRegCode() const
{
    if (!genIsReferenced()) {
        return strings::genEmptyString();
    }

    util::GenStringsList fields;
    for (auto* fPtr : m_wiresharkFields) {
        auto str = fPtr->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        fields.push_back(std::move(str));
    }

    return util::genStrListToString(fields, "", "");
}

bool WiresharkMessage::wiresharkNeedsOptionalModeDefinition() const
{
    if (!genIsReferenced()) {
        return false;
    }

    return
        std::any_of(
            m_wiresharkFields.begin(), m_wiresharkFields.end(),
            [](auto* fPtr)
            {
                return fPtr->wiresharkNeedsOptionalModeDefinition();
            });
}

const WiresharkMessage::WiresharkFieldsList& WiresharkMessage::wiresharkMemberFields() const
{
    return m_wiresharkFields;
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
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        };

        fields.push_back(util::genProcessTemplate(FieldTempl, fieldRepl));
    }

    static const std::string Templ =
        "local result = #^#SUCCESS#$#\n"
        "local next_offset = offset\n"
        "tree = tree:add(#^#PROTO#$#, #^#NAME#$#)\n"
        "#^#READ_COND#$#\n"
        "#^#FIELDS#$#\n"
        "#^#VALID#$#\n"
        "return result, next_offset\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkMessageNameVarNameStr()},
        {"FIELDS", util::genStrListToString(fields, "\n", "")},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"PROTO", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
        {"VALID", wiresharkDissectValidCheckCodeInternal()},
        {"READ_COND", wiresharkDissectReadCondCheckCodeInternal()},
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
        "#^#VAR_NAME#$# = \"#^#NAME#$#\"\n"
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

std::string WiresharkMessage::wiresharkValidFuncNameInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkFuncNameFor(*this, strings::genValidSuffixStr());
}

std::string WiresharkMessage::wiresharkValidFuncCodeInternal() const
{
    if (wiresharkHasTrivialValidInternal()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "function #^#NAME#$##^#SUFFIX#$#()\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputRelPathFor(*this, strings::genValidSuffixStr());
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"NAME", wiresharkValidFuncNameInternal()},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkValidFuncBodyInternal();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkMessage::wiresharkValidFuncBodyInternal() const
{
    util::GenStringsList fields;
    for (auto* f : m_wiresharkFields) {
        if (f->wiresharkHasTrivialValid()) {
            continue;
        }

        static const std::string FieldTempl =
            "result, _ = #^#FUNC#$#(#^#FIELD#$#)\n"
            "if not result then\n"
            "    return false, false\n"
            "end\n"
            ;

        util::GenReplacementMap fieldRepl = {
            {"FUNC", f->wiresharkValidFuncName()},
            {"FIELD", f->wiresharkFieldObjName()},
        };

        fields.push_back(util::genProcessTemplate(FieldTempl, fieldRepl));
    }

    static const std::string Templ =
        "local #^#RESULT#$# = true\n"
        "#^#FIELDS#$#\n"
        "#^#COND#$#\n"
        "return true\n"
        ;

    auto parseObj = genParseObj();
    auto cond = parseObj.parseValidCond();

    util::GenReplacementMap repl = {
        {"FIELDS", util::genStrListToString(fields, "\n", "")},
        {"RESULT", WiresharkField::wiresharkResultStr()},
    };

    if (cond.parseValid()) {
        static const std::string CondTempl =
            "#^#RESULT#$# =\n"
            "    #^#VALID#$#\n"
            "\n"
            "if not #^#RESULT#$# then\n"
            "    return false, true\n"
            "end\n"
            ;

        auto* ns = genParentNamespace();
        assert(ns != nullptr);
        auto* iFace = WiresharkNamespace::wiresharkCast(ns)->wiresharkInterface();
        assert(iFace != nullptr);

        auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
        util::GenReplacementMap condRepl = {
            {"VALID", WiresharkField::wiresharkDslCondToString(wiresharkGenerator, m_wiresharkFields, *iFace, cond)},
            {"RESULT", WiresharkField::wiresharkResultStr()},
        };

        repl["COND"] = util::genProcessTemplate(CondTempl, condRepl);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkMessage::wiresharkDissectValidCheckCodeInternal() const
{
    if (wiresharkHasTrivialValidInternal()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "local valid, print_warn = #^#VALID_FUNC#$#()\n"
        "if not valid then\n"
        "    #^#CODE#$#\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"VALID_FUNC", wiresharkValidFuncNameInternal()},
        {"TREE", WiresharkField::wiresharkTreeStr()},
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::InvalidMsgData)},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
    };

    if (genParseObj().parseIsFailOnInvalid()) {
        static const std::string FailTempl =
            //"#^#TREE#$#:set_hidden(true)\n"
            "return #^#ERROR#$#, #^#OFFSET#$#\n"
            ;

        repl["CODE"] = util::genProcessTemplate(FailTempl, repl);
    }
    else {
        static const std::string FailTempl =
            "if print_warn then\n"
            "    #^#TREE#$#:add_expert_info(PI_PROTOCOL, PI_WARN, \"Invalid message contents\")\n"
            "end"
            ;
        repl["CODE"] = util::genProcessTemplate(FailTempl, repl);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkMessage::wiresharkDissectReadCondCheckCodeInternal() const
{
    auto parseObj = genParseObj();
    auto readCond = parseObj.parseReadCond();
    if (!readCond.parseValid()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "local read_valid = \n"
        "    #^#CONDS#$#\n"
        "\n"
        "if not read_valid then\n"
        "    return #^#ERROR#$#, #^#OFFSET#$#\n"
        "end\n"
        ;

    auto* ns = genParentNamespace();
    assert(ns != nullptr);
    auto* iFace = WiresharkNamespace::wiresharkCast(ns)->wiresharkInterface();
    assert(iFace != nullptr);

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"CONDS", WiresharkField::wiresharkDslCondToString(wiresharkGenerator, m_wiresharkFields, *iFace, readCond)},
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::InvalidMsgData)},
        {"OFFSET", WiresharkField::wiresharkOffsetStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkMessage::wiresharkHasTrivialValidInternal() const
{
    auto parseObj = genParseObj();
    if (parseObj.parseValidCond().parseValid()) {
        return false;
    }

    return
        std::all_of(
            m_wiresharkFields.begin(), m_wiresharkFields.end(),
            [](auto* f)
            {
                return f->wiresharkHasTrivialValid();
            });
}

} // namespace commsdsl2wireshark
