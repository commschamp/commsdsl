//
// Copyright 2021 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "SwigValueLayer.h"

#include "SwigGenerator.h"
#include "SwigInterface.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

SwigValueLayer::SwigValueLayer(SwigGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

std::string SwigValueLayer::swigDeclFuncsImpl() const
{
    auto obj = valueDslObj();
    if (!obj.pseudo()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "Field& pseudoField();\n";

    return Templ;
}

std::string SwigValueLayer::swigCodeFuncsImpl() const
{
    auto obj = valueDslObj();
    if (!obj.pseudo()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "Field& pseudoField() { return reinterpret_cast<Field&>(Base::pseudoField()); }\n";

    return Templ;
}

bool SwigValueLayer::swigIsMainInterfaceSupportedImpl() const
{
    auto& gen = SwigGenerator::cast(generator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    return isInterfaceSupported(iFace);
}

} // namespace commsdsl2swig
