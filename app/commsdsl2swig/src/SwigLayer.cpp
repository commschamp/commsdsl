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
        "#^#FIELD#$#\n"
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "#^#PUBLIC#$#\n"
        "    #^#CODE#$#\n"
        "};\n";

    auto& gen = SwigGenerator::cast(m_layer.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(m_layer)},
        {"CODE", swigDeclFuncsImpl()},
    };

    auto* memField = SwigField::cast(m_layer.memberField());
    if (memField != nullptr) {
        repl["FIELD"] = memField->swigClassDef();
    }

    if (!repl["CODE"].empty()) {
        repl["PUBLIC"] = "public:";
    }

    return util::processTemplate(Templ, repl);
}

std::string SwigLayer::swigDeclFuncsImpl() const
{
    return strings::emptyString();
}

} // namespace commsdsl2swig
