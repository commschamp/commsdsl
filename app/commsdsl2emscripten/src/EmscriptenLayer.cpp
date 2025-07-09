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

#include "EmscriptenLayer.h"

#include "EmscriptenField.h"
#include "EmscriptenFrame.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"
#include "EmscriptenNamespace.h"
#include "EmscriptenProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;


namespace commsdsl2emscripten
{

EmscriptenLayer::EmscriptenLayer(commsdsl::gen::GenLayer& layer) :
    m_layer(layer)
{
}

EmscriptenLayer::~EmscriptenLayer() = default;

const EmscriptenLayer* EmscriptenLayer::cast(const commsdsl::gen::GenLayer* layer)
{
    if (layer == nullptr) {
        return nullptr;
    }

    auto* emscriptenLayer = dynamic_cast<const EmscriptenLayer*>(layer);    
    assert(emscriptenLayer != nullptr);
    return emscriptenLayer;
}

bool EmscriptenLayer::emscriptenIsMainInterfaceSupported() const
{
    return emscriptenIsMainInterfaceSupportedImpl();
}

std::string EmscriptenLayer::emscriptenFieldAccName() const
{
    return "m_" + comms::genAccessName(m_layer.genParseObj().parseName());
}

std::string EmscriptenLayer::emscriptenFieldAccFuncName() const
{
    return "get" + comms::genClassName(m_layer.genParseObj().parseName());
}

void EmscriptenLayer::emscriptenAddHeaderInclude(StringsList& includes) const
{
    auto& gen = EmscriptenGenerator::cast(m_layer.genGenerator());
    auto* extField = m_layer.genExternalField();
    if (extField != nullptr) {
        includes.push_back(gen.emscriptenRelHeaderFor(*extField));
    }

    auto* memField = m_layer.genMemberField();
    if (memField != nullptr) {
        EmscriptenField::cast(memField)->emscriptenHeaderAddExtraIncludes(includes);
    }
}

std::string EmscriptenLayer::emscriptenHeaderClass() const
{
    static const std::string Templ = 
        "#^#FIELD#$#\n"
        "#^#DEF#$#\n";

    util::GenReplacementMap repl = {
        {"FIELD", emscriptenHeaderFieldDefInternal()},
        {"DEF", emscriptenHeaderClassDefInternal()}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenLayer::emscriptenSourceCode() const
{
    static const std::string Templ = 
        "#^#FIELD#$#\n"
        "#^#CODE#$#\n";

    util::GenReplacementMap repl = {
        {"FIELD", emscriptenSourceFieldBindInternal()},
        {"CODE", emscriptenSourceCodeInternal()}
    };

    return util::genProcessTemplate(Templ, repl);
}

bool EmscriptenLayer::emscriptenIsMainInterfaceSupportedImpl() const
{
    return true;
}

std::string EmscriptenLayer::emscriptenHeaderFieldDefImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenLayer::emscriptenFieldClassNameImpl() const
{
    assert(false); // should not be called.
    return strings::genEmptyString();
}

std::string EmscriptenLayer::emscriptenHeaderExtraFuncsImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenLayer::emscriptenSourceFieldBindImpl() const
{
    assert(false); // should not be called.
    return strings::genEmptyString();
}

std::string EmscriptenLayer::emscriptenSourceExtraFuncsImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenLayer::emscriptenTemplateScope() const
{
    auto& gen = EmscriptenGenerator::cast(m_layer.genGenerator());
    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);

    auto* frame = layer().genGetParent();
    assert(frame->genElemType() == commsdsl::gen::GenElem::Type_Frame);

    auto* ns = EmscriptenFrame::cast(static_cast<const commsdsl::gen::GenFrame*>(frame))->emscriptenFindInputNamespace();
    if (ns == nullptr) {
        ns = EmscriptenNamespace::cast(static_cast<const commsdsl::gen::GenNamespace*>((iFace->genParentNamespace())));
        assert(ns->emscriptenHasInput());
    }

    return 
        m_layer.genTemplateScopeOfComms(
            gen.emscriptenClassName(*iFace), 
            EmscriptenNamespace::cast(static_cast<const commsdsl::gen::GenNamespace*>(ns))->emscriptenInputClassName(), 
            EmscriptenProtocolOptions::emscriptenClassName(gen));
}

std::string EmscriptenLayer::emscriptenHeaderFieldDefInternal() const
{
    auto* memField = m_layer.genMemberField();
    if (memField != nullptr) {
        return EmscriptenField::cast(memField)->emscriptenHeaderClass();
    }

    return emscriptenHeaderFieldDefImpl();
}

std::string EmscriptenLayer::emscriptenHeaderClassDefInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#;\n\n"
        "public:\n"
        "    using Field = #^#FIELD#$#;\n"
        "    #^#FUNCS#$#\n"
        "};\n";

    auto& gen = EmscriptenGenerator::cast(m_layer.genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(m_layer)},
        {"COMMS_CLASS", emscriptenTemplateScope()},
        {"FIELD", emscriptenFieldClassNameInternal()},
        {"FUNCS", emscriptenHeaderExtraFuncsImpl()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenLayer::emscriptenFieldClassNameInternal() const
{
    auto* field = m_layer.genExternalField();
    if (field == nullptr) {
        field = m_layer.genMemberField();
    }

    if (field != nullptr) {
        auto& gen = EmscriptenGenerator::cast(m_layer.genGenerator());
        return gen.emscriptenClassName(*field);
    }

    return emscriptenFieldClassNameImpl();
}

std::string EmscriptenLayer::emscriptenSourceFieldBindInternal() const
{
    auto* memField = m_layer.genMemberField();
    if (memField != nullptr) {
        return EmscriptenField::cast(memField)->emscriptenSourceCode();
    }

    auto* extField = m_layer.genExternalField();
    if (extField != nullptr) {
        return strings::genEmptyString();
    }

    return emscriptenSourceFieldBindImpl();
}

std::string EmscriptenLayer::emscriptenSourceCodeInternal() const
{
    static const std::string Templ =
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$#&>()\n"
        "        #^#FUNCS#$#\n"
        "        ;\n"
        "}\n";

    auto& gen = EmscriptenGenerator::cast(m_layer.genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(m_layer)},
        {"FUNCS", emscriptenSourceExtraFuncsImpl()}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten
