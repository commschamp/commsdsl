//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "SwigEnumField.h"

#include "SwigGenerator.h"
#include "SwigIntField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

namespace 
{

std::uintmax_t maxTypeValueInternal(commsdsl::parse::EnumField::Type val)
{
    static const std::uintmax_t Map[] = {
        /* Int8 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int8_t>::max()),
        /* Uint8 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint8_t>::max()),
        /* Int16 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int16_t>::max()),
        /* Uint16 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint16_t>::max()),
        /* Int32 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int32_t>::max()),
        /* Uint32 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint32_t>::max()),
        /* Int64 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int64_t>::max()),
        /* Uint64 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint64_t>::max()),
        /* Intvar */ static_cast<std::uintmax_t>(std::numeric_limits<std::int64_t>::max()),
        /* Uintvar */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint64_t>::max())
    };
    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::EnumField::Type::NumOfValues),
            "Invalid map");

    if (commsdsl::parse::EnumField::Type::NumOfValues <= val) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        val = commsdsl::parse::EnumField::Type::Uint64;
    }
    return Map[static_cast<unsigned>(val)];
}


} // namespace 
    

SwigEnumField::SwigEnumField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

SwigEnumField::StringsList SwigEnumField::swigEnumValues() const
{
    StringsList result;
    
    auto obj = enumDslObj();
    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::parse::EnumField::Type::Uint64) ||
        (type == commsdsl::parse::EnumField::Type::Uintvar);
    unsigned hexW = hexWidth();

    using RevValueInfo = std::pair<std::intmax_t, const std::string*>;
    using SortedRevValues = std::vector<RevValueInfo>;
    SortedRevValues sortedRevValues;
    for (auto& v : obj.revValues()) {
        sortedRevValues.push_back(std::make_pair(v.first, &v.second));
    }

    if (bigUnsigned) {
        std::sort(
            sortedRevValues.begin(), sortedRevValues.end(),
            [](const auto& elem1, const auto& elem2) -> bool
            {
                return static_cast<std::uintmax_t>(elem1.first) < static_cast<std::uintmax_t>(elem2.first);
            });
    }

    auto valToStrFunc =
        [bigUnsigned, hexW](std::intmax_t val) -> std::string
        {
            if ((bigUnsigned) || (0U < hexW)) {
                return util::numToString(static_cast<std::uintmax_t>(val), hexW);
            }
            else {
                return util::numToString(val);
            }
        };

    util::StringsList valuesStrings;
    valuesStrings.reserve(sortedRevValues.size() + 3);
    auto& values = obj.values();

    for (auto& v : sortedRevValues) {
        auto iter = values.find(*v.second);
        if (iter == values.end()) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            continue;
        }

        bool exists =
            generator().doesElementExist(
                iter->second.m_sinceVersion,
                iter->second.m_deprecatedSince,
                false);

        if (!exists) {
            continue;
        }

        static const std::string Templ =
            "#^#NAME#$# = #^#VALUE#$#, ";


        std::string valStr = valToStrFunc(v.first);

        assert(!valStr.empty());
        util::ReplacementMap repl = {
            {"NAME", *v.second},
            {"VALUE", std::move(valStr)},
        };
        valuesStrings.push_back(util::processTemplate(Templ, repl));
    }

    if (!sortedRevValues.empty()) {
        auto addNameSuffixFunc =
            [&values](const std::string& n) -> std::string
            {
                std::string suffix;
                while (true) {
                    auto s = n + suffix;
                    if (values.find(s) == values.end()) {
                        return s;
                    }

                    suffix += '_';
                }
            };

        auto& firstElem = sortedRevValues.front();
        assert(firstElem.second != nullptr);
        assert(!firstElem.second->empty());
        auto firstLetter = firstElem.second->front();
        bool useLower = (std::tolower(firstLetter) == static_cast<int>(firstLetter));

        auto adjustFirstLetterNameFunc =
            [useLower](const std::string& s)
            {
                if (!useLower) {
                    assert(!s.empty());
                    assert(s[0] == static_cast<char>(std::toupper(s[0])));
                    return s;
                }

                auto sCpy = s;
                assert(!s.empty());
                sCpy[0] = static_cast<char>(std::tolower(sCpy[0]));
                return sCpy;
            };

        auto createValueStrFunc =
            [&adjustFirstLetterNameFunc, &addNameSuffixFunc, &valToStrFunc](const std::string& n, std::intmax_t val, const std::string& doc) -> std::string
            {
                return
                    adjustFirstLetterNameFunc(addNameSuffixFunc(n)) + " = " +
                    valToStrFunc(val) + ", // " + doc;

            };

        valuesStrings.push_back("\n// --- Extra values generated for convenience ---");
        valuesStrings.push_back(createValueStrFunc("FirstValue", sortedRevValues.front().first, "First defined value."));
        valuesStrings.push_back(createValueStrFunc("LastValue", sortedRevValues.back().first, "Last defined value."));

        bool putLimit =
            (!bigUnsigned) &&
            sortedRevValues.back().first < static_cast<std::intmax_t>(maxTypeValueInternal(obj.type()));

        if (bigUnsigned) {
            putLimit = static_cast<std::uintmax_t>(sortedRevValues.back().first) < maxTypeValueInternal(obj.type());
        }

        if (putLimit) {
            valuesStrings.push_back(createValueStrFunc("ValuesLimit", sortedRevValues.back().first + 1, "Upper limit for defined values."));
        }
    }

    return valuesStrings;    
}

bool SwigEnumField::writeImpl() const
{
    return swigWrite();
}

std::string SwigEnumField::swigValueTypeImpl() const
{
    auto& gen = generator();
    if (dslObj().semanticType() == commsdsl::parse::Field::SemanticType::MessageId) {
        static const std::string Templ =
            "using ValueType = #^#MSG_ID#$#;\n";

        util::ReplacementMap repl = {
            {"MSG_ID", SwigGenerator::swigScopeToName(comms::scopeForRoot(strings::msgIdEnumNameStr(), gen))}
        };
        return util::processTemplate(Templ, repl);
    }

    static const std::string Templ =
        "enum class ValueType : #^#TYPE#$#\n"
        "{\n"
        "    #^#VALUES#$#\n"
        "};\n"
    ;    

    auto values = swigEnumValues();
    util::ReplacementMap repl = {
        {"TYPE", SwigIntField::swigConvertIntType(enumDslObj().type(), enumDslObj().maxLength())},
        {"VALUES", util::strListToString(values, "\n", "")}
    };
    return util::processTemplate(Templ, repl); 
}

std::string SwigEnumField::swigExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "using ValueNameInfo = #^#NAME_INFO_TYPE#$#;\n"
        "using ValueNamesMapInfo = std::pair<const ValueNameInfo*, unsigned long long>;\n"
        "static const char* valueNameOf(ValueType val);\n"
        "const char* valueName() const;\n"
        "static ValueNamesMapInfo valueNamesMap();\n"
    ;

    util::ReplacementMap repl = {
        {"NAME_INFO_TYPE", swigIsDirectValueNameMappingInternal() ? "const char*" : "std::pair<ValueType, const char*>"}
    };

    return util::processTemplate(Templ, repl);
}

bool SwigEnumField::swigIsDirectValueNameMappingInternal() const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    assert(!revValues.empty());
    auto firstIter = revValues.begin();
    if (firstIter->first < 0) {
        // has negative numbers
        return false;
    }

    auto lastIter = revValues.end();
    std::advance(lastIter, -1);
    auto lastVal = static_cast<std::uintmax_t>(lastIter->first);
    auto maxDirectAllowed = static_cast<std::size_t>((revValues.size() * 110U) / 100);
    if ((maxDirectAllowed <= lastVal) && (10 <= lastVal)) {
        // Too sparse
        return false;
    }

    return true;    
}

} // namespace commsdsl2swig
