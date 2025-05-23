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

#include "EmscriptenDataField.h"

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

EmscriptenDataField::EmscriptenDataField(EmscriptenGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenDataField::writeImpl() const
{
    return emscriptenWrite();
}

std::string EmscriptenDataField::emscriptenHeaderValueAccImpl() const
{
    return emscriptenHeaderValueAccByPointer();
}

std::string EmscriptenDataField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "void assignJsArray(const emscripten::val& jsArray)\n"
        "{\n"
        "    Base::value() = #^#JS_ARRAY_FUNC#$#(jsArray);"
        "}\n";

    util::ReplacementMap repl = {
        {"JS_ARRAY_FUNC", EmscriptenDataBuf::emscriptenJsArrayToDataBufFuncName()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenDataField::emscriptenSourceBindValueAccImpl() const
{
    return emscriptenSourceBindValueAccByPointer();
}

std::string EmscriptenDataField::emscriptenSourceBindFuncsImpl() const
{
    static const std::string Templ = 
        ".function(\"assignJsArray\", &#^#CLASS_NAME#$#::assignJsArray)"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::processTemplate(Templ, repl);    
}

} // namespace commsdsl2emscripten
