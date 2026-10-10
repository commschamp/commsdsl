//
// Copyright 2022 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenSetField.h"

#include "EmscriptenGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

EmscriptenSetField::EmscriptenSetField(EmscriptenGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    EmscriptenBase(static_cast<GenBase&>(*this))
{
}

bool EmscriptenSetField::genWriteImpl() const
{
    return emscriptenWrite();
}

std::string EmscriptenSetField::emscriptenHeaderValueAccImpl() const
{
    return emscriptenHeaderValueAccByValue();
}

std::string EmscriptenSetField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    auto obj = genSetFieldParseObj();

    util::GenStringsList accesses;

    for (auto& bitInfo : obj.parseRevBits()) {

        static const std::string Templ =
            "bool getBitValue_#^#NAME#$#() const\n"
            "{\n"
            "    return Base::getBitValue_#^#NAME#$#();\n"
            "}\n\n"
            "void setBitValue_#^#NAME#$#(bool val)\n"
            "{\n"
            "    Base::setBitValue_#^#NAME#$#(val);\n"
            "}\n";

        util::GenReplacementMap repl = {
            {"NAME", bitInfo.second}
        };

        accesses.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "bool hasAllBitsSet(ValueType mask) const\n"
        "{\n"
        "    return Base::hasAllBitsSet(mask);\n"
        "}\n\n"
        "bool hasAnyBitsSet(ValueType mask) const\n"
        "{\n"
        "    return Base::hasAnyBitsSet(mask);\n"
        "}\n\n"
        "void setBits(ValueType mask)\n"
        "{\n"
        "    Base::setBits(mask);\n"
        "}\n\n"
        "void clearBits(ValueType mask)\n"
        "{\n"
        "    Base::clearBits(mask);\n"
        "}\n\n"
        "bool getBitValue(unsigned bitIdx) const\n"
        "{\n"
        "    return Base::getBitValue(static_cast<Base::BitIdx>(bitIdx));\n"
        "}\n\n"
        "void setBitValue(unsigned bitIdx, bool val)\n"
        "{\n"
        "    Base::setBitValue(static_cast<Base::BitIdx>(bitIdx), val);\n"
        "}\n\n"
        "static ValueType bitAsMask(unsigned bitIdx)\n"
        "{\n"
        "    return Base::bitAsMask(bitIdx);\n"
        "}\n\n"
        "#^#ACCESS_FUNCS#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"ACCESS_FUNCS", util::genStrListToString(accesses, "\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenSetField::emscriptenSourceBindFuncsImpl() const
{
    auto obj = genSetFieldParseObj();

    util::GenReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
    };

    util::GenStringsList accesses;
    for (auto& bitInfo : obj.parseRevBits()) {

        static const std::string Templ =
            ".function(\"getBitValue_#^#NAME#$#\", &#^#CLASS_NAME#$#::getBitValue_#^#NAME#$#)\n"
            ".function(\"setBitValue_#^#NAME#$#\", &#^#CLASS_NAME#$#::setBitValue_#^#NAME#$#)";

        repl["NAME"] = bitInfo.second;
        accesses.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "#^#ACCESS_FUNCS#$#\n"
        ".function(\"hasAllBitsSet\", &#^#CLASS_NAME#$#::hasAllBitsSet)\n"
        ".function(\"hasAnyBitsSet\", &#^#CLASS_NAME#$#::hasAnyBitsSet)\n"
        ".function(\"setBits\", &#^#CLASS_NAME#$#::setBits)\n"
        ".function(\"clearBits\", &#^#CLASS_NAME#$#::clearBits)\n"
        ".function(\"getBitValue\", &#^#CLASS_NAME#$#::getBitValue)\n"
        ".function(\"setBitValue\", &#^#CLASS_NAME#$#::setBitValue)\n"
        ".class_function(\"bitAsMask\", &#^#CLASS_NAME#$#::bitAsMask)"
        ;

    repl["ACCESS_FUNCS"] = util::genStrListToString(accesses, "\n", "");
    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenSetField::emscriptenSourceBindExtraImpl() const
{
    auto obj = genSetFieldParseObj();
    auto& bits = obj.parseBits();

    util::GenStringsList values;
    for (auto& bitInfo : obj.parseRevBits()) {
        auto iter = bits.find(bitInfo.second);
        assert(iter != bits.end());
        if (iter == bits.end()) {
            continue;
        }

        if (!genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, false)) {
            continue;
        }

        static const std::string BitIdxTempl =
            ".value(\"#^#NAME#$#\", #^#CLASS_NAME#$#::BitIdx_#^#NAME#$#)";

        util::GenReplacementMap bitIdxRepl = {
            {"CLASS_NAME", emscriptenBindClassName()},
            {"NAME", bitInfo.second},
        };

        values.push_back(util::genProcessTemplate(BitIdxTempl, bitIdxRepl));
    }

    util::GenStringsList masks;
    for (auto& maskInfo : obj.parseMasks()) {
        bool hasValidBits =
            std::any_of(
                maskInfo.second.m_bits.begin(), maskInfo.second.m_bits.end(),
                [this, &bits](auto& bitName)
                {
                    auto iter = bits.find(bitName);
                    assert(iter != bits.end());
                    if (iter == bits.end()) {
                        return false;
                    }

                    return genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, false);
                });

        if (!hasValidBits) {
            continue;
        }

        static const std::string MaskTempl =
            "emscripten::constant(\"#^#CLASS_NAME#$#_BitMask_#^#NAME#$#\", static_cast<#^#CLASS_NAME#$#::#^#VALUE_TYPE#$#>(#^#CLASS_NAME#$#::BitMask_#^#NAME#$#));";

        util::GenReplacementMap maskRepl = {
            {"CLASS_NAME", emscriptenBindClassName()},
            {"NAME", maskInfo.first},
            {"VALUE_TYPE", strings::genValueTypeStr()},
        };

        masks.push_back(util::genProcessTemplate(MaskTempl, maskRepl));
    }

    static const std::string Templ =
        "emscripten::enum_<#^#CLASS_NAME#$#::BitIdx>(\"#^#CLASS_NAME#$#_BitIdx\")\n"
        "    #^#VALUES#$#\n"
        "    .value(\"BitIdx_numOfValues\", #^#CLASS_NAME#$#::BitIdx_numOfValues)\n"
        "   ;\n"
        "#^#MASKS#$#\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
        {"VALUES", util::genStrListToString(values, "\n", "")},
        {"MASKS", util::genStrListToString(masks, "\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten
