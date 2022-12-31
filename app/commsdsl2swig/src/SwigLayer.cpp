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
#include "SwigInterface.h"
#include "SwigProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
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
        "public:\n"
        "    using Field = #^#FIELD#$#;\n"
        "    #^#FUNCS#$#\n"
        "};\n";

    auto& gen = SwigGenerator::cast(m_layer.generator());
    auto* memField = SwigField::cast(m_layer.memberField());
    auto* field = memField;
    if (field == nullptr) {
        field = SwigField::cast(m_layer.externalField());
    }

    std::string fieldDef = swigFieldTypeImpl();
    if (field != nullptr) {
        fieldDef = gen.swigClassName(field->field());
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(m_layer)},
        {"FIELD", std::move(fieldDef)},
        {"FUNCS", swigDeclFuncsImpl()},
        {"MEMBER", swigMemberFieldDeclImpl()}
    };

    if (memField != nullptr) {
        repl["MEMBER"] = memField->swigClassDecl();
    }

    return util::processTemplate(Templ, repl);
}

void SwigLayer::swigAddDef(StringsList& list) const
{
    auto* field = SwigField::cast(m_layer.memberField());
    if (field == nullptr) {
        field = SwigField::cast(m_layer.externalField());
    }

    if (field != nullptr) {
        field->swigAddDef(list);
    }
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

    swigAddCodeImpl(list);

    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#;\n"
        "public:\n"
        "    using Field = #^#FIELD#$#;\n"
        "    #^#FUNCS#$#\n"
        "};\n";

    auto& gen = SwigGenerator::cast(m_layer.generator());

    std::string fieldDef = swigFieldTypeImpl();
    if (field != nullptr) {
        fieldDef = gen.swigClassName(field->field());
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(m_layer)},
        {"COMMS_CLASS", swigTemplateScope()},
        {"FIELD", std::move(fieldDef)},
        {"FUNCS", swigCodeFuncsImpl()},
    };

    list.push_back(util::processTemplate(Templ, repl));
}

void SwigLayer::swigAddToAllFieldsDecl(StringsList& list) const
{
    static const std::string Templ = 
        "#^#CLASS_NAME#$#::Field #^#ACC_NAME#$#;\n";

    auto& gen = SwigGenerator::cast(m_layer.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(m_layer)},
        {"ACC_NAME", swigFieldAccName()}
    };

    list.push_back(util::processTemplate(Templ, repl));
}

bool SwigLayer::swigIsMainInterfaceSupported() const
{
    return swigIsMainInterfaceSupportedImpl();
}

std::string SwigLayer::swigFieldAccName() const
{
    return "m_" + comms::accessName(m_layer.dslObj().name());
}

std::string SwigLayer::swigDeclFuncsImpl() const
{
    return strings::emptyString();
}

std::string SwigLayer::swigCodeFuncsImpl() const
{
    return strings::emptyString();
}

bool SwigLayer::swigIsMainInterfaceSupportedImpl() const
{
    return true;
}

std::string SwigLayer::swigMemberFieldDeclImpl() const
{
    return strings::emptyString();
}

void SwigLayer::swigAddCodeImpl(StringsList& list) const
{
    static_cast<void>(list);
}

std::string SwigLayer::swigFieldTypeImpl() const
{
    return strings::emptyString();
}

std::string SwigLayer::swigTemplateScope() const
{
    auto& gen = SwigGenerator::cast(m_layer.generator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    return m_layer.templateScopeOfComms(gen.swigClassName(*iFace), strings::allMessagesStr(), SwigProtocolOptions::swigClassName(gen));
}


} // namespace commsdsl2swig
