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

EmscriptenListField::EmscriptenListField(EmscriptenGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
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
    }
    else {
        m_element = EmscriptenField::cast(externalElementField());
    }
    assert(m_element != nullptr);
    m_element->emscriptenSetListElement();
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
    std::string templ = 
        "using ValueType = std::vector<#^#ELEMENT#$#>;\n\n"
        "ValueType* value()\n"
        "{\n"
        "    return reinterpret_cast<ValueType*>(&Base::value());\n"
        "}\n\n"
        "const ValueType* getValue() const\n"
        "{\n"
        "    return reinterpret_cast<const ValueType*>(&Base::getValue());\n"
        "}\n"
        ;   

    if (!field().dslObj().parseIsFixedValue()) {
        templ += 
            "\n"
            "void setValue(const ValueType& val)\n"
            "{\n"
            "    Base::setValue(*reinterpret_cast<const Base::ValueType*>(&val));\n"
            "}\n"
            ;              
    }             

    assert(m_element != nullptr);
    auto& gen = EmscriptenGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"ELEMENT", gen.emscriptenClassName(m_element->field())},
        {"STORAGE", emscriptenHeaderValueStorageAccByPointer()},
    };

    return util::processTemplate(templ, repl);
}

std::string EmscriptenListField::emscriptenSourceBindValueAccImpl() const
{
    static const std::string Templ = 
        "#^#STORAGE#$#\n"
        "#^#ACC#$#";

    util::ReplacementMap repl = {
        {"STORAGE", emscriptenSourceBindValueStorageAccByPointer()},
        {"ACC", emscriptenSourceBindValueAccByPointer()}
    };

    return util::processTemplate(Templ, repl);
}


} // namespace commsdsl2emscripten
