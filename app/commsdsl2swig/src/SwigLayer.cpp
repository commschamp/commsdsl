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

#include "SwigCustomLayer.h"
#include "SwigField.h"
#include "SwigFrame.h"
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

bool SwigLayer::swigReorder(SwigLayersList& siblings, bool& success) const
{
    return swigReorderImpl(siblings, success);
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

bool SwigLayer::isMainInterfaceSupported() const
{
    return isMainInterfaceSupportedImpl();
}

bool SwigLayer::swigReorderImpl(SwigLayersList& siblings, bool& success) const
{
    static_cast<void>(siblings);
    success = true;
    return false;
}

std::string SwigLayer::swigDeclFuncsImpl() const
{
    return strings::emptyString();
}

std::string SwigLayer::swigCodeFuncsImpl() const
{
    return strings::emptyString();
}

bool SwigLayer::isMainInterfaceSupportedImpl() const
{
    return true;
}

std::string SwigLayer::swigTemplateScopeInternal() const
{
    auto& gen = SwigGenerator::cast(m_layer.generator());
    auto commsScope = comms::scopeFor(m_layer, gen);
    std::string optionsParams = "<" + SwigProtocolOptions::swigClassName(gen) + ">";

    auto* parent = m_layer.getParent();
    assert(parent != nullptr);
    assert(parent->elemType() == commsdsl::gen::Elem::Type_Frame);

    auto optLevelScope = comms::scopeFor(*parent, gen) + strings::layersSuffixStr();
    assert(optLevelScope.size() < commsScope.size());
    assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
    
    auto result = optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());

    auto* frame = static_cast<const SwigFrame*>(parent);
    auto allLayers = frame->swigLayers();

    auto iter = std::find(allLayers.begin(), allLayers.end(), this);
    if (iter == allLayers.end()) {
        assert(false); // Mustn't happen
        return result;
    }

    auto addIdParams = 
        [&gen, &result]()
        {
            static const std::string Templ = 
                "<#^#INTERFACE#$#, #^#ALL_MESSAGES#$#>";

            auto* iFace = gen.swigMainInterface();
            assert(iFace != nullptr);
            util::ReplacementMap repl = {
                {"INTERFACE", gen.swigClassName(*iFace)},
                {"ALL_MESSAGES", strings::allMessagesStr()}
            };

            result += util::processTemplate(Templ, repl);
        };

    for (auto iterTmp = iter; iterTmp != allLayers.end(); ++iterTmp) {
        auto kind = (*iterTmp)->layer().dslObj().kind();
        if (kind == commsdsl::parse::Layer::Kind::Id) {
            addIdParams();
            break;
        }

        if (kind != commsdsl::parse::Layer::Kind::Custom) {
            continue;
        }

        auto& customLayer = static_cast<const commsdsl::gen::CustomLayer&>((*iterTmp)->layer());
        auto customKind = customLayer.customDslObj().semanticLayerType();
        if (customKind == commsdsl::parse::Layer::Kind::Id) {
            addIdParams();
            break;            
        }
    }

    return result;
}


} // namespace commsdsl2swig
