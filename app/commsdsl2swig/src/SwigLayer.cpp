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

#include "SwigLayer.h"

#include "SwigField.h"
#include "SwigGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;


namespace commsdsl2swig
{

SwigLayer::SwigLayer(commsdsl::gen::Layer& layer) :
    m_layer(layer)
{
}

SwigLayer::~SwigLayer() = default;

const SwigLayer* SwigLayer::cast(const commsdsl::gen::Layer* layer)
{
    if (layer == nullptr) {
        return nullptr;
    }

    auto* swigLayer = dynamic_cast<const SwigLayer*>(layer);    
    assert(swigLayer != nullptr);
    return swigLayer;
}

std::string SwigLayer::swigDeclCode() const
{
    static const std::string Templ = 
        "#^#MEMBER#$#\n"
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "#^#PUBLIC#$#\n"
        "    #^#FIELD#$#\n"
        "    #^#FUNCS#$#\n"
        "};\n";

    auto& gen = SwigGenerator::cast(m_layer.generator());
    auto* memField = SwigField::cast(m_layer.memberField());
    auto* field = memField;
    if (field == nullptr) {
        field = SwigField::cast(m_layer.externalField());
    }

    std::string fieldDef;
    if (field != nullptr) {
        fieldDef = "using Field = " + gen.swigClassName(field->field()) + ";";
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(m_layer)},
        {"FIELD", std::move(fieldDef)},
        {"FUNCS", swigDeclFuncsImpl()},
    };

    if (memField != nullptr) {
        repl["MEMBER"] = memField->swigClassDecl();
    }

    if ((!repl["FIELD"].empty()) || (!repl["FUNCS"].empty())) {
        repl["PUBLIC"] = "public:";
    }

    return util::processTemplate(Templ, repl);
}

void SwigLayer::swigAddDef(StringsList& list) const
{
    auto* memField = SwigField::cast(m_layer.memberField());
    if (memField == nullptr) {
        return;
    }

    memField->swigAddDef(list);
}

void SwigLayer::swigAddCode(StringsList& list) const
{
    auto* field = SwigField::cast(m_layer.memberField());
    if (field == nullptr) {
        field = SwigField::cast(m_layer.externalField());
    }

    if (field != nullptr) {
        field->swigAddCode(list);
    }

    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#;\n"
        "#^#PUBLIC#$#\n"
        "    #^#FIELD#$#\n"
        "    #^#FUNCS#$#\n"
        "};\n";

    auto& gen = SwigGenerator::cast(m_layer.generator());

    std::string fieldDef;
    if (field != nullptr) {
        fieldDef = "using Field = " + gen.swigClassName(field->field()) + ";";
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(m_layer)},
        {"COMMS_CLASS", swigTemplateScopeInternal()},
        {"FIELD", std::move(fieldDef)},
        {"FUNCS", swigCodeFuncsImpl()},
    };

    if ((!repl["FIELD"].empty()) || (!repl["FUNCS"].empty())) {
        repl["PUBLIC"] = "public:";
    }

    list.push_back(util::processTemplate(Templ, repl));
}

std::string SwigLayer::swigDeclFuncsImpl() const
{
    return strings::emptyString();
}

std::string SwigLayer::swigCodeFuncsImpl() const
{
    return strings::emptyString();
}

std::string SwigLayer::swigTemplateScopeInternal() const
{
    static const std::string TemplParams = "<>";

    auto& gen = SwigGenerator::cast(m_layer.generator());
    auto commsScope = comms::scopeFor(m_layer, gen);

    using Elem = commsdsl::gen::Elem;
    auto* parent = m_layer.getParent();
    assert(parent != nullptr);
    assert(parent->elemType() == Elem::Type_Frame);

    auto optLevelScope = comms::scopeFor(*parent, gen) + strings::layersSuffixStr();
    assert(optLevelScope.size() < commsScope.size());
    assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
    
    return optLevelScope + TemplParams + commsScope.substr(optLevelScope.size());
}


} // namespace commsdsl2swig
