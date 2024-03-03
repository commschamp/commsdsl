//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtValueLayer.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtValueLayer::ToolsQtValueLayer(ToolsQtGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtValueLayer::prepareImpl() 
{
    return Base::prepareImpl() && ToolsBase::prepare();
}

std::string ToolsQtValueLayer::toolExtraFieldTemplParamsImpl() const
{
    if (!toolsIsForcedPseudoInternal()) {
        return strings::emptyString();
    }

    return ", comms::option::def::EmptySerialization";
}

std::string ToolsQtValueLayer::toolsForcedSerHiddenStrImpl() const
{
    if (!toolsIsForcedPseudoInternal()) {
        return ToolsBase::toolsForcedSerHiddenStrImpl();
    }

    return "true";
}

bool ToolsQtValueLayer::toolsIsForcedPseudoInternal() const
{
    auto* field = toolsExternalField();
    if (field == nullptr) {
        field = toolsMemberField();
    }

    assert(field != nullptr);
    if (field->field().dslObj().isPseudo()) {
        // Already pseudo
        return false;
    }

    return valueDslObj().pseudo();
}

} // namespace commsdsl2tools_qt
