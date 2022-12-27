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

#include "EmscriptenSetField.h"

#include "EmscriptenGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

EmscriptenSetField::EmscriptenSetField(EmscriptenGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenSetField::writeImpl() const
{
    return emscriptenWrite();
}

std::string EmscriptenSetField::emscriptenHeaderValueAccImpl() const
{
    return emscriptenHeaderValueAccByValue();
}

std::string EmscriptenSetField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    auto obj = setDslObj();

    util::StringsList accesses;

    for (auto& bitInfo : obj.revBits()) {

        static const std::string Templ = 
            "bool getBitValue_#^#NAME#$#() const\n"
            "{\n"
            "    return Base::getBitValue_#^#NAME#$#();\n"
            "}\n\n"
            "void setBitValue_#^#NAME#$#(bool val)\n"
            "{\n"
            "    Base::setBitValue_#^#NAME#$#(val);\n"
            "}\n";

        util::ReplacementMap repl = {
            {"NAME", bitInfo.second}
        };

        accesses.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "bool getBitValue(unsigned bitNum) const\n"
        "{\n"
        "    return Base::getBitValue(static_cast<Base::BitIdx>(bitNum));\n"
        "}\n\n"
        "void setBitValue(unsigned bitNum, bool val)\n"        
        "{\n"
        "    Base::setBitValue(static_cast<Base::BitIdx>(bitNum), val);\n"
        "}\n\n"
        "#^#ACCESS_FUNCS#$#\n"
        ;    

    util::ReplacementMap repl = {
        {"ACCESS_FUNCS", util::strListToString(accesses, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenSetField::emscriptenSourceBindFuncsImpl() const
{
    auto obj = setDslObj();

    auto& gen = EmscriptenGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
    };    

    util::StringsList accesses;
    for (auto& bitInfo : obj.revBits()) {

        static const std::string Templ = 
            ".function(\"getBitValue_#^#NAME#$#\", &#^#CLASS_NAME#$#::getBitValue_#^#NAME#$#)\n"
            ".function(\"setBitValue_#^#NAME#$#\", &#^#CLASS_NAME#$#::setBitValue_#^#NAME#$#)";

        repl["NAME"] = bitInfo.second;
        accesses.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "#^#ACCESS_FUNCS#$#\n"
        ".function(\"getBitValue\", &#^#CLASS_NAME#$#::getBitValue)\n"
        ".function(\"setBitValue\", &#^#CLASS_NAME#$#::setBitValue)";    
        ;    

    repl["ACCESS_FUNCS"] = util::strListToString(accesses, "\n", "");
    return util::processTemplate(Templ, repl);
}

std::string EmscriptenSetField::emscriptenSourceBindExtraImpl() const
{
    auto obj = setDslObj();

    auto& gen = EmscriptenGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
    };    

    util::StringsList values;
    for (auto& bitInfo : obj.revBits()) {

        static const std::string Templ = 
            ".value(\"#^#NAME#$#\", &#^#CLASS_NAME#$#::BitIdx_#^#NAME#$#)";

        repl["NAME"] = bitInfo.second;
        values.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "emscripten::enum_<#^#CLASS_NAME#$#::BitIdx>(\"#^#CLASS_NAME#$#_BitIdx\")\n"
        "    #^#VALUES#$#\n"
        "    .value(\"BitIdx_numOfValues\", #^#CLASS_NAME#$#::BitIdx_numOfValues)\n"
        "   ;\n";

    repl["VALUES"] = util::strListToString(values, "\n", "");
    return util::processTemplate(Templ, repl);
}


} // namespace commsdsl2emscripten
