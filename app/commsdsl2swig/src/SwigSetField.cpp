//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "SwigSetField.h"

#include "SwigGenerator.h"
#include "SwigIntField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <cstdint>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

SwigSetField::SwigSetField(SwigGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    SwigBase(static_cast<GenBase&>(*this))
{
}

bool SwigSetField::genWriteImpl() const
{
    return swigWrite();
}

std::string SwigSetField::swigValueTypeDeclImpl() const
{
    static const std::string Templ =
        "using ValueType = #^#TYPE#$#;\n";

    auto obj = genSetFieldParseObj();
    util::GenReplacementMap repl = {
        {"TYPE", SwigGenerator::swigCast(genGenerator()).swigConvertIntType(obj.parseType(), obj.parseMaxLength())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigSetField::swigExtraPublicFuncsDeclImpl() const
{
    auto obj = genSetFieldParseObj();
    auto& bits = obj.parseBits();

    util::GenStringsList indices;
    util::GenStringsList accesses;

    for (auto& bitInfo : obj.parseRevBits()) {
        auto iter = bits.find(bitInfo.second);
        assert(iter != bits.end());
        if (iter == bits.end()) {
            continue;
        }

        if (!genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, false)) {
            continue;
        }

        indices.push_back("BitIdx_" + bitInfo.second + " = " + std::to_string(bitInfo.first));

        static const std::string Templ =
            "bool getBitValue_#^#NAME#$#() const;\n"
            "void setBitValue_#^#NAME#$#(bool val);"
            ;

        util::GenReplacementMap repl = {
            {"NAME", bitInfo.second}
        };

        accesses.push_back(util::genProcessTemplate(Templ, repl));
    }

    util::GenStringsList masks;
    for (auto& maskInfo : obj.parseMasks()) {
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
            "static constexpr ValueType BitMask_#^#NAME#$# = #^#VAL#$#;";

        util::GenReplacementMap maskRepl = {
            {"NAME", maskInfo.first},
            {"VAL", util::genNumToString(maskVal, static_cast<unsigned>(obj.parseMinLength() * 2U))},
        };

        masks.push_back(util::genProcessTemplate(MaskTempl, maskRepl));
    }

    static const std::string Templ =
        "enum BitIdx : unsigned\n"
        "{\n"
        "    #^#INDICES#$#\n"
        "    BitIdx_numOfValues\n"
        "};\n\n"
        "#^#MASKS#$#\n"
        "bool hasAllBitsSet(ValueType mask);\n"
        "bool hasAnyBitsSet(ValueType mask);\n"
        "void setBits(ValueType mask);\n"
        "void clearBits(ValueType mask);\n"
        "bool getBitValue(unsigned bitNum) const;\n"
        "void setBitValue(unsigned bitNum, bool val);\n"
        "static ValueType bitAsMask(BitIdx idx);\n"
        "#^#ACCESS_FUNCS#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"INDICES", util::genStrListToString(indices, ",\n", ",")},
        {"ACCESS_FUNCS", util::genStrListToString(accesses, "\n", "")},
        {"MASKS", util::genStrListToString(masks, "\n", "\n")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2swig
