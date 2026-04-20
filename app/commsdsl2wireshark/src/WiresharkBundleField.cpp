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

#include "WiresharkBundleField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkBundleField::WiresharkBundleField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkBundleField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }

    m_wiresharkFields = wiresharkTransformFieldsList(genMembers());
    auto& generator = genGenerator();
    m_wiresharkFields.erase(
        std::remove_if(
            m_wiresharkFields.begin(), m_wiresharkFields.end(),
            [&generator](auto* fPtr)
            {
                auto parseObj = fPtr->wiresharkGenField().genParseObj();
                return !generator.genDoesElementExist(parseObj.parseSinceVersion(), parseObj.parseDeprecatedSince(), parseObj.parseIsDeprecatedRemoved());
            }),
        m_wiresharkFields.end());
    return true;
}

std::string WiresharkBundleField::wiresharkMembersDissectCodeImpl() const
{
    util::GenStringsList elems;
    for (auto* f : m_wiresharkFields) {
        auto str = f->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "\n", "\n");
}

std::string WiresharkBundleField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    // TODO: re-evaluate. Return failure on member read failure.
    auto parseObj = genBundleFieldParseObj();
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenStringsList members;
    bool hasLimit = false;
    for (auto* f : m_wiresharkFields) {
        static const std::string MemTempl =
            "#^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#DISSECT#$#(#^#TVB#$#, #^#SUBTREE#$#, #^#NEXT_OFFSET#$#, #^#LIMIT#$#)\n"
            "#^#LIMIT_CHECK#$#"
            "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
            "    #^#SUBTREE#$#:set_hidden(true)\n"
            "    return #^#RESULT#$#, #^#OFFSET#$#\n"
            "end\n"
            ;

        util::GenReplacementMap memRepl = {
            {"RESULT", wiresharkResultStr()},
            {"DISSECT", f->wiresharkDissectName()},
            {"TVB", wiresharkTvbStr()},
            {"SUBTREE", wiresharkFieldSubtreeStr()},
            {"LIMIT", wiresharkOffsetLimitStr()},
            {"NEXT_OFFSET", wiresharkNextOffsetStr()},
            {"OFFSET", wiresharkOffsetStr()},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        };

        if (hasLimit) {
            static const std::string CheckTempl =
                "if #^#RESULT#$# == #^#NOT_ENOUGH_DATA#$# then\n"
                "    return #^#ERROR#$#, #^#OFFSET#$#\n"
                "end\n"
                ;

            util::GenReplacementMap checkRepl = {
                {"RESULT", wiresharkResultStr()},
                {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
                {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::MalformedPacket)},
                {"OFFSET", wiresharkOffsetStr()},
            };

            memRepl["LIMIT_CHECK"] = util::genProcessTemplate(CheckTempl, checkRepl);
        }

        members.push_back(util::genProcessTemplate(MemTempl, memRepl));

        auto memParseObj = f->wiresharkGenField().genParseObj();
        if (memParseObj.parseSemanticType() != commsdsl::parse::ParseField::ParseSemanticType::Length) {
            continue;
        }

        hasLimit = true;

        static const std::string LimitTempl =
            "local next_limit = #^#VALUE_FUNC#$#(#^#FIELD#$#)\n"
            "if #^#LIMIT#$# < next_limit then\n"
            "    return #^#NOT_ENOUGH_DATA#$#, #^#OFFSET#$#\n"
            "end\n"
            "#^#LIMIT#$# = next_limit\n"
            ;

        util::GenReplacementMap limitRepl = {
            {"LIMIT", wiresharkOffsetLimitStr()},
            {"VALUE_FUNC", f->wiresharkValueFuncName()},
            {"FIELD", f->wiresharkFieldObjName()},
            {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
            {"OFFSET", wiresharkOffsetStr()},
        };

        members.push_back(util::genProcessTemplate(LimitTempl, limitRepl));
    }

    static const std::string Templ =
        "local #^#RANGE#$# = #^#TVB#$#(#^#OFFSET#$#, -1)\n"
        "local #^#SUBTREE#$# = #^#TREE#$#:add(#^#FIELD#$#, #^#RANGE#$#)\n"
        "#^#MEMBERS#$#\n"
        "#^#SUBTREE#$#:set_len(#^#NEXT_OFFSET#$# - #^#OFFSET#$#)\n"
        ;

    util::GenReplacementMap repl = {
        {"RANGE", wiresharkRangeStr()},
        {"LEN", std::to_string(parseObj.parseMaxLength())},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"MEMBERS", util::genStrListToString(members, "\n", "")},
        {"FIELD", wiresharkFieldStr()},
        {"TVB", wiresharkTvbStr()},
        {"TREE", wiresharkTreeStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkBundleField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
{
    util::GenStringsList members;
    for (auto* f : m_wiresharkFields) {
        auto str = f->wiresharkExtractorsRegCode(refField);
        if (str.empty()) {
            continue;
        }

        members.push_back(std::move(str));
    }

    return WiresharkBase::wiresharkExtractorsRegCodeImpl(refField) + '\n' + util::genStrListToString(members, "\n", "");
}

std::string WiresharkBundleField::wiresharkValidFuncBodyImpl(const WiresharkField* refField) const
{
    util::GenStringsList members;
    for (auto* f : m_wiresharkFields) {
        if (f->wiresharkHasTrivialValid()) {
            continue;
        }

        static const std::string MemTempl =
            "result, _ = #^#FUNC#$#(#^#FIELD#$#)\n"
            "if not result then\n"
            "    return false, false\n"
            "end\n"
            ;

        util::GenReplacementMap memRepl = {
            {"FUNC", f->wiresharkValidFuncName(refField)},
            {"FIELD", f->wiresharkFieldObjName(refField)},
        };

        members.push_back(util::genProcessTemplate(MemTempl, memRepl));
    }

    static const std::string Templ =
        "local result = false\n"
        "#^#MEMBERS#$#\n"
        "#^#CONDS#$#\n"
        "return true\n"
        ;

    util::GenReplacementMap repl = {
        {"MEMBERS", util::genStrListToString(members, "\n", "")},
        {"CONDS", wiresharkExtraValidCondsCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkBundleField::wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    if (accStr.empty()) {
        // TODO: support value override to allow proper comparison to bundle value
        return WiresharkBase::wiresharkValueAccessStrImpl(accStr, refField);
    }

    assert(refField == nullptr);
    auto memInfo = wiresharkSplitMemberAccStr(accStr, m_wiresharkFields);
    if (memInfo.first == nullptr) {
        genGenerator().genLogger().genError("BUG: Unexpected access string \"" + accStr + "\" for " + genParseObj().parseInnerRef());
        return strings::genUnexpectedValueStr();
    }

    return memInfo.first->wiresharkValueAccessStr(memInfo.second);
}

std::string WiresharkBundleField::wiresharkSizeAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    if (accStr.empty()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return WiresharkBase::wiresharkSizeAccessStrImpl(accStr, refField);
    }

    assert(refField == nullptr);
    auto memInfo = wiresharkSplitMemberAccStr(accStr, m_wiresharkFields);
    if (memInfo.first == nullptr) {
        genGenerator().genLogger().genError("BUG: Unexpected access string \"" + accStr + "\" for " + genParseObj().parseInnerRef());
        return strings::genUnexpectedValueStr();
    }

    return memInfo.first->wiresharkSizeAccessStr(memInfo.second);
}

std::string WiresharkBundleField::wiresharkExistsCheckStrImpl(const std::string& accStr) const
{
    if (accStr.empty()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return WiresharkBase::wiresharkExistsCheckStrImpl(accStr);
    }

    auto memInfo = wiresharkSplitMemberAccStr(accStr, m_wiresharkFields);
    if (memInfo.first == nullptr) {
        genGenerator().genLogger().genError("BUG: Unexpected access string \"" + accStr + "\" for " + genParseObj().parseInnerRef());
        return strings::genUnexpectedValueStr();
    }

    return memInfo.first->wiresharkExistsCheckStr(memInfo.second);
}

std::string WiresharkBundleField::wiresharkDefaultAssignmentsImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    util::GenStringsList elems;
    for (auto* f : m_wiresharkFields) {
        auto str = f->wiresharkDefaultAssignments();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "", "");
}

bool WiresharkBundleField::wiresharkHasTrivialValidImpl() const
{
    auto parseObj = genBundleFieldParseObj();
    auto validCond = parseObj.parseValidCond();
    if (validCond.parseValid()) {
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

const WiresharkBundleField::WiresharkFieldsList& WiresharkBundleField::wiresharkMemberFieldsImpl() const
{
    return m_wiresharkFields;
}

std::string WiresharkBundleField::wiresharkExtraValidCondsCodeInternal() const
{
    auto parseObj = genBundleFieldParseObj();
    auto validCond = parseObj.parseValidCond();
    if (!validCond.parseValid()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "result =\n"
        "    #^#COND#$#\n"
        "\n"
        "if not result then\n"
        "    return false, true\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"COND", wiresharkDslCondToString(wiresharkGenerator, m_wiresharkFields, wiresharkInterface(), validCond)},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
