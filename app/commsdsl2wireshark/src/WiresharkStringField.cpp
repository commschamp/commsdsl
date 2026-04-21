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

#include "WiresharkStringField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

WiresharkStringField::WiresharkStringField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkStringField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }

    m_prefixField = WiresharkField::wiresharkCast(genMemberPrefixField());
    if (m_prefixField == nullptr) {
        m_prefixField = WiresharkField::wiresharkCast(genExternalPrefixField());
    }

    return true;
}

std::string WiresharkStringField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
{
    auto* memPrefix = WiresharkField::wiresharkCast(genMemberPrefixField());
    if (memPrefix == nullptr) {
        return WiresharkBase::wiresharkExtractorsRegCodeImpl(refField);
    }

    return WiresharkBase::wiresharkExtractorsRegCodeImpl(refField) + memPrefix->wiresharkExtractorsRegCode();
}

std::string WiresharkStringField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.string#^#Z#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, base.UNICODE, #^#DESC#$#))\n"
    ;

    util::GenReplacementMap repl = {
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    if (genStringFieldParseObj().parseHasZeroTermSuffix()) {
        repl["Z"] = "z";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkStringField::wiresharkMembersDissectCodeImpl() const
{
    if (m_prefixField == nullptr) {
        return strings::genEmptyString();
    }

    return m_prefixField->wiresharkDissectCode();
}

std::string WiresharkStringField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    static const std::string Templ =
        "#^#LENGTH#$#\n"
        "if #^#LIMIT#$# < (#^#NEXT_OFFSET#$# + len) then\n"
        "    return #^#NOT_ENOUGH_DATA#$#, #^#OFFSET#$#\n"
        "end\n"
        "\n"
        "local #^#RANGE#$# = #^#TVB#$#(#^#NEXT_OFFSET#$#, len)\n"
        "local #^#SUBTREE#$# = #^#TREE#$#:add(#^#FIELD#$#, #^#RANGE#$#)\n"
        "#^#NEXT_OFFSET#$# = #^#NEXT_OFFSET#$# + len\n"
        "#^#RESULT#$# = #^#SUCCESS#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"LENGTH", wiresharkDissectLengthCodeInternal()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
        {"OFFSET", wiresharkOffsetStr()},
        {"RANGE", wiresharkRangeStr()},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"TVB", wiresharkTvbStr()},
        {"TREE", wiresharkTreeStr()},
        {"FIELD", wiresharkFieldStr()},
        {"RESULT", wiresharkResultStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkStringField::wiresharkValidFuncBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    static const std::string Templ =
        "#^#PREFIX#$#\n"
        "#^#VALUE#$#\n"
        "#^#CHECKS#$#\n"
        ;

    util::GenReplacementMap repl;

    if ((m_prefixField != nullptr) && (!m_prefixField->wiresharkHasTrivialValid())) {
        static const std::string PrefixTempl =
            "if not #^#VALID_FUNC#$#(#^#PREFIX#$#) then\n"
            "    return false, true\n"
            "end\n"
            ;

        util::GenReplacementMap prefixRepl = {
            {"VALID_FUNC", m_prefixField->wiresharkValidFuncName()},
            {"PREFIX", m_prefixField->wiresharkFieldObjName()},
        };

        repl["PREFIX"] = util::genProcessTemplate(PrefixTempl, prefixRepl);
    }

    auto parseObj = genStringFieldParseObj();
    auto& validValues = parseObj.parseValidValues();

    util::GenStringsList comps;

    for (auto& v : validValues) {
        if (!genGenerator().genDoesElementExist(v.m_sinceVersion, v.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string CompTempl =
            "if value == \"#^#VAL#$#\" then\n"
            "    return true\n"
            "end\n"
            ;

        util::GenReplacementMap compRepl = {
            {"VAL", v.m_value},
        };

        comps.push_back(util::genProcessTemplate(CompTempl, compRepl));
    }

    if (comps.empty()) {
        repl["VALUE"] = "return true";
        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string ValueTempl =
        "local value = #^#VALUE_FUNC#$#(#^#FIELD#$#)"
        ;

    util::GenReplacementMap valueRepl = {
        {"VALUE_FUNC", wiresharkValueFuncName()},
        {"FIELD", wiresharkFieldStr()},
    };
    repl["VALUE"] = util::genProcessTemplate(ValueTempl, valueRepl);

    comps.push_back("return false, true");
    repl["CHECKS"] = util::genStrListToString(comps, "\n", "");
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkStringField::wiresharkCompPrepValueStrImpl(const std::string& value) const
{
    return '\"' + value + '\"';
}

bool WiresharkStringField::wiresharkHasTrivialValidImpl() const
{
    if ((m_prefixField != nullptr) && (!m_prefixField->wiresharkHasTrivialValid())) {
        return false;
    }

    auto parseObj = genStringFieldParseObj();
    auto& validValues = parseObj.parseValidValues();
    return (validValues.empty());
}

std::string WiresharkStringField::wiresharkDissectLengthCodeInternal() const
{
    auto parseObj = genStringFieldParseObj();
    auto fixedLen = parseObj.parseFixedLength();
    if (fixedLen != 0U) {
        return "local len = " + std::to_string(fixedLen);
    }

    if (m_prefixField != nullptr) {
        static const std::string Templ =
            "local prefix_tree = #^#TREE#$#:add(#^#PROTO#$#, #^#TVB#$#(#^#NEXT_OFFSET#$#, 0))\n"
            "prefix_tree:set_hidden(true)\n"
            "#^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#READ_FUNC#$#(#^#TVB#$#, prefix_tree, #^#NEXT_OFFSET#$#, #^#LIMIT#$#, #^#FIELD#$#)\n"
            "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
            "    return #^#RESULT#$#\n"
            "end\n"
            "\n"
            "local len = #^#VALUE_FUNC#$#(#^#FIELD#$#)"
            ;

        auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
        util::GenReplacementMap repl = {
            {"RESULT", wiresharkResultStr()},
            {"NEXT_OFFSET", wiresharkNextOffsetStr()},
            {"READ_FUNC", m_prefixField->wiresharkDissectName()},
            {"TVB", wiresharkTvbStr()},
            {"TREE", wiresharkTreeStr()},
            {"LIMIT", wiresharkOffsetLimitStr()},
            {"FIELD", m_prefixField->wiresharkFieldObjName()},
            {"VALUE_FUNC", m_prefixField->wiresharkValueFuncName()},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
            {"PROTO", Wireshark::wiresharkProtocolObjName(wiresharkGenerator)},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    auto& detachedPrefix = parseObj.parseDetachedPrefixFieldName();
    if (!detachedPrefix.empty()) {
        // TODO: test
        auto& siblings = wiresharkSiblings();
        auto sep = detachedPrefix.find_first_of('.');
        auto siblingName = detachedPrefix.substr(0, sep);

        auto iter =
            std::find_if(
                siblings.begin(), siblings.end(),
                [&siblingName](auto& f)
                {
                    return f->wiresharkGenField().genParseObj().parseName() == siblingName;
                });

        if (iter == siblings.end()) {
            genGenerator().genLogger().genError("BUG: Failed to find sibling \"" + detachedPrefix + "\" of " + parseObj.parseInnerRef());
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return strings::genEmptyString();
        }

        std::string remStr;
        if (sep < detachedPrefix.size()) {
            remStr = detachedPrefix.substr(sep + 1);
        }

        return "local len = " + (*iter)->wiresharkValueAccessStr(remStr);
    }

    if (parseObj.parseHasZeroTermSuffix()) {
        static const std::string Templ =
            "local len = 0\n"
            "while true do\n"
            "    local from = #^#NEXT_OFFSET#$# + len\n"
            "    if #^#LIMIT#$# <= from then\n"
            "        return #^#NOT_ENOUGH_DATA#$#, #^#OFFSET#$#\n"
            "    end\n"
            "\n"
            "    len = len + 1\n"
            "    if #^#TVB#$#(from, 1):uint() == 0 then\n"
            "        break\n"
            "    end\n"
            "end\n"
            ;

        auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
        util::GenReplacementMap repl = {
            {"NEXT_OFFSET", wiresharkNextOffsetStr()},
            {"LIMIT", wiresharkOffsetLimitStr()},
            {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
            {"TVB", wiresharkTvbStr()},
            {"OFFSET", wiresharkOffsetStr()},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string Templ =
        "local len = #^#LIMIT#$# - #^#NEXT_OFFSET#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
