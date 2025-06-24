//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenEnumField.h"

#include "EmscriptenGenerator.h"
#include "EmscriptenNamespace.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

EmscriptenEnumField::EmscriptenEnumField(EmscriptenGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

std::string EmscriptenEnumField::emscriptenBindValues(const EmscriptenNamespace* forcedParent) const
{
    StringsList result;

    auto addValueBind = 
        [this, &result, forcedParent](const std::string& name)
        {
            if ((forcedParent != nullptr) && 
                (dslObj().parseSemanticType() == commsdsl::parse::ParseField::SemanticType::MessageId)) {
                static const std::string Templ = 
                    ".value(\"#^#NAME#$#\", #^#SCOPE#$#_#^#NAME#$#)";

                util::ReplacementMap repl = {
                    {"NAME", name},
                    {"SCOPE", comms::scopeForMsgId(strings::msgIdEnumNameStr(), generator(), *forcedParent)}
                };                    

                result.push_back(util::processTemplate(Templ, repl));
                return;
            }

            static const std::string Templ = 
                ".value(\"#^#NAME#$#\", #^#CLASS_NAME#$#::ValueType::#^#NAME#$#)";

            util::ReplacementMap repl = {
                {"NAME", name},
                {"CLASS_NAME", emscriptenBindClassName()}
            };

            result.push_back(util::processTemplate(Templ, repl));
        };

    auto& revValues = sortedRevValues();
    result.reserve(revValues.size() + 3);
    auto obj = enumDslObj();
    auto& values = obj.parseValues();

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

        addValueBind(iter->first);
    }

    if (hasValuesLimit()) {
        addValueBind(valuesLimitStr());
    }

    return util::strListToString(result, "\n", "");
}

bool EmscriptenEnumField::writeImpl() const
{
    return emscriptenWrite();
}

std::string EmscriptenEnumField::emscriptenHeaderValueAccImpl() const
{
    return emscriptenHeaderValueAccByValue();
}

std::string EmscriptenEnumField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "static std::string valueNameOf(ValueType val)\n"
        "{\n"
        "    return std::string(Base::valueNameOf(val));\n"
        "}\n\n"        
        "std::string valueName() const\n"
        "{\n"
        "    return std::string(Base::valueName());\n"
        "}\n\n"
        "static typename std::underlying_type<ValueType>::type asConstant(ValueType val)\n"
        "{\n"
        "    return static_cast<typename std::underlying_type<ValueType>::type>(val);\n"
        "}\n\n"    
        "typename std::underlying_type<ValueType>::type getValueConstant() const\n"
        "{\n"
        "    return asConstant(getValue());\n"
        "}\n"  
        ;      

    if (field().dslObj().parseIsFixedValue()) {
        return Templ;
    }

    static const std::string SetTempl = 
        "\n"
        "void setValueConstant(typename std::underlying_type<ValueType>::type val)\n"               
        "{\n"
        "    return setValue(static_cast<ValueType>(val));\n"
        "}\n";

    return Templ + SetTempl;
}

std::string EmscriptenEnumField::emscriptenSourceBindFuncsImpl() const
{
    std::string templ = 
        ".class_function(\"valueNameOf\", &#^#CLASS_NAME#$#::valueNameOf)\n"
        ".function(\"valueName\", &#^#CLASS_NAME#$#::valueName)\n"
        ".class_function(\"asConstant\", &#^#CLASS_NAME#$#::asConstant)\n"
        ".function(\"getValueConstant\", &#^#CLASS_NAME#$#::getValueConstant)"
        ;

    if (!field().dslObj().parseIsFixedValue()) {
        templ += 
            "\n"
            ".function(\"setValueConstant\", &#^#CLASS_NAME#$#::setValueConstant)"
            ;
    }        

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };
    
    return util::processTemplate(templ, repl);
}

std::string EmscriptenEnumField::emscriptenSourceBindExtraImpl() const
{
    if (!emscriptenCanProvideValuesInternal()) {
        generator().logger().warning(
            "emscripten cannot handle enum values with types wider than long, cannot provide bindings for " +
            emscriptenBindClassName() + "::ValueType.");
        return strings::emptyString();
    }

    if (dslObj().parseSemanticType() == commsdsl::parse::ParseField::SemanticType::MessageId) {
        auto* parentNs = EmscriptenNamespace::cast(parentNamespace());
        auto allMsgIdFields = parentNs->findMessageIdFields();
        if (allMsgIdFields.size() == 1U) {
            // The bindings for MsgId are created separately
            return strings::emptyString();  
        }
    }

    static const std::string Templ = 
        "emscripten::enum_<#^#CLASS_NAME#$#::ValueType>(\"#^#CLASS_NAME#$#_ValueType\")\n"
        "    #^#VALUES#$#\n"
        "   ;\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
        {"VALUES", emscriptenBindValues()}
    };

    return util::processTemplate(Templ, repl);
}

bool EmscriptenEnumField::emscriptenCanProvideValuesInternal() const
{
    auto type = enumDslObj().parseType();
    if (type < commsdsl::parse::ParseIntField::Type::Int64) {
        return true;
    }

    if (type == commsdsl::parse::ParseIntField::Type::Int64) {
        return false;
    }

    if (type == commsdsl::parse::ParseIntField::Type::Uint64) {
        return false;
    }   

    if ((type == commsdsl::parse::ParseIntField::Type::Intvar) &&
        (dslObj().parseMinLength() > sizeof(std::int32_t))) {
        return false;
    }

    if ((type == commsdsl::parse::ParseIntField::Type::Uintvar) &&
        (dslObj().parseMinLength() > sizeof(std::int32_t))) {
        return false;
    }

    return true;
}

} // namespace commsdsl2emscripten
