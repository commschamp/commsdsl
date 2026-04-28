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

#include "WiresharkVariantField.h"

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

WiresharkVariantField::WiresharkVariantField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkVariantField::genPrepareImpl()
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

std::string WiresharkVariantField::wiresharkDissectCodeImpl(const WiresharkField* refField) const
{
    assert(refField == nullptr);

    static const std::string Templ =
        "#^#NAME#$#_member_rec = {}\n"
        "\n"
        "function #^#NAME#$#_member_rec_set(field, value)\n"
        "    #^#NAME#$#_member_rec[field] = value\n"
        "end\n"
        "\n"
        "function #^#NAME#$#_member_rec_get(field)\n"
        "    local value = #^#NAME#$#_member_rec[field]\n"
        "    if value == #^#NIL#$# then\n"
        "        return 0\n"
        "    end\n"
        "\n"
        "    return value\n"
        "end\n"
        "\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldObjName(refField)},
        {"NIL", strings::genNilStr()},
    };

    return util::genProcessTemplate(Templ, repl) + WiresharkBase::wiresharkDissectCodeImpl(refField);
}

std::string WiresharkVariantField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
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

std::string WiresharkVariantField::wiresharkMembersDissectCodeImpl() const
{
    util::GenStringsList elems;
    util::GenStringsList dissect;
    for (auto* f : m_wiresharkFields) {
        auto str = f->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
        dissect.push_back(f->wiresharkDissectName());
    }

    static const std::string Templ =
        "#^#MEMBERS#$#\n"
        "#^#NAME#$#_member_dissect = {\n"
        "    #^#MEMBERS_DISSECT#$#\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"MEMBERS", util::genStrListToString(elems, "\n", "\n")},
        {"MEMBERS_DISSECT", util::genStrListToString(dissect, ",\n", "")},
        {"NAME", wiresharkFieldObjName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkVariantField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    static const std::string Templ =
        "local #^#RANGE#$# = #^#TVB#$#(#^#OFFSET#$#, -1)\n"
        "local #^#SUBTREE#$# = #^#TREE#$#:add(#^#FIELD#$#, #^#RANGE#$#)\n"
        "#^#NAME#$#_member_rec_set(#^#FIELD#$#, 0)\n"
        "for idx, func in ipairs(#^#NAME#$#_member_dissect) do\n"
        "    #^#RESULT#$#, #^#NEXT_OFFSET#$# = func(#^#TVB#$#, #^#SUBTREE#$#, #^#OFFSET#$#, #^#LIMIT#$#)\n"
        "    if #^#RESULT#$# == #^#SUCCESS#$# then\n"
        "        #^#NAME#$#_member_rec_set(#^#FIELD#$#, idx)\n"
        "        break\n"
        "    end\n"
        "end\n"
        "\n"
        "if #^#NAME#$#_member_rec_get(#^#FIELD#$#) < 1 then\n"
        "    #^#SUBTREE#$#:set_hidden(true)\n"
        "    return #^#ERROR#$#, #^#OFFSET#$#\n"
        "end\n"
        "#^#SUBTREE#$#:set_len(#^#NEXT_OFFSET#$# - #^#OFFSET#$#)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"RANGE", wiresharkRangeStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"TREE", wiresharkTreeStr()},
        {"NAME", wiresharkFieldObjName()},
        {"FIELD", wiresharkFieldStr()},
        {"RESULT", wiresharkResultStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"TVB", wiresharkTvbStr()},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::InvalidMsgData)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkVariantField::wiresharkValidFuncBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    static const std::string Templ =
        "local idx = #^#NAME#$#_member_rec_get(#^#FIELD#$#)\n"
        "if idx < 1 then\n"
        "    return false, true\n"
        "end\n"
        "\n"
        "local mem_valid_func = #^#NAME#$#_member_valid[idx]\n"
        "if mem_valid_func == #^#NIL#$# then\n"
        "    return true\n"
        "end\n"
        "\n"
        "local result = mem_valid_func()\n"
        "return result, false\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldObjName()},
        {"FIELD", wiresharkFieldStr()},
        {"NIL", strings::genNilStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkVariantField::wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    if (accStr.empty()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
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

std::string WiresharkVariantField::wiresharkSizeAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
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

std::string WiresharkVariantField::wiresharkExistsCheckStrImpl(const std::string& accStr) const
{
    if (accStr.empty()) {
        // TODO: variant has actual member set.
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

std::string WiresharkVariantField::wiresharkDefaultAssignmentsImpl([[maybe_unused]] const WiresharkField* refField) const
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

std::string WiresharkVariantField::wiresharkValidFuncCodeImpl(const WiresharkField* refField) const
{
    assert(refField == nullptr);
    util::GenStringsList elems;
    for (auto* f : m_wiresharkFields) {
        if (f->wiresharkHasTrivialValid()) {
            elems.push_back(strings::genNilStr());
            continue;
        }

        elems.push_back(f->wiresharkValidFuncName());
    }

    static const std::string Templ =
        "#^#NAME#$#_member_valid = {\n"
        "    #^#ELEMS#$#\n"
        "}\n"
        "\n"
        "#^#FUNC#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"ELEMS", util::genStrListToString(elems, ",\n", "")},
        {"NAME", wiresharkFieldObjName(refField)},
        {"FUNC", WiresharkBase::wiresharkValidFuncCodeImpl(refField)},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkVariantField::wiresharkHasTrivialValidImpl() const
{
    return false;
}

const WiresharkVariantField::WiresharkFieldsList& WiresharkVariantField::wiresharkMemberFieldsImpl() const
{
    return m_wiresharkFields;
}

} // namespace commsdsl2wireshark
