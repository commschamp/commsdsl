//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CEnumField.h"

#include "CGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <limits>
#include <string>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2c
{

namespace
{

std::intmax_t cMaxLimitValStrInternal(commsdsl::parse::ParseIntField::ParseType value, std::size_t len)
{
    static const std::intmax_t TypeMap[] = {
        /* Int8 */ std::numeric_limits<std::int8_t>::max(),
        /* Uint8 */ std::numeric_limits<std::uint8_t>::max(),
        /* Int16 */ std::numeric_limits<std::int16_t>::max(),
        /* Uint16 */ std::numeric_limits<std::uint16_t>::max(),
        /* Int32 */ std::numeric_limits<std::int32_t>::max(),
        /* Uint32 */ std::numeric_limits<std::uint32_t>::max(),
        /* Int64 */ std::numeric_limits<std::int32_t>::max(),
        /* Uint64 */ static_cast<std::intmax_t>(std::numeric_limits<std::int32_t>::max()),
        /* Intvar */ 0,
        /* Uintvar */ 0
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<std::size_t>(commsdsl::parse::ParseIntField::ParseType::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(value);
    if (TypeMapSize <= idx) {
        assert(false); // Should not happen
        return 0;
    }

    auto& typeVal = TypeMap[idx];
    if (typeVal != 0) {
        return typeVal;
    }

    // Variable length
    auto offset = idx - static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Intvar);
    assert(offset < 2U);

    if (len <= 2U) {
        auto base = static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Int16);
        return TypeMap[base + offset];
    }

    if (len <= 4U) {
        auto base = static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Int32);
        return TypeMap[base + offset];
    }

    auto base = static_cast<decltype(idx)>(commsdsl::parse::ParseIntField::ParseType::Int64);
    return TypeMap[base + offset];
}

} // namespace

CEnumField::CEnumField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

CEnumField::GenStringsList CEnumField::cEnumValues(const std::string& forcedPrefix) const
{
    GenStringsList result;

    auto prefix = forcedPrefix;
    if (prefix.empty()) {
        prefix = cName() + '_' + strings::genValueTypeStr() + '_';
    }

    auto& revValues = genSortedRevValues();
    GenStringsList valuesStrings;
    valuesStrings.reserve(revValues.size() + 3);
    auto obj = genEnumFieldParseObj();
    auto& values = obj.parseValues();

    for (auto& v : revValues) {
        auto iter = values.find(*v.second);
        if (iter == values.end()) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        bool exists =
            genGenerator().genDoesElementExist(
                iter->second.m_sinceVersion,
                iter->second.m_deprecatedSince,
                false);

        if (!exists) {
            continue;
        }

        static const std::string Templ =
            "#^#NAME#$# = #^#VALUE#$#, ";

        std::string valStr = genValueToString(v.first);
        std::string docStr;
        if (!iter->second.m_description.empty()) {
            docStr = " ///< " + iter->second.m_description;
            docStr = util::genStrMakeMultiline(docStr, 40);
        }
        else if (genParseObj().parseSemanticType() == commsdsl::parse::ParseField::ParseSemanticType::MessageId) {
            if (!iter->second.m_displayName.empty()) {
                docStr = " ///< message id of <b>" + iter->second.m_displayName + "</b> message.";
            }
            else {
                docStr = " ///< message id of @b " + *v.second + " message.";
            }
        }
        else if (!iter->second.m_displayName.empty()) {
            docStr = " ///< value <b>" + iter->second.m_displayName + "</b>.";
        }
        else {
            docStr = " ///< value @b " + *v.second + '.';
        }

        auto deprecatedVer = iter->second.m_deprecatedSince;
        if (genGenerator().genIsElementDeprecated(deprecatedVer)) {
            docStr += "\nDeprecated since version " + std::to_string(deprecatedVer) + '.';
        }
        docStr = util::genStrReplace(docStr, "\n", "\n ///  ");

        assert(!valStr.empty());
        util::GenReplacementMap repl = {
            {"NAME", prefix + *v.second},
            {"VALUE", std::move(valStr)},
        };
        auto templ = util::genProcessTemplate(Templ, repl);
        assert(2U <= templ.size());
        static const std::string DocElem("#^#DOC#$#");
        templ.insert((templ.end() - 1U), DocElem.begin(), DocElem.end());

        util::GenReplacementMap docRepl = {
            {"DOC", std::move(docStr)}
        };
        valuesStrings.push_back(util::genProcessTemplate(templ, docRepl));
    }

    if (!revValues.empty()) {
        auto createValueStrFunc =
            [this](const std::string& n, std::intmax_t val, const std::string& doc) -> std::string
            {
                return
                    n + " = " + genValueToString(val) + ", ///< " + doc;

            };

        valuesStrings.push_back("\n// --- Extra values generated for convenience ---");
        valuesStrings.push_back(createValueStrFunc(prefix + genFirstValueStr(), revValues.front().first, "First defined value."));
        valuesStrings.push_back(createValueStrFunc(prefix + genLastValueStr(), revValues.back().first, "Last defined value."));

        if (genHasValuesLimit()) {
            valuesStrings.push_back(createValueStrFunc(prefix + genValuesLimitStr(), revValues.back().first + 1, "Upper limit for defined values."));

            auto parseObj = genEnumFieldParseObj();
            valuesStrings.push_back(
                createValueStrFunc(
                    prefix + "TypeValuesLimit",
                    cMaxLimitValStrInternal(parseObj.parseType(), parseObj.parseMaxLength()),
                    "Allow unknown values beyond @ref " + prefix + genValuesLimitStr()));
        }
    }

    return valuesStrings;
}

bool CEnumField::genWriteImpl() const
{
    return cWrite();
}

std::string CEnumField::cHeaderCodeImpl() const
{
    static const std::string Templ =
        "#^#ENUM#$#\n"
        "#^#FUNCS#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"ENUM", cHeaderEnumInternal()},
        {"FUNCS", cHeaderCommonValueAccessFuncs()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CEnumField::cSourceCodeImpl() const
{
    return cSourceCommonValueAccessFuncs();
}

std::string CEnumField::cHeaderEnumInternal() const
{
    static const std::string Templ =
        "/// @brief Values enumerator for\n"
        "///     @ref #^#NAME#$# field.\n"
        "typedef enum\n"
        "{\n"
        "    #^#VALUES#$#\n"
        "} #^#TYPE#$#;\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"VALUES", util::genStrListToString(cEnumValues(), "\n", "")},
        {"TYPE", cHeaderEnumTypeNameInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CEnumField::cFrameValueDefImpl(const std::string& name) const
{
    return cCommonFrameValueDef(cHeaderEnumTypeNameInternal(), name);
}

std::string CEnumField::cFrameValueAssignImpl(const std::string& valueAccess, const std::string& fieldAccess) const
{
    return cCommonFrameValueAssign(valueAccess, fieldAccess);
}

std::string CEnumField::cHeaderEnumTypeNameInternal() const
{
    auto str = cName();
    if (cIsVersionOptional()) {
        str += strings::genVersionOptionalFieldSuffixStr();
    }
    str += '_';
    str += strings::genValueTypeStr();
    return str;
}

} // namespace commsdsl2c
