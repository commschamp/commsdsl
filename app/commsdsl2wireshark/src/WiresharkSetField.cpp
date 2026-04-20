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

#include "WiresharkSetField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"
#include "WiresharkIntField.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <limits>
#include <sstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkSetField::WiresharkSetField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

std::string WiresharkSetField::wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const
{
    if ((refField != nullptr) && (!refField->wiresharkIsBitfieldMember())) {
        // No need for bits info for non-bitfield member <ref> field.
        return WiresharkBase::wiresharkExtractorsRegCodeImpl(refField);
    }

    GenStringsList elems;

    auto parseObj = genSetFieldParseObj();
    auto& bits = parseObj.parseBits();

    auto refName = wiresharkFieldRefName(refField);
    auto parentWidth = wiresharkBitParentWidthInternal(refField);

    for (const auto& bitInfo : bits) {
        if (!genGenerator().genDoesElementExist(bitInfo.second.m_sinceVersion, bitInfo.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "#^#REG_FUNC#$#(\"#^#REF_NAME#$#\", #^#FIELD#$#)\n"
            ;

        auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
        util::GenReplacementMap repl = {
            {"REG_FUNC", Wireshark::wiresharkCreateExtractorFuncName(wiresharkGenerator)},
            {"REF_NAME", refName + '.' + bitInfo.first},
            {"FIELD", wiresharkBitObjNameInternal(refField, bitInfo.first)},
        };

        elems.push_back(util::genProcessTemplate(Templ, repl));
    }

    return WiresharkBase::wiresharkExtractorsRegCodeImpl(refField) + util::genStrListToString(elems, "", "");
}

std::string WiresharkSetField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, base.HEX, #^#NIL#$#, #^#MASK#$#, #^#DESC#$#))\n"
        "#^#BITS#$#\n"
    ;

    auto parseObj = genSetFieldParseObj();
    util::GenReplacementMap repl = {
        {"BITS", wiresharkRegistrationBitsInternal(refField)},
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkForcedIntegralFieldType(refField)},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"MASK", wiresharkForcedIntegralFieldMask(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
        {"NIL", strings::genNilStr()},
    };

    if (repl["TYPE"].empty()) {
        repl["TYPE"] = WiresharkIntField::wiresharkIntegralType(parseObj.parseType(), parseObj.parseMaxLength());
    }

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSetField::wiresharkDissectLengthCheckImpl(const WiresharkField* refField) const
{
    auto parseObj = genSetFieldParseObj();

    if (parseObj.parseAvailableLengthLimit()) {
        return wiresharkEmptyBufferCheckCode();
    }

    return WiresharkBase::wiresharkDissectLengthCheckImpl(refField);
}

std::string WiresharkSetField::wiresharkDissectBodyImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local len = math.min(#^#LEN#$#, #^#LIMIT#$# - #^#OFFSET#$#)\n"
        "local #^#RANGE#$# = #^#TVB#$#(#^#OFFSET#$#, len)\n"
        "local #^#SUBTREE#$# = #^#TREE#$#:add#^#SUFFIX#$#(#^#FIELD#$#, #^#RANGE#$#)\n"
        "#^#BITS#$#\n"
        "#^#RESULT#$# = #^#SUCCESS#$#\n"
        "#^#NEXT_OFFSET#$# = #^#OFFSET#$# + len\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genSetFieldParseObj();
    util::GenReplacementMap repl = {
        {"LEN", std::to_string(wiresharkMinFieldLength(refField))},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"TVB", wiresharkTvbStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"RESULT", wiresharkResultStr()},
        {"TREE", wiresharkTreeStr()},
        {"LIMIT", wiresharkOffsetLimitStr()},
        {"FIELD", wiresharkFieldStr()},
        {"RANGE", wiresharkRangeStr()},
    };

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        repl["SUFFIX"] = strings::genLittleEndianSuffixStr();
    }

    util::GenStringsList elems;
    auto& revBits = parseObj.parseRevBits();

    for (auto& b : revBits) {
        static const std::string BitTempl =
            "#^#SUBTREE#$#:add#^#SUFFIX#$#(#^#FIELD#$#, range)"
            ;

        util::GenReplacementMap bitRepl = repl;
        bitRepl["FIELD"] = wiresharkBitObjNameInternal(refField, b.second);
        elems.push_back(util::genProcessTemplate(BitTempl, bitRepl));
    }

    repl["BITS"] = util::genStrListToString(elems, "\n", "");
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSetField::wiresharkValidFuncBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    auto parseObj = genSetFieldParseObj();
    auto bitsCount = parseObj.parseMaxLength() * 8U;

    std::uintmax_t reservedMask = std::numeric_limits<std::uintmax_t>::max();
    std::uintmax_t reservedValue = 0;

    if (bitsCount < std::numeric_limits<decltype(reservedMask)>::digits) {
        reservedMask = (1ULL << bitsCount) - 1U;
    }

    if (parseObj.parseReservedBitValue()) {
        reservedValue = std::numeric_limits<std::uintmax_t>::max() & reservedMask;
    }

    auto& bits = parseObj.parseBits();
    for (auto& info : bits) {
        auto bitMask = (1ULL << info.second.m_idx);
        auto clearMask = (~bitMask);

        if (!info.second.m_reserved) {
            reservedMask = static_cast<decltype(reservedMask)>(reservedMask & clearMask);
            reservedValue = static_cast<decltype(reservedValue)>(reservedValue & clearMask);
            continue;
        }

        reservedMask = static_cast<decltype(reservedMask)>(reservedMask | bitMask);
        if (info.second.m_reservedValue) {
            reservedValue = static_cast<decltype(reservedValue)>(reservedValue | bitMask);
            continue;
        }

        reservedValue = static_cast<decltype(reservedValue)>(reservedValue & clearMask);
    }

    if (reservedMask == 0U) {
        return "return true";
    }

    static const std::string Templ =
        "local value = #^#FUNC#$#(#^#FIELD#$#)\n"
        "return bit32.band(value, #^#MASK#$#) == #^#VAL#$#, true\n"
        ;

    util::GenReplacementMap repl = {
        {"SUFFIX", strings::genValsSuffixStr()},
        {"FUNC", wiresharkValueFuncName()},
        {"FIELD", wiresharkFieldStr()},
        {"MASK", wiresharkHexString(reservedMask, 2U)},
        {"VAL", wiresharkHexString(reservedValue, 2U)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSetField::wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    if (accStr.empty()) {
        return WiresharkBase::wiresharkValueAccessStrImpl(accStr, refField);
    }

    auto& bits = genSetFieldParseObj().parseBits();
    auto iter = bits.find(accStr);
    if (iter == bits.end()) {
        genGenerator().genLogger().genError("Failed to find bit reference " + accStr + " for field " + genParseObj().parseInnerRef());
        assert(false);
        return WiresharkBase::wiresharkValueAccessStrImpl(std::string(), refField);
    }

    static const std::string Templ =
        "#^#VALUE_FUNC#$#(#^#NAME#$#)"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    util::GenReplacementMap repl = {
        {"VALUE_FUNC", Wireshark::wiresharkFieldValueFuncName(wiresharkGenerator)},
        {"NAME", wiresharkBitObjNameInternal(refField, accStr)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSetField::wiresharkDefaultAssignmentsImpl(const WiresharkField* refField) const
{
    auto parseObj = genSetFieldParseObj();
    static const std::string Templ =
        "#^#TREE#$#:add(#^#FIELD#$#, #^#TVB#$#(#^#OFFSET#$#, 0), #^#VAL#$#):set_hidden(true)\n"
        ;

    std::uintmax_t val = 0U;
    std::uintmax_t mask = std::numeric_limits<std::uintmax_t>::max();
    auto bitLength = parseObj.parseBitLength();
    if (bitLength == 0U) {
        bitLength = parseObj.parseMaxLength() * std::numeric_limits<std::uint8_t>::digits;
    }

    if (bitLength < std::numeric_limits<decltype(mask)>::digits) {
        mask = (1ULL << bitLength) - 1U;
    }

    if (parseObj.parseDefaultBitValue()) {
        val = (~val) & mask;
    }

    util::GenReplacementMap repl = {
        {"TREE", wiresharkTreeStr()},
        {"FIELD", wiresharkFieldObjName(refField)},
        {"TVB", wiresharkTvbStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"VAL", wiresharkHexString(val, 2U)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSetField::wiresharkCompPrepValueStrImpl(const std::string& value) const
{
    return wiresharkProcessIntegralValue(value);
}

bool WiresharkSetField::wiresharkHasTrivialValidImpl() const
{
    auto parseObj = genSetFieldParseObj();
    auto& bits = parseObj.parseBits();
    if (bits.size() < (parseObj.parseMaxLength() * 8U)) {
        return false;
    }

    bool hasReservedBits =
        std::any_of(
            bits.begin(), bits.end(),
            [](auto& info)
            {
                return info.second.m_reserved;
            });

    return (!hasReservedBits);
}

std::string WiresharkSetField::wiresharkRegistrationBitsInternal(const WiresharkField* refField) const
{
    if ((refField != nullptr) && (!refField->wiresharkIsBitfieldMember())) {
        // No need for bits info for non-bitfield member <ref> field.
        return strings::genEmptyString();
    }

    GenStringsList elems;

    auto parseObj = genSetFieldParseObj();
    auto& bits = parseObj.parseBits();
    auto& revBits = parseObj.parseRevBits();

    auto refName = wiresharkFieldRefName(refField);
    auto parentWidth = wiresharkBitParentWidthInternal(refField);

    for (auto b : revBits) {
        auto iter = bits.find(b.second);
        assert(iter != bits.end());
        if (iter == bits.end()) {
            continue;
        }

        auto& bitInfo = *iter;

        if (!genGenerator().genDoesElementExist(bitInfo.second.m_sinceVersion, bitInfo.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "local #^#BIT_OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.bool(\"#^#REF_NAME#$#\", \"#^#DISP_NAME#$#\", #^#PARENT_WIDTH#$#, #^#NIL#$#, #^#MASK#$#, #^#DESC#$#))\n"
        ;

        auto forcedShift = wiresharkForcedMaskShift(refField);
        util::GenReplacementMap repl = {
            {"BIT_OBJ_NAME", wiresharkBitObjNameInternal(refField, bitInfo.first)},
            {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
            {"REF_NAME", refName + '.' + bitInfo.first},
            {"DISP_NAME", util::genDisplayName(bitInfo.second.m_displayName, bitInfo.first)},
            {"MASK", wiresharkBitMaskInternal(bitInfo.second.m_idx + forcedShift)},
            {"DESC", strings::genNilStr()},
            {"NIL", strings::genNilStr()},
            {"PARENT_WIDTH", parentWidth},
        };

        if (!bitInfo.second.m_description.empty()) {
            repl["DESC"] = '\"' + bitInfo.second.m_description + '\"';
        }

        elems.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(elems, "", "");
}

std::string WiresharkSetField::wiresharkBitMaskInternal(unsigned idx) const
{
    auto mask = 1U << idx;
    auto parseObj = genSetFieldParseObj();
    std::stringstream stream;
    stream << std::hex << "0x" << std::uppercase <<
        std::setfill('0') << std::setw(static_cast<int>(parseObj.parseMaxLength() * 2U)) << mask;
    return stream.str();
}

std::string WiresharkSetField::wiresharkBitParentWidthInternal(const WiresharkField* refField) const
{
    return std::to_string(std::max(genParseObj().parseMaxLength() * 8U, static_cast<std::size_t>(wiresharkForcedBitLength(refField))));
}

std::string WiresharkSetField::wiresharkBitObjNameInternal(const WiresharkField* refField, const std::string& bitName) const
{
    return wiresharkFieldObjName(refField) + '_' + bitName;
}

bool WiresharkSetField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }
    return true;
}

} // namespace commsdsl2wireshark
