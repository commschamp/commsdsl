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

#include "WiresharkBitfieldField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"
#include "WiresharkIntField.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/parse/ParseIntField.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <type_traits>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

using ParseIntField = commsdsl::parse::ParseIntField;

namespace commsdsl2wireshark
{

WiresharkBitfieldField::WiresharkBitfieldField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkBitfieldField::wiresharkForcedBitfieldMask(const WiresharkField& member) const
{
    std::size_t bitOffset = 0U;
    for (auto& mPtr : genMembers()) {
        auto parseObj = mPtr->genParseObj();
        auto bitLen = parseObj.parseBitLength();
        assert(bitLen != 0U);
        if (mPtr.get() == &member.wiresharkGenField()) {
            auto mask = ((1ULL << bitLen) - 1U) << bitOffset;
            std::stringstream stream;
            stream << "0x" << std::hex << std::setfill('0') << std::setw(static_cast<int>(genParseObj().parseMaxLength() * 2)) << mask;
            return stream.str();
        }

        bitOffset += bitLen;
    }

    [[maybe_unused]] static constexpr bool Should_not_happen = false;
    assert(Should_not_happen);
    return strings::genNilStr();
}

std::string WiresharkBitfieldField::wiresharkIntegralType() const
{
    using ParseType = ParseIntField::ParseType;
    static const ParseType TypeMap[] = {
        ParseType::Uint8,
        ParseType::Uint16,
        ParseType::Uint32,
        ParseType::Uint32,
        ParseType::Uint64,
        ParseType::Uint64,
        ParseType::Uint64,
        ParseType::Uint64,
    };
    static const std::size_t TypeMapSize = std::extent_v<decltype(TypeMap)>;

    auto parseObj = genParseObj();
    auto len = parseObj.parseMaxLength();
    if (TypeMapSize < len) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genEmptyString();
    }
    auto idx = len - 1U;
    return WiresharkIntField::wiresharkIntegralType(TypeMap[idx], len);
}

unsigned WiresharkBitfieldField::wiresharkMaskShiftFor(const WiresharkField& member) const
{
    unsigned bitOffset = 0U;
    for (auto& mPtr : genMembers()) {
        auto parseObj = mPtr->genParseObj();
        auto bitLen = parseObj.parseBitLength();
        assert(bitLen != 0U);
        if (mPtr.get() == &member.wiresharkGenField()) {
            return bitOffset;
        }

        bitOffset += static_cast<decltype(bitOffset)>(bitLen);
    }

    [[maybe_unused]] static constexpr bool Should_not_happen = false;
    assert(Should_not_happen);
    return 0U;
}

bool WiresharkBitfieldField::genPrepareImpl()
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

std::string WiresharkBitfieldField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, base.HEX, #^#NIL#$#, #^#MASK#$#, #^#DESC#$#))\n"
    ;

    auto obj = genParseObj();
    util::GenReplacementMap repl = {
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkIntegralType()},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"NIL", strings::genNilStr()},
        {"MASK", wiresharkForcedIntegralFieldMask(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkBitfieldField::wiresharkMembersDissectCodeImpl() const
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

std::string WiresharkBitfieldField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    auto parseObj = genBitfieldFieldParseObj();
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenStringsList members;
    for (auto* f : m_wiresharkFields) {
        static const std::string MemTempl =
            "#^#RESULT#$#, _ = #^#DISSECT#$#(#^#TVB#$#, #^#SUBTREE#$#, #^#OFFSET#$#, #^#OFFSET#$# + #^#LEN#$#)\n"
            "if #^#RESULT#$# ~= #^#SUCCESS#$# then\n"
            "    return #^#RESULT#$#, #^#OFFSET#$#\n"
            "end\n"
            ;

        util::GenReplacementMap memRepl = {
            {"RESULT", wiresharkResultStr()},
            {"DISSECT", f->wiresharkDissectName()},
            {"TVB", wiresharkTvbStr()},
            {"SUBTREE", wiresharkFieldSubtreeStr()},
            {"OFFSET", wiresharkOffsetStr()},
            {"LEN", std::to_string(parseObj.parseMaxLength())},
            {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        };

        members.push_back(util::genProcessTemplate(MemTempl, memRepl));
    }

    static const std::string Templ =
        "local #^#RANGE#$# = #^#TVB#$#(#^#OFFSET#$#, #^#LEN#$#)\n"
        "local #^#SUBTREE#$# = #^#TREE#$#:add#^#SUFFIX#$#(#^#FIELD#$#, #^#RANGE#$#)\n"
        "#^#MEMBERS#$#\n"
        "#^#NEXT_OFFSET#$# = #^#OFFSET#$# + #^#LEN#$#\n"
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

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        repl["SUFFIX"] = strings::genLittleEndianSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkBitfieldField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
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

std::string WiresharkBitfieldField::wiresharkValidFuncBodyImpl(const WiresharkField* refField) const
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

std::string WiresharkBitfieldField::wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    if (accStr.empty()) {
        // TODO: support value override to allow proper comparison to bitfield value
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

std::string WiresharkBitfieldField::wiresharkSizeAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
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

std::string WiresharkBitfieldField::wiresharkExistsCheckStrImpl(const std::string& accStr) const
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

std::string WiresharkBitfieldField::wiresharkDefaultAssignmentsImpl([[maybe_unused]] const WiresharkField* refField) const
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

bool WiresharkBitfieldField::wiresharkHasTrivialValidImpl() const
{
    auto parseObj = genBitfieldFieldParseObj();
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

const WiresharkBitfieldField::WiresharkFieldsList& WiresharkBitfieldField::wiresharkMemberFieldsImpl() const
{
    return m_wiresharkFields;
}

std::string WiresharkBitfieldField::wiresharkExtraValidCondsCodeInternal() const
{
    auto parseObj = genBitfieldFieldParseObj();
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
