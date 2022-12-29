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

#include "EmscriptenListField.h"

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

EmscriptenListField::EmscriptenListField(EmscriptenGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenListField::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    auto* memElement = memberElementField();
    if (memElement != nullptr) {
        emscriptenAddMember(memElement);
        m_element = EmscriptenField::cast(memElement);
        return true;
    }
    
    m_element = EmscriptenField::cast(externalElementField());
    assert(m_element != nullptr);
    return true;
}

bool EmscriptenListField::writeImpl() const
{
    return emscriptenWrite();
}

void EmscriptenListField::emscriptenHeaderAddExtraIncludesImpl(StringsList& incs) const
{
    auto* extElement = externalElementField();
    if (extElement == nullptr) {
        return;
    }

    auto* emsciptenField = EmscriptenField::cast(extElement);
    assert(emsciptenField != nullptr);
    incs.push_back(emsciptenField->emscriptenRelHeaderPath());
}

std::string EmscriptenListField::emscriptenHeaderValueAccImpl() const
{
    static const std::string Templ = 
        "using ValueType = std::vector<#^#ELEMENT#$#>;\n\n"
        "const ValueType* getValue() const\n"
        "{\n"
        "    return reinterpret_cast<const ValueType*>(&Base::getValue());\n"
        "}\n\n"
        "void setValue(const ValueType& val) const\n"
        "{\n"
        "    Base::setValue(reinterpret_cast<Base::ValueType&>(val));\n"
        "}\n";        

    assert(m_element != nullptr);
    auto& gen = EmscriptenGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"ELEMENT", gen.emscriptenClassName(m_element->field())},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenListField::emscriptenSourceBindValueAccImpl() const
{
    return emscriptenSourceBindValueAccByPointer();
}

std::string EmscriptenListField::emscriptenSourceBindExtraImpl() const
{
    static const std::string Templ = 
        "emscripten::register_vector<#^#CLASS_NAME#$#::ValueType>(\"#^#CLASS_NAME#$#_ValueType\");";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten
