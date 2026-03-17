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

std::string WiresharkSetField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, base.HEX, #^#NIL#$#, #^#MASK#$#, #^#DESC#$#))\n"
        "#^#BITS#$#\n"
    ;

    auto parseObj = genSetFieldParseObj();
    util::GenReplacementMap repl = {
        {"BITS", wiresharkBitsInternal(refField)},
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

std::string WiresharkSetField::wiresharkDissectLengthCheckImpl() const
{
    auto parseObj = genSetFieldParseObj();

    if (parseObj.parseAvailableLengthLimit()) {
        return wiresharkEmptyBufferCheckCode();
    }

    return WiresharkBase::wiresharkDissectLengthCheckImpl();
}

std::string WiresharkSetField::wiresharkDissectBodyImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local len = math.min(#^#LEN#$#, offset_limit - offset)\n"
        "local range = tvb(offset, len)\n"
        "local bits_tree = tree:add#^#SUFFIX#$#(field, range)\n"
        "#^#BITS#$#\n"
        "result = #^#SUCCESS#$#\n"
        "next_offset = offset + len\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genSetFieldParseObj();
    util::GenReplacementMap repl = {
        {"LEN", std::to_string(parseObj.parseMaxLength())},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::StatusCode::Success)},
    };

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        repl["SUFFIX"] = strings::genLittleEndianSuffixStr();
    }

    util::GenStringsList elems;
    auto& revBits = parseObj.parseRevBits();

    for (auto& b : revBits) {
        static const std::string BitTempl =
            "bits_tree:add#^#SUFFIX#$#(#^#FIELD#$#, range)"
            ;

        util::GenReplacementMap bitRepl = repl;
        bitRepl["FIELD"] = wiresharkBitObjName(refField, b.second);
        elems.push_back(util::genProcessTemplate(BitTempl, bitRepl));
    }

    repl["BITS"] = util::genStrListToString(elems, "\n", "");
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSetField::wiresharkBitsInternal(const WiresharkField* refField) const
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
            {"BIT_OBJ_NAME", wiresharkBitObjName(refField, bitInfo.first)},
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

std::string WiresharkSetField::wiresharkBitObjName(const WiresharkField* refField, const std::string& bitName) const
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
