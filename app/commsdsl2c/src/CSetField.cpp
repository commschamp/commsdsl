//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "CSetField.h"

#include "CGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <limits>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2c
{

namespace
{

const auto CMaxBits = std::numeric_limits<std::uintmax_t>::digits;
const std::string BitIdxStr = "BitIdx";

const std::string& cCodeTemplInternal()
{
    static const std::string Templ =
        "#^#VALUE#$#\n"
        "#^#BITS#$#\n"
        ;

    return Templ;
}

} // namespace

CSetField::CSetField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CSetField::genWriteImpl() const
{
    return cWrite();
}

void CSetField::cAddHeaderIncludesImpl(CIncludesList& includes) const
{
    includes.push_back("<stdint.h>");
}

std::string CSetField::cHeaderCodeImpl() const
{
    util::GenReplacementMap repl = {
        {"VALUE", cHeaderValueCodeInternal()},
        {"BITS", cHeaderBitsCodeInternal()},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CSetField::cSourceCodeImpl() const
{
    util::GenReplacementMap repl = {
        {"VALUE", cSourceCommonValueAccessFuncs()},
        {"BITS", cSourceBitsCodeInternal()},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CSetField::cTypeInternal() const
{
    auto parseObj = genSetFieldParseObj();
    auto cppType = comms::genCppIntTypeFor(parseObj.parseType(), parseObj.parseMaxLength());
    return util::genStrReplace(cppType, "std::", std::string());
}

std::string CSetField::cHeaderValueCodeInternal() const
{
    static const std::string Templ =
        "/// @brief Inner value storage type of @ref #^#NAME#$#.\n"
        "typedef #^#TYPE#$# #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$#;\n"
        "\n"
        "#^#FUNCS#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"TYPE", cTypeInternal()},
        {"VALUE_TYPE", strings::genValueTypeStr()},
        {"FUNCS", cHeaderCommonValueAccessFuncs()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CSetField::cHeaderBitsCodeInternal() const
{
    auto parseObj = genSetFieldParseObj();
    util::GenStringsList assigns;
    util::GenStringsList accesses;

    auto& bits = parseObj.parseBits();
    for (auto& bitInfo : parseObj.parseRevBits()) {
        auto idx = bitInfo.first;
        if (CMaxBits <= idx) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        auto iter = bits.find(bitInfo.second);
        assert(iter != bits.end());

        if (!genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, false)) {
            continue;
        }

        static const std::string BitTempl =
            "#^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$#_#^#BIT_NAME#$# = #^#IDX#$#, ///< Bit <b>#^#DISP_NAME#$#</b>"
            ;

        util::GenReplacementMap bitRepl = {
            {"NAME", cName()},
            {"BIT_IDX", BitIdxStr},
            {"BIT_NAME", bitInfo.second},
            {"IDX", util::genNumToString(idx)},
            {"DISP_NAME", util::genDisplayName(iter->second.m_displayName, bitInfo.second)},
        };

        if (cIsVersionOptional()) {
            bitRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
        }

        assigns.push_back(util::genProcessTemplate(BitTempl, bitRepl));

        static const std::string AccTempl =
            "/// @brief Get value of the <b>#^#DISP_NAME#$#</b> bit in the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
            "bool #^#NAME#$##^#SUFFIX#$#_getBitValue_#^#BIT_NAME#$#(const #^#NAME#$##^#SUFFIX#$#* field);\n"
            "\n"
            "/// @brief Set value of the <b>#^#DISP_NAME#$#</b> bit in the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
            "void #^#NAME#$##^#SUFFIX#$#_setBitValue_#^#BIT_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field, bool bitVal);\n"
            ;

        accesses.push_back(util::genProcessTemplate(AccTempl, bitRepl));
    }

    util::GenStringsList masks;
    for (auto& maskInfo : parseObj.parseMasks()) {
        std::uintmax_t maskVal = 0U;
        for (auto& b : maskInfo.second.m_bits) {
            auto iter = bits.find(b);
            assert(iter != bits.end());
            if (iter == bits.end()) {
                continue;
            }

            if (!genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, false)) {
                continue;
            }

            maskVal |= static_cast<decltype(maskVal)>(1) << iter->second.m_idx;
        }

        if (maskVal == 0U) {
            continue;
        }

        static const std::string MaskTempl =
            "/// @brief #^#BRIEF#$#\n"
            "#^#DETAILS#$#\n"
            "#define #^#NAME#$##^#SUFFIX#$#_BitMask_#^#MASK_NAME#$# ((#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$#)#^#VAL#$#)\n";

        util::GenReplacementMap maskRepl = {
            {"NAME", cName()},
            {"MASK_NAME", maskInfo.first},
            {"VALUE_TYPE", strings::genValueTypeStr()},
            {"VAL", util::genNumToString(maskVal, static_cast<unsigned>(parseObj.parseMinLength() * 2U))},
        };

        if (cIsVersionOptional()) {
            maskRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
        }

        do {
            if (maskInfo.second.m_description.empty()) {
                maskRepl["BRIEF"] = "Mask " + maskInfo.first;
                break;
            }

            auto dotPos = maskInfo.second.m_description.find_first_of(".");
            if (dotPos == std::string::npos) {
                maskRepl["BRIEF"] = maskInfo.second.m_description;
                break;
            }

            maskRepl["BRIEF"] = maskInfo.second.m_description.substr(0, dotPos + 1);
            auto details = util::genStrMakeMultiline(maskInfo.second.m_description.substr(dotPos + 1));
            if (details.empty()) {
                break;
            }

            util::genRemoveHeadingTrailingWhitespaces(details);

            static const std::string DetailsTempl =
                "/// @details #^#TEXT#$#"
                ;

            util::GenReplacementMap detailsRepl = {
                {"TEXT", util::genStrReplace(details, "\n", "///     ")},
            };

            maskRepl["DETAILS"] = util::genProcessTemplate(DetailsTempl, detailsRepl);
        } while (false);

        masks.push_back(util::genProcessTemplate(MaskTempl, maskRepl));
    }

    static const std::string Templ =
        "/// @brief Bit access enumerator for @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "typedef enum\n"
        "{\n"
        "    #^#ASSIGNS#$#\n"
        "    #^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$#_numOfValues ///< Upper limit of available bits\n"
        "} #^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$#;\n"
        "\n"
        "#^#MASKS#$#\n"
        "/// @brief Check all bits of the requested mask are set in the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_hasAllBitsSet(const #^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask);\n"
        "\n"
        "/// @brief Check any bits of the requested mask are set in the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_hasAnyBitsSet(const #^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask);\n"
        "\n"
        "/// @brief Set all bits of the requested mask in the @ref #^#NAME#$##^#SUFFIX#$# field value.\n"
        "void #^#NAME#$##^#SUFFIX#$#_setBits(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask);\n"
        "\n"
        "/// @brief Clear all bits of the requested mask in the @ref #^#NAME#$##^#SUFFIX#$# field value.\n"
        "void #^#NAME#$##^#SUFFIX#$#_clearBits(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask);\n"
        "\n"
        "/// @brief Get bit value of @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_getBitValue(const #^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$# bit);\n"
        "\n"
        "/// @brief Set bit value of @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "void #^#NAME#$##^#SUFFIX#$#_setBitValue(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$# bit, bool bitValue);\n"
        "\n"
        "/// @brief Convert bit index of the @ref #^#NAME#$##^#SUFFIX#$# field to a mask.\n"
        "#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# #^#NAME#$##^#SUFFIX#$#_bitAsMask(#^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$# bit);\n"
        "#^#ACCESSES#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"BIT_IDX", BitIdxStr},
        {"ASSIGNS", util::genStrListToString(assigns, "\n", "")},
        {"ACCESSES", util::genStrListToString(accesses, "\n", "")},
        {"MASKS", util::genStrListToString(masks, "\n", "\n")},
        {"VALUE_TYPE", strings::genValueTypeStr()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CSetField::cSourceBitsCodeInternal() const
{
    auto parseObj = genSetFieldParseObj();
    util::GenStringsList asserts;
    util::GenStringsList accesses;

    for (auto& bitInfo : parseObj.parseRevBits()) {
        auto idx = bitInfo.first;
        if (CMaxBits <= idx) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        static const std::string AssertTempl =
            "static_assert(static_cast<unsigned>(#^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$#_#^#BIT_NAME#$#) == static_cast<unsigned>(#^#NAME#$##^#SUFFIX#$##^#COMMS#$#::BitIdx_#^#BIT_NAME#$#), \"Bit index mismatch\");"
            ;

        util::GenReplacementMap bitRepl = {
            {"NAME", cName()},
            {"BIT_IDX", BitIdxStr},
            {"BIT_NAME", bitInfo.second},
            {"COMMS", strings::genCommsNameSuffixStr()},
            {"CONV_SUFFIX", cConversionSuffix()},
        };

        if (cIsVersionOptional()) {
            bitRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
        }

        asserts.push_back(util::genProcessTemplate(AssertTempl, bitRepl));

        static const std::string AccTempl =
            "bool #^#NAME#$##^#SUFFIX#$#_getBitValue_#^#BIT_NAME#$#(const #^#NAME#$##^#SUFFIX#$#* field)\n"
            "{\n"
            "    return from#^#CONV_SUFFIX#$#(field)->getBitValue_#^#BIT_NAME#$#();\n"
            "}\n"
            "\n"
            "void #^#NAME#$##^#SUFFIX#$#_setBitValue_#^#BIT_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field, bool bitVal)\n"
            "{\n"
            "    from#^#CONV_SUFFIX#$#(field)->setBitValue_#^#BIT_NAME#$#(bitVal);\n"
            "}\n"
            ;

        accesses.push_back(util::genProcessTemplate(AccTempl, bitRepl));
    }

    static const std::string Templ =
        "#^#ASSERTS#$#\n"
        "bool #^#NAME#$##^#SUFFIX#$#_hasAllBitsSet(const #^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->hasAllBitsSet(mask);\n"
        "}\n"
        "\n"
        "bool #^#NAME#$##^#SUFFIX#$#_hasAnyBitsSet(const #^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->hasAnyBitsSet(mask);\n"
        "}\n"
        "\n"
        "void #^#NAME#$##^#SUFFIX#$#_setBits(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask)\n"
        "{\n"
        "    from#^#CONV_SUFFIX#$#(field)->setBits(mask);\n"
        "}\n"
        "\n"
        "void #^#NAME#$##^#SUFFIX#$#_clearBits(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# mask)\n"
        "{\n"
        "    from#^#CONV_SUFFIX#$#(field)->clearBits(mask);\n"
        "}\n"
        "\n"
        "bool #^#NAME#$##^#SUFFIX#$#_getBitValue(const #^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$# bit)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->getBitValue(static_cast<#^#NAME#$##^#SUFFIX#$##^#COMMS#$#::BitIdx>(bit));\n"
        "}\n"
        "\n"
        "void #^#NAME#$##^#SUFFIX#$#_setBitValue(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$# bit, bool bitValue)\n"
        "{\n"
        "    from#^#CONV_SUFFIX#$#(field)->setBitValue(static_cast<#^#NAME#$##^#SUFFIX#$##^#COMMS#$#::BitIdx>(bit), bitValue);\n"
        "}\n"
        "\n"
        "#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# #^#NAME#$##^#SUFFIX#$#_bitAsMask(#^#NAME#$##^#SUFFIX#$#_#^#BIT_IDX#$# bit)\n"
        "{\n"
        "    return static_cast<#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$#>(#^#COMMS_TYPE#$#::bitAsMask(bit));\n"
        "}\n"
        "\n"
        "#^#ACCESSES#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"BIT_IDX", BitIdxStr},
        {"ASSERTS", util::genStrListToString(asserts, "\n", "\n")},
        {"ACCESSES", util::genStrListToString(accesses, "\n", "")},
        {"CONV_SUFFIX", cConversionSuffix()},
        {"COMMS", strings::genCommsNameSuffixStr()},
        {"VALUE_TYPE", strings::genValueTypeStr()},
        {"COMMS_TYPE", cCommsTypeName(cIsVersionOptional())},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
