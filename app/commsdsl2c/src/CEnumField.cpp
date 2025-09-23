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

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2c
{

namespace 
{

const std::string ValueTypeStr("_ValueType");

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
        prefix = cName() + ValueTypeStr + '_';
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
        ; 

    util::GenReplacementMap repl = {
        {"ENUM", cHeaderEnumInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CEnumField::cHeaderEnumInternal() const
{
    static const std::string Templ = 
        "/// @brief Values enumerator for\n"
        "///     @ref #^#HANDLE#$# field.\n"
        "typedef enum\n"
        "{\n"
        "    #^#VALUES#$#\n"
        "} #^#NAME#$#;\n"   
        ; 

    util::GenReplacementMap repl = {
        {"HANDLE", cName()},
        {"NAME", cName() + ValueTypeStr},
        {"VALUES", util::genStrListToString(cEnumValues(), "\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
