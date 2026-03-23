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

#include "WiresharkEnumField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"
#include "WiresharkIntField.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkEnumField::WiresharkEnumField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkEnumField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "#^#VALS#$#\n"
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, #^#BASE#$#, #^#VALS_NAME#$#, #^#MASK#$#, #^#DESC#$#))\n"
    ;

    auto obj = genEnumFieldParseObj();
    util::GenReplacementMap repl = {
        {"VALS", wiresharkValsInternal(refField)},
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkForcedIntegralFieldType(refField)},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"VALS_NAME", wiresharkFieldObjName(refField) + strings::genValsSuffixStr()},
        {"BASE", "base.DEC_HEX"},
        {"MASK", wiresharkForcedIntegralFieldMask(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    if (obj.parseHexAssign()) {
        repl["BASE"] = "base.HEX_DEC";
    }

    if (repl["TYPE"].empty()) {
        repl["TYPE"] = WiresharkIntField::wiresharkIntegralType(obj.parseType(), obj.parseMaxLength());
    }
    else if (!genIsUnsignedType() && (repl["TYPE"].front() == 'u')) {
        repl["TYPE"] = repl["TYPE"].substr(1U);
    }

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkEnumField::wiresharkTvbRangeAccessImpl() const
{
    auto obj = genEnumFieldParseObj();
    return WiresharkIntField::wiresharkTvbRangeAccessIntegralValue(obj.parseType(), obj.parseEndian(), obj.parseMaxLength());
}

std::string WiresharkEnumField::wiresharkDissectLengthCheckImpl() const
{
    auto parseObj = genEnumFieldParseObj();

    if (parseObj.parseAvailableLengthLimit()) {
        return wiresharkEmptyBufferCheckCode();
    }

    return WiresharkBase::wiresharkDissectLengthCheckImpl();
}

std::string WiresharkEnumField::wiresharkDissectBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    static const std::string Templ =
        "local len = math.min(#^#LEN#$#, offset_limit - offset)\n"
        "local #^#RANGE#$# = tvb(offset, len)\n"
        "#^#VAL_DECL#$#\n"
        "#^#VAR_LEN#$#\n"
        "local #^#SUBTREE#$# = tree:add#^#SUFFIX#$#(field, #^#RANGE#$##^#VAL#$#)\n"
        "result = #^#SUCCESS#$#\n"
        "next_offset = offset + len\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genEnumFieldParseObj();
    bool hasVal = false;
    util::GenReplacementMap repl = {
        {"LEN", std::to_string(parseObj.parseMaxLength())},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
        {"VAR_LEN", wiresharkVarLengthCodeInternal(hasVal)},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"RANGE", wiresharkRangeStr()},
    };

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        repl["SUFFIX"] = strings::genLittleEndianSuffixStr();
    }

    if (hasVal) {
        repl["VAL_DECL"] = wiresharkValDeclCodeInternal();
        repl["VAL"] = ", val";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkEnumField::wiresharkValidFuncBodyImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local extractor = #^#MAP#$#[#^#FIELD#$#]\n"
        "local info = {extractor()}\n"
        "local last = info[#info]\n"
        "local name = #^#NAME#$##^#SUFFIX#$#[last.value]\n"
        "return name ~= #^#NIL#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldObjName(refField)},
        {"SUFFIX", strings::genValsSuffixStr()},
        {"NIL", strings::genNilStr()},
        {"MAP", Wireshark::wiresharkExtractorsMapName(wiresharkGenerator)},
        {"FIELD", wiresharkFieldStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkEnumField::wiresharkHasTrivialValidImpl() const
{
    return false;
}

std::string WiresharkEnumField::wiresharkValsInternal(const WiresharkField* refField) const
{
    auto& values = genSortedRevValues();
    assert(!values.empty());

    auto actValues = genEnumFieldParseObj().parseValues();
    util::GenStringsList elems;
    for (auto& v : values) {
        assert(v.second != nullptr);
        auto iter = actValues.find(*v.second);
        if (iter == actValues.end()) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        if (!genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "[#^#VAL#$#] = \"#^#NAME#$#\"";

        util::GenReplacementMap repl = {
            {"NAME", util::genDisplayName(iter->second.m_displayName, iter->first)},
            {"VAL", std::to_string(v.first)},
        };

        bool unsignedType = genIsUnsignedUnderlyingType();
        auto parseObj = genEnumFieldParseObj();
        if (parseObj.parseHexAssign() && unsignedType) {
            repl["VAL"] = wiresharkHexString(static_cast<std::uintmax_t>(v.first), static_cast<unsigned>(parseObj.parseMinLength() * 2U));
        }
        else if ((v.first < 0) && unsignedType) {
            repl["VAL"] = std::to_string(static_cast<std::uintmax_t>(v.first));
        }

        elems.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "local #^#NAME#$##^#SUFFIX#$# = {\n"
        "    #^#ELEMS#$#\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", wiresharkFieldObjName(refField)},
        {"SUFFIX",  strings::genValsSuffixStr()},
        {"ELEMS", util::genStrListToString(elems, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkEnumField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }
    return true;
}

std::string WiresharkEnumField::wiresharkValDeclCodeInternal() const
{
    auto parseObj = genEnumFieldParseObj();
    auto type = parseObj.parseType();
    if (WiresharkIntField::genIsVarLengthType(type)) {
        return "local val = 0\n";
    }

    static const std::string Templ =
        "local val = #^#RANGE#$#:#^#ACC#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"LEN", std::to_string(parseObj.parseMinLength())},
        {"ACC", WiresharkIntField::wiresharkTvbRangeAccessIntegralValue(parseObj.parseType(), parseObj.parseEndian(), parseObj.parseMinLength())},
        {"RANGE", wiresharkRangeStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkEnumField::wiresharkVarLengthCodeInternal(bool& hasVal) const
{
    auto parseObj = genEnumFieldParseObj();
    auto type = parseObj.parseType();
    if (!WiresharkIntField::genIsVarLengthType(type)) {
        return strings::genEmptyString();
    }

    hasVal = true;

    if (sizeof(std::uint32_t) < parseObj.parseMaxLength()) {
        return wiresharkVarLengthCodeLargeNumInternal();
    }

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        return wiresharkVarLengthCodeLittleEndianInternal();
    }

    return wiresharkVarLengthCodeBigEndianInternal();
}

std::string WiresharkEnumField::wiresharkVarLengthCodeLargeNumInternal() const
{
    // TODO: Implement and test
    assert(false);
    return strings::genEmptyString();
}

std::string WiresharkEnumField::wiresharkVarLengthCodeLittleEndianInternal() const
{
    // TODO: Implement
    return "-- TODO: Not implemented";
}

std::string WiresharkEnumField::wiresharkVarLengthCodeBigEndianInternal() const
{
    // TODO: test
    assert (false);

    static const std::string Templ =
        "local has_more = true\n"
        "while has_more and (next_offset < (offset + len)) do\n"
        "    local b = tvb(next_offset, 1):uint()\n"
        "    local data = bit32.band(b, 0x7F)\n"
        "    has_more = (bit32.band(b, 0x80) ~= 0)\n"
        "    val = bit32.bor(bit32.lshift(val, 7), data)\n"
        "    next_offset = next_offset + 1\n"
        "end\n"
        "\n"
        "if has_more then\n"
        "    return #^#ERROR#$#, offset\n"
        "end\n"
        "len = next_offset - offset\n"
        "#^#RANGE#$# = tvb(offset, len)\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"ERROR", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::MalformedPacket)},
        {"RANGE", wiresharkRangeStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2wireshark
