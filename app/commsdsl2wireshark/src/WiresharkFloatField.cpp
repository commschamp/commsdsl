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

#include "WiresharkFloatField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <cstddef>
#include <cmath>
#include <type_traits>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkFloatField::WiresharkFloatField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkFloatField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }
    return true;
}

std::string WiresharkFloatField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.#^#TYPE#$#(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, #^#UNIT_NAME#$#, #^#DESC#$#))\n"
    ;

    util::GenReplacementMap repl = {
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"TYPE", wiresharkFloatTypeInternal()},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"UNIT_NAME", wiresharkUnitNameInternal()},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    assert(!repl["TYPE"].empty());
    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFloatField::wiresharkDissectBodyImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local #^#RANGE#$# = #^#TVB#$#(#^#OFFSET#$#, #^#LEN#$#)\n"
        "local #^#SUBTREE#$# = #^#TREE#$#:add#^#SUFFIX#$#(#^#FIELD#$#, #^#RANGE#$#)\n"
        "#^#RESULT#$# = #^#SUCCESS#$#\n"
        "#^#NEXT_OFFSET#$# = #^#OFFSET#$# + #^#LEN#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto parseObj = genFloatFieldParseObj();
    util::GenReplacementMap repl = {
        {"LEN", std::to_string(wiresharkMinFieldLength(refField))},
        {"SUCCESS", Wireshark::wiresharkStatusCodeStr(wiresharkGenerator, Wireshark::WiresharkStatusCode::Success)},
        {"SUBTREE", wiresharkFieldSubtreeStr()},
        {"TREE", wiresharkTreeStr()},
        {"TVB", wiresharkTvbStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"RESULT", wiresharkResultStr()},
        {"NEXT_OFFSET", wiresharkNextOffsetStr()},
        {"FIELD", wiresharkFieldStr()},
        {"RANGE", wiresharkRangeStr()}
    };

    if (parseObj.parseEndian() == commsdsl::parse::ParseEndian_Little) {
        repl["SUFFIX"] = strings::genLittleEndianSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFloatField::wiresharkValidFuncBodyImpl([[maybe_unused]] const WiresharkField* refField) const
{
    static const std::string Templ =
        "local value = #^#FUNC#$#(#^#FIELD#$#)\n"
        "local epsilon = #^#EPSILON#$#\n"
        "#^#ELEMS#$#\n"
        "return false\n"
        ;

    util::GenStringsList elems;
    auto parseObj = genFloatFieldParseObj();
    auto& ranges = parseObj.parseValidRanges();
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());

    auto numToStr =
        [](double value) -> std::string
        {
            if (!std::isinf(value)) {
                return std::to_string(value);
            }

            if (0 < value) {
                return "math.huge";
            }

            return "-math.huge";
        };

    for (auto& r : ranges) {
        if (!wiresharkGenerator.genDoesElementExist(r.m_sinceVersion, r.m_deprecatedSince, true)) {
            continue;
        }

        if (std::isnan(r.m_min)) {
            static const std::string CompTempl =
                "if value ~= value then\n"
                "    return true\n"
                "end\n"
                ;

            elems.push_back(CompTempl);
            continue;
        }

        if (r.m_min == r.m_max) {
            static const std::string CompTempl =
                "if (value == #^#VAL#$#) or (math.abs(value - (#^#VAL#$#)) < epsilon) then\n"
                "    return true\n"
                "end\n"
                ;

            util::GenReplacementMap compRepl = {
                {"VAL", numToStr(r.m_min)},
            };

            elems.push_back(util::genProcessTemplate(CompTempl, compRepl));
            continue;
        }

        static const std::string CompTempl =
            "if ((#^#MIN#$# <= value) or (math.abs(#^#MIN#$# - value) < epsilon)) and\n"
            "    ((value <= #^#MAX#$#) or (math.abs(#^#MAX#$# - value) < epsilon)) then\n"
            "    return true\n"
            "end\n"
            ;

        util::GenReplacementMap compRepl = {
            {"MIN", numToStr(r.m_min)},
            {"MAX", numToStr(r.m_max)},
        };

        elems.push_back(util::genProcessTemplate(CompTempl, compRepl));
    }

    util::GenReplacementMap repl = {
        {"ELEMS", util::genStrListToString(elems, "\n", "")},
        {"FUNC", wiresharkValueFuncName()},
        {"FIELD", wiresharkFieldStr()},
        {"EPSILON", (parseObj.parseType() == commsdsl::parse::ParseFloatField::ParseType::Float) ? "1e-6" : "1e-12"}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkFloatField::wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const
{
    assert(refField == nullptr);
    if (accStr.empty()) {
        return "tostring(" + WiresharkBase::wiresharkValueAccessStrImpl(accStr, refField) + ")";
    }

    auto& specials = genFloatFieldParseObj().parseSpecialValues();
    auto iter = specials.find(accStr);
    if (iter == specials.end()) {
        genGenerator().genLogger().genError("Failed to find reference " + accStr + " for field " + genParseObj().parseInnerRef());
        assert(false);
        return "tostring(" + WiresharkBase::wiresharkValueAccessStrImpl(std::string(), refField) + ")";
    }

    if (std::isnan(iter->second.m_value)) {
        return "\"nan\"";
    }

    if (!std::isinf(iter->second.m_value)) {
        return "tostring(" + std::to_string(iter->second.m_value) + ")";
    }

    if (iter->second.m_value < 0) {
        return "tostring(-math.huge)";
    }

    return "tostring(math.huge)";
}

std::string WiresharkFloatField::wiresharkCompPrepValueStrImpl(const std::string& value) const
{
    if (value == "nan") {
        return value;
    }

    if (value == "inf") {
        return "tostring(math.huge)";
    }

    if (value == "-inf") {
        return "tostring(-math.huge)";
    }

    return wiresharkProcessFloatValue(value);
}

std::string WiresharkFloatField::wiresharkDefaultAssignmentsImpl(const WiresharkField* refField) const
{
    auto parseObj = genFloatFieldParseObj();
    static const std::string Templ =
        "#^#TREE#$#:add(#^#FIELD#$#, #^#TVB#$#(#^#OFFSET#$#, 0), #^#VAL#$#)\n"
        ;

    util::GenReplacementMap repl = {
        {"TREE", wiresharkTreeStr()},
        {"FIELD", wiresharkFieldObjName(refField)},
        {"TVB", wiresharkTvbStr()},
        {"OFFSET", wiresharkOffsetStr()},
        {"VAL", std::to_string(parseObj.parseDefaultValue())}
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkFloatField::wiresharkHasTrivialValidImpl() const
{
    auto parseObj = genFloatFieldParseObj();
    return parseObj.parseValidRanges().empty();
}

std::string WiresharkFloatField::wiresharkUnitNameInternal() const
{
    static const std::string Map[] = {
        /* Unknown */ strings::genEmptyString(),
        /* Nanoseconds */ "ns",
        /* Microseconds */ "us",
        /* Milliseconds */ "ms",
        /* Seconds */ "s",
        /* Minutes */ "min",
        /* Hours */ "h",
        /* Days */ "days",
        /* Weeks */ "weeks",
        /* Nanometers */ "nm",
        /* Micrometers */ "um",
        /* Millimeters */ "mm",
        /* Centimeters */ "cm",
        /* Meters */ "m",
        /* Kilometers */ "km",
        /* NanometersPerSecond */ "nm/s",
        /* MicrometersPerSecond */ "um/s",
        /* MillimetersPerSecond */ "mm/s",
        /* CentimetersPerSecond */ "cm/s",
        /* MetersPerSecond */ "m/s",
        /* KilometersPerSecond */ "km/s",
        /* KilometersPerHour */ "km/h",
        /* Hertz */ "hz",
        /* KiloHertz */ "khz",
        /* MegaHertz */ "mhz",
        /* GigaHertz */ "ghz",
        /* Degrees */ "deg",
        /* Radians */ "rad",
        /* Nanoamps */ "namp",
        /* Microamps */ "uamp",
        /* Milliamps */ "mamp",
        /* Amps */ "amp",
        /* Kiloamps */ "kamp",
        /* Nanovolts */ "nv",
        /* Microvolts */ "uv",
        /* Millivolts */ "mv",
        /* Volts */ "v",
        /* Kilovolts */ "kv",
        /* Bytes */ "b",
        /* Kilobytes */ "kb",
        /* Megabytes */ "mb",
        /* Gigabytes */ "gb",
        /* Terabytes */ "tb",
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(commsdsl::parse::ParseUnits::NumOfValues));

    auto parseObj = genFloatFieldParseObj();
    auto idx = static_cast<unsigned>(parseObj.parseUnits());
    assert (idx < MapSize);
    if ((MapSize <= idx) || (Map[idx].empty())) {
        return strings::genNilStr();
    }

    return '\"' + Map[idx] + '\"';
}

const std::string& WiresharkFloatField::wiresharkFloatTypeInternal() const
{
    static const std::string Map[] = {
        /* Float */ "float",
        /* Double */ "double",
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(ParseFloatField::ParseType::NumOfValues));

    auto parseObj = genFloatFieldParseObj();
    auto idx = static_cast<unsigned>(parseObj.parseType());
    if (MapSize <= idx) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        idx = static_cast<decltype(idx)>(ParseFloatField::ParseType::Double);
    }

    return Map[idx];
}

} // namespace commsdsl2wireshark
