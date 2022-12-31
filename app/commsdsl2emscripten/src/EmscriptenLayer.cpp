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

#include "EmscriptenLayer.h"

#include "EmscriptenField.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"
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

EmscriptenLayer::EmscriptenLayer(commsdsl::gen::Layer& layer) :
    m_layer(layer)
{
}

EmscriptenLayer::~EmscriptenLayer() = default;

const EmscriptenLayer* EmscriptenLayer::cast(const commsdsl::gen::Layer* layer)
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
    return "m_" + comms::accessName(m_layer.dslObj().name());
}

bool EmscriptenLayer::emscriptenIsMainInterfaceSupportedImpl() const
{
    return true;
}

std::string EmscriptenLayer::emscriptenTemplateScope() const
{
    auto& gen = EmscriptenGenerator::cast(m_layer.generator());
    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);
    return m_layer.templateScopeOfComms(gen.emscriptenClassName(*iFace), strings::allMessagesStr(), EmscriptenProtocolOptions::emscriptenClassName(gen));
}


} // namespace commsdsl2emscripten
