//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

SwigEnumField::SwigEnumField(SwigGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

SwigEnumField::StringsList SwigEnumField::swigEnumValues() const
{
    StringsList result;
    
    auto obj = enumDslObj();
    auto& revValues = sortedRevValues();
    util::StringsList valuesStrings;
    valuesStrings.reserve(revValues.size() + 3);
    auto& values = obj.values();

    for (auto& v : revValues) {
        auto iter = values.find(*v.second);
        if (iter == values.end()) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
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


        std::string valStr = valueToString(v.first);

        assert(!valStr.empty());
        util::ReplacementMap repl = {
            {"NAME", *v.second},
            {"VALUE", std::move(valStr)},
        };
        valuesStrings.push_back(util::processTemplate(Templ, repl));
    }

    if (!revValues.empty()) {
        auto createValueStrFunc =
            [this](const std::string& n, std::intmax_t val, const std::string& doc) -> std::string
            {
                return n + " = " + valueToString(val) + ", // " + doc;

            };

        valuesStrings.push_back("\n// --- Extra values generated for convenience ---");
        valuesStrings.push_back(createValueStrFunc(firstValueStr(), revValues.front().first, "First defined value."));
        valuesStrings.push_back(createValueStrFunc(lastValueStr(), revValues.back().first, "Last defined value."));

        if (hasValuesLimit()) {
            valuesStrings.push_back(createValueStrFunc(valuesLimitStr(), revValues.back().first + 1, "Upper limit for defined values."));
        }
    }

    return valuesStrings;    
}

bool SwigEnumField::writeImpl() const
{
    return swigWrite();
}

std::string SwigEnumField::swigValueTypeDeclImpl() const
{
    auto& gen = SwigGenerator::cast(generator());


    static const std::string Templ =
        "enum class ValueType : #^#TYPE#$#\n"
        "{\n"
        "    #^#VALUES#$#\n"
        "};\n"
    ;    

    auto values = swigEnumValues();
    util::ReplacementMap repl = {
        {"TYPE", gen.swigConvertIntType(enumDslObj().type(), enumDslObj().maxLength())},
        {"VALUES", util::strListToString(values, "\n", "")}
    };
    return util::processTemplate(Templ, repl); 
}

std::string SwigEnumField::swigExtraPublicFuncsDeclImpl() const
{
    static const std::string Templ = 
        "static const char* valueNameOf(#^#TYPE#$# val);\n"
        "const char* valueName() const;\n"
    ;

    auto& gen = SwigGenerator::cast(generator());
    auto type = gen.swigConvertIntType(enumDslObj().type(), enumDslObj().maxLength());
    util::ReplacementMap repl = {
        {"TYPE", type}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigEnumField::swigExtraPublicFuncsCodeImpl() const
{
    static const std::string Templ = 
        "#^#VALUE_TYPE#$#\n"
        "static const char* valueNameOf(#^#TYPE#$# val)\n"
        "{\n"
        "    return Base::valueNameOf(static_cast<Base::ValueType>(val));\n"
        "}\n"
    ;

    auto& gen = SwigGenerator::cast(generator());
    auto type = gen.swigConvertIntType(enumDslObj().type(), enumDslObj().maxLength());
    util::ReplacementMap repl = {
        {"TYPE", type}
    };

    if (dslObj().semanticType() == commsdsl::parse::ParseField::SemanticType::MessageId) {
        std::string valTempl = 
            "#^#TYPE#$#\n"
            "const ValueType& getValue() const\n"
            "{\n"
            "    static_assert(sizeof(ValueType) == sizeof(Base::ValueType), \"Invalid assumption\");\n"
            "    return *(reinterpret_cast<const ValueType*>(&Base::getValue()));\n"
            "}\n";

        if (!field().dslObj().isFixedValue()) {
            valTempl += 
                "\n"
                "void setValue(const ValueType& val)\n"
                "{\n"
                "    Base::setValue(static_cast<Base::ValueType>(val));\n"
                "}\n";            
        }

        util::ReplacementMap valRepl = {
            {"TYPE", swigValueTypeDeclImpl()},
        };

        repl["VALUE_TYPE"] = util::processTemplate(valTempl, valRepl);
    }

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig
