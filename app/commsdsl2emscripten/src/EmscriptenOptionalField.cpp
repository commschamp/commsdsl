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

#include "EmscriptenOptionalField.h"

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

EmscriptenOptionalField::EmscriptenOptionalField(EmscriptenGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

const std::string& EmscriptenOptionalField::emscriptenHeaderCommonModeFuncs()
{
    static const std::string Templ = 
        "Mode getMode() const\n"
        "{\n"
        "    return Base::getMode();\n"
        "}\n\n"        
        "void setMode(Mode val)\n"
        "{\n"
        "    Base::setMode(val);\n"
        "}\n\n"        
        "bool isTentative() const\n"
        "{\n"
        "    return Base::isTentative();\n"
        "}\n\n"
        "void setTentative()\n"
        "{\n"
        "    Base::setTentative();\n"
        "}\n\n"
        "bool doesExist() const\n"
        "{\n"
        "    return Base::doesExist();\n"
        "}\n\n"        
        "void setExists()\n"
        "{\n"
        "    Base::setExists();\n"
        "}\n\n"
        "bool isMissing() const\n"
        "{\n"
        "    return Base::isMissing();\n"
        "}\n\n"        
        "void setMissing()\n"
        "{\n"
        "    Base::setMissing();\n"
        "}\n"; 

    return Templ;
}

bool EmscriptenOptionalField::genPrepareImpl()
{
    if (!Base::genPrepareImpl()) {
        return false;
    }

    auto* memField = genMemberField();
    if (memField != nullptr) {
        emscriptenAddMember(memField);
        m_field = EmscriptenField::cast(memField);
        return true;
    }    

    m_field = EmscriptenField::cast(genExternalField());
    assert(m_field != nullptr);
    return true;
}

bool EmscriptenOptionalField::genWriteImpl() const
{
    return emscriptenWrite();
}

void EmscriptenOptionalField::emscriptenHeaderAddExtraIncludesImpl(StringsList& incs) const
{
    auto* extField = genExternalField();
    if (extField == nullptr) {
        return;
    }

    auto* emsciptenField = EmscriptenField::cast(extField);
    assert(emsciptenField != nullptr);
    incs.push_back(emsciptenField->emscriptenRelHeaderPath());
}

std::string EmscriptenOptionalField::emscriptenHeaderValueAccImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenOptionalField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "#^#FIELD#$#* field()\n"
        "{\n"
        "    return static_cast<#^#FIELD#$#*>(#^#PTR#$#);\n"
        "}\n\n"
        "#^#COMMON#$#\n";

    auto& gen = EmscriptenGenerator::cast(genGenerator());
    util::GenReplacementMap repl = {
        {"FIELD", gen.emscriptenClassName(m_field->field())},
        {"COMMON", emscriptenHeaderCommonModeFuncs()}
    };     

    if (genMemberField() != nullptr) {
        repl["PTR"] = "&Base::field()";
    }
    else {
        repl["PTR"] = "static_cast<" + m_field->emscriptenTemplateScope() + "*>(&Base::field())";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenOptionalField::emscriptenSourceBindValueAccImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenOptionalField::emscriptenSourceBindFuncsImpl() const
{
    static const std::string Templ = 
        ".function(\"field\", &#^#CLASS_NAME#$#::field, emscripten::allow_raw_pointers())\n"
        ".function(\"getMode\", &#^#CLASS_NAME#$#::getMode)\n"
        ".function(\"setMode\", &#^#CLASS_NAME#$#::setMode)\n"
        ".function(\"isTentative\", &#^#CLASS_NAME#$#::isTentative)\n"
        ".function(\"setTentative\", &#^#CLASS_NAME#$#::setTentative)\n"
        ".function(\"doesExist\", &#^#CLASS_NAME#$#::doesExist)\n"
        ".function(\"setExists\", &#^#CLASS_NAME#$#::setExists)\n"
        ".function(\"isMissing\", &#^#CLASS_NAME#$#::isMissing)\n"
        ".function(\"setMissing\", &#^#CLASS_NAME#$#::setMissing)"
        ;

    util::GenReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten
