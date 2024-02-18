//
// Copyright 2022 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenPayloadLayer.h"

#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

namespace 
{

const std::string FieldClassNameSuffix("Field");

} // namespace 
    

EmscriptenPayloadLayer::EmscriptenPayloadLayer(EmscriptenGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

std::string EmscriptenPayloadLayer::emscriptenHeaderFieldDefImpl() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public #^#COMMS_SCOPE#$#::Field\n"
        "{\n"
        "    using Base = #^#COMMS_SCOPE#$#::Field;\n"
        "public:\n"
        "    #^#CLASS_NAME#$#() = default;\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = default;\n"
        "    ~#^#CLASS_NAME#$#() = default;\n\n"
        "    const ValueType* getValue() const\n"
        "    {\n"
        "        return &Base::getValue();\n"
        "    }\n\n"
        "    void setValue(const ValueType& val)\n"
        "    {\n"
        "        Base::setValue(val);\n"
        "    }\n\n"
        "    void assignJsArray(const emscripten::val& jsArray)\n"
        "    {\n"
        "        Base::value() = #^#JS_ARRAY_FUNC#$#(jsArray);\n"
        "    }\n"
        "};\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenFieldClassNameImpl()},
        {"COMMS_SCOPE", emscriptenTemplateScope()},
        {"JS_ARRAY_FUNC", EmscriptenDataBuf::emscriptenJsArrayToDataBufFuncName()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenPayloadLayer::emscriptenFieldClassNameImpl() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    return gen.emscriptenClassName(*this) + FieldClassNameSuffix;
}

std::string EmscriptenPayloadLayer::emscriptenSourceFieldBindImpl() const
{
    static const std::string Templ =
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$#&>()\n"
        "        .function(\"getValue\", &#^#CLASS_NAME#$#::getValue, emscripten::allow_raw_pointers())\n"
        "        .function(\"setValue\", &#^#CLASS_NAME#$#::setValue)\n"
        "        .function(\"assignJsArray\", &#^#CLASS_NAME#$#::assignJsArray)\n"
        "        ;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenFieldClassNameImpl()},
    };

    return util::processTemplate(Templ, repl);
}


} // namespace commsdsl2emscripten
