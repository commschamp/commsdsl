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

#include "WiresharkOptionalField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <utility>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

WiresharkOptionalField::WiresharkOptionalField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkOptionalField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }

    m_wiresharkField = WiresharkField::wiresharkCast(genMemberField());
    if (m_wiresharkField == nullptr) {
        m_wiresharkField = WiresharkField::wiresharkCast(genExternalField());
    }

    assert(m_wiresharkField != nullptr);
    return true;
}

std::string WiresharkOptionalField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
{
    auto* memField = WiresharkField::wiresharkCast(genMemberField());
    if (memField == nullptr) {
        return WiresharkBase::wiresharkExtractorsRegCodeImpl(refField);
    }

    return WiresharkBase::wiresharkExtractorsRegCodeImpl(refField) + memField->wiresharkExtractorsRegCode();
}

std::string WiresharkOptionalField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.uint8(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, base.DEC, #^#VALS#$#, #^#NIL#$#, #^#DESC#$#))\n"
    ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(wiresharkGenerator)},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
        {"NIL", strings::genNilStr()},
        {"VALS", Wireshark::wiresharkOptModeValsName(wiresharkGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkOptionalField::wiresharkMembersDissectCodeImpl() const
{
    assert(m_wiresharkField != nullptr);
    return m_wiresharkField->wiresharkDissectCode();
}

std::string WiresharkOptionalField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    static const std::string Templ =
        "repeat\n"
        "    local mode = #^#MODE#$#\n"
        "    #^#MODE_COND#$#\n"
        "    #^#TENTATIVE_CHECK#$#\n"
        "    local #^#RANGE#$# = #^#TVB#$#(#^#OFFSET#$#, 0)\n"
        "    #^#SUBTREE#$# = #^#TREE#$#:add(#^#FIELD#$#, #^#RANGE#$#, mode)\n"
        "\n"
        "    if mode == #^#MISSING#$# then\n"
        "        #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#SUCCESS#$#, #^#OFFSET#$#\n"
        "        break\n"
        "    end\n"
        "\n"
        "    #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#MEM_FIELD#$#(#^#TVB#$#, #^#SUBTREE#$#, #^#OFFSET#$#, #^#LIMIT#$#)\n"
        "\n"
        "    #^#READ_FAIL#$#\n"
        "    #^#MISS_INVALID#$#\n"
        "    #^#SUBTREE#$#:set_len(#^#NEXT_OFFSET#$# - #^#OFFSET#$#)\n"
        "until true\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"MODE", wiresharkDefaultModeInternal()},
        {"RANGE", wiresharkRangeStr()},
        {"TVB", wiresharkTvbStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"TREE", wiresharkTreeStr()},
        {"FIELD", wiresharkFieldStr()},
        {"MISSING", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Missing)},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"RESULT", wiresharkResultStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"MEM_FIELD", m_wiresharkField->wiresharkDissectName()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"READ_FAIL", wiresharkReadFailCodeInternal()},
        {"MISS_INVALID", wiresharkMissInvalidCodeInternal()},
        {"MODE_COND", wiresharkCondCodeInternal()},
        {"TENTATIVE_CHECK", wiresharkTentativeCheckCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkOptionalField::wiresharkValidFuncBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    assert(refField == nullptr);
    static const std::string Templ =
        "if #^#VALUE_FUNC#$#(#^#FIELD_STR#$#) ~= #^#EXISTS#$# then\n"
        "    return true\n"
        "end\n"
        "\n"
        "return #^#VALID_FUNC#$#(#^#FIELD#$#)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"VALUE_FUNC", wiresharkValueFuncName()},
        {"FIELD_STR", wiresharkFieldStr()},
        {"EXISTS", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Exists)},
        {"VALID_FUNC", wiresharkValidFuncName()},
        {"FIELD", m_wiresharkField->wiresharkFieldObjName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkOptionalField::wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    if (accStr.empty()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return WiresharkBase::wiresharkValueAccessStrImpl(accStr, refField);
    }

    auto splitAccStr = wiresharkSplitAccStr(accStr);
    assert(splitAccStr.first == m_wiresharkField->wiresharkGenField().genName());
    return m_wiresharkField->wiresharkValueAccessStr(splitAccStr.second);
}

std::string WiresharkOptionalField::wiresharkSizeAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    if (accStr.empty()) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return WiresharkBase::wiresharkValueAccessStrImpl(accStr, refField);
    }

    auto splitAccStr = wiresharkSplitAccStr(accStr);
    assert(splitAccStr.first == m_wiresharkField->wiresharkGenField().genName());
    return m_wiresharkField->wiresharkValueAccessStr(splitAccStr.second);
}

std::string WiresharkOptionalField::wiresharkExistsCheckStrImpl(const std::string& accStr) const
{
    if (accStr.empty()) {
        static const std::string Templ =
            "(#^#VALUE_FUNC#$#(#^#FIELD#$#) == #^#EXISTS#$#)"
            ;

        auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
        util::GenReplacementMap repl = {
            {"VALUE_FUNC", wiresharkValueFuncName()},
            {"EXISTS", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Exists)},
            {"FIELD", wiresharkFieldObjName()},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    auto splitAccStr = wiresharkSplitAccStr(accStr);
    assert(splitAccStr.first == m_wiresharkField->wiresharkGenField().genName());
    return m_wiresharkField->wiresharkExistsCheckStr(splitAccStr.second);
}

bool WiresharkOptionalField::wiresharkHasTrivialValidImpl() const
{
    auto parseObj = genOptionalFieldParseObj();
    if ((parseObj.parseMissingOnReadFail()) ||
        (parseObj.parseMissingOnInvalid())) {
        // Always valid even if the member field is invalid
        return true;
    }

    assert(m_wiresharkField != nullptr);
    return m_wiresharkField->wiresharkHasTrivialValid();
}

std::string WiresharkOptionalField::wiresharkDefaultModeInternal() const
{
    static_assert(static_cast<int>(ParseOptionalField::ParseMode::Tentative) == static_cast<int>(Wireshark::WiresharkOptMode::Tentative));
    static_assert(static_cast<int>(ParseOptionalField::ParseMode::Exists) == static_cast<int>(Wireshark::WiresharkOptMode::Exists));
    static_assert(static_cast<int>(ParseOptionalField::ParseMode::Missing) == static_cast<int>(Wireshark::WiresharkOptMode::Missing));
    static_assert(static_cast<int>(ParseOptionalField::ParseMode::NumOfValues) == static_cast<int>(Wireshark::WiresharkOptMode::ValuesLimit));

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genOptionalFieldParseObj();
    auto wiresharkMode = static_cast<Wireshark::WiresharkOptMode>(parseObj.parseDefaultMode());
    return Wireshark::wiresharkOptModeStr(wiresharkGenerator, wiresharkMode);
}

std::string WiresharkOptionalField::wiresharkReadFailCodeInternal() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genOptionalFieldParseObj();
    if (!parseObj.parseMissingOnReadFail()) {
        static const std::string Templ =
            "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
            "    break\n"
            "end\n"
            ;

        util::GenReplacementMap repl = {
            {"RESULT", wiresharkResultStr()},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string Templ =
        "if result == #^#NOT_ENOUGH_DATA#$# then\n"
        "    break\n"
        "end\n"
        "\n"
        "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
        "    #^#SUBTREE#$#:set_hidden(true)\n"
        "    #^#TREE#$#:add(#^#FIELD#$#, #^#RANGE#$#, #^#MISSING#$#)\n"
        "    #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#SUCCESS#$#, #^#OFFSET#$#\n"
        "    break\n"
        "end\n"
        ;

    util::GenReplacementMap repl = {
        {"RESULT", wiresharkResultStr()},
        {"MISSING", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Missing)},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"NOT_ENOUGH_DATA", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::NotEnoughData)},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"FIELD", wiresharkFieldStr()},
        {"RANGE", wiresharkRangeStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"TREE", wiresharkTreeStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkOptionalField::wiresharkMissInvalidCodeInternal() const
{
    auto parseObj = genOptionalFieldParseObj();
    if (!parseObj.parseMissingOnInvalid()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "local field_valid = #^#VALID_FUNC#$#(#^#ACT_FIELD#$#)\n"
        "if not field_valid then\n"
        "    #^#SUBTREE#$#:set_hidden(true)\n"
        "    #^#TREE#$#:add(#^#FIELD#$#, #^#RANGE#$#, #^#MISSING#$#)\n"
        "    #^#RESULT#$#, #^#NEXT_OFFSET#$# = #^#SUCCESS#$#, #^#OFFSET#$#\n"
        "    break\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"MISSING", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Missing)},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"FIELD", wiresharkFieldStr()},
        {"RANGE", wiresharkRangeStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"VALID_FUNC", m_wiresharkField->wiresharkValidFuncName()},
        {"ACT_FIELD", m_wiresharkField->wiresharkFieldObjName()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"RESULT", wiresharkResultStr()},
        {"TREE", wiresharkTreeStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkOptionalField::wiresharkCondCodeInternal() const
{
    auto parseObj = genOptionalFieldParseObj();
    auto cond = parseObj.parseCond();
    if (!cond.parseValid()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "local exists =\n"
        "   #^#COND#$#\n"
        "if exists then\n"
        "    mode = #^#EXISTS#$#\n"
        "else\n"
        "    mode = #^#MISSING#$#\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"COND", wiresharkDslCondToString(wiresharkGenerator, wiresharkSiblings(), wiresharkInterface(), cond)},
        {"EXISTS", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Exists)},
        {"MISSING", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Missing)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkOptionalField::wiresharkTentativeCheckCodeInternal() const
{
    auto parseObj = genOptionalFieldParseObj();
    if (parseObj.parseDefaultMode() != ParseOptionalField::ParseMode::Tentative) {
        return strings::genEmptyString();
    }

    auto cond = parseObj.parseCond();
    if (cond.parseValid()) {
        // The mode has been updated
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "if #^#LIMIT#$# <= #^#OFFSET#$# then\n"
        "    mode = #^#MISSING#$#\n"
        "else\n"
        "    mode = #^#EXISTS#$#\n"
        "end\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"EXISTS", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Exists)},
        {"MISSING", Wireshark::wiresharkOptModeStr(wiresharkGenerator, Wireshark::WiresharkOptMode::Missing)},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
