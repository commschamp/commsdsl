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

std::string WiresharkSetField::wiresharkFieldRegistrationImpl(const std::string& objName, const std::string& refName) const
{
    static const std::string Templ =
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", \"#^#DISP_NAME#$#\", base.HEX, #^#NIL#$#, #^#MASK#$#, #^#DESC#$#))\n"
        "#^#BITS#$#\n"
    ;

    auto parseObj = genSetFieldParseObj();
    util::GenReplacementMap repl = {
        {"BITS", wiresharkBitsInternal()},
        {"OBJ_NAME", objName},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkForcedIntegralFieldType()},
        {"REF_NAME", refName},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"MASK", wiresharkForcedIntegralFieldMask()},
        {"DESC", wiresharkFieldDescriptionStr()},
        {"NIL", strings::genNilStr()},
    };

    if (repl["OBJ_NAME"].empty()) {
        repl["OBJ_NAME"] = wiresharkFieldObjName();
    }

    if (repl["REF_NAME"].empty()) {
        repl["REF_NAME"] = wiresharkFieldRefName();
    }

    if (repl["TYPE"].empty()) {
        repl["TYPE"] = WiresharkIntField::wiresharkIntegralType(parseObj.parseType(), parseObj.parseMaxLength());
    }

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkSetField::wiresharkBitsInternal() const
{
    GenStringsList elems;

    auto parseObj = genSetFieldParseObj();
    auto& bits = parseObj.parseBits();
    auto& revBits = parseObj.parseRevBits();

    auto refName = wiresharkFieldRefName();
    auto parentWidth = wiresharkBitParentWidthInternal();

    for (auto b : revBits) {
        auto iter = bits.find(b.second);
        assert(iter != bits.end());
        if (iter == bits.end()) {
            continue;
        }

        auto& bitInfo = *iter;

        static const std::string Templ =
            "local #^#BIT_OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.bool(\"#^#REF_NAME#$#\", \"#^#DISP_NAME#$#\", #^#PARENT_WIDTH#$#, #^#NIL#$#, #^#MASK#$#, #^#DESC#$#))\n"
        ;

        auto forcedShift = wiresharkForcedMaskShift();
        util::GenReplacementMap repl = {
            {"BIT_OBJ_NAME", wiresharkBitObjName(bitInfo.first)},
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

std::string WiresharkSetField::wiresharkBitParentWidthInternal() const
{
    return std::to_string(std::max(genParseObj().parseMaxLength() * 8U, static_cast<std::size_t>(wiresharkForcedBitLength())));
}

std::string WiresharkSetField::wiresharkBitObjName(const std::string& bitName) const
{
    return wiresharkFieldObjName() + '_' + bitName;
}

} // namespace commsdsl2wireshark
