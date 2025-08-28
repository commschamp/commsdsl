//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#pragma once

#include "SwigLayer.h"

#include "commsdsl/gen/GenValueLayer.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigValueLayer final : public commsdsl::gen::GenValueLayer, public SwigLayer
{
    using GenBase = commsdsl::gen::GenValueLayer;
    using SwigBase = SwigLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    SwigValueLayer(SwigGenerator& generator, ParseLayer parseObj, GenElem* parent);

protected:
    // SwigBase overrides
    virtual std::string swigDeclFuncsImpl() const override;    
    virtual std::string swigCodeFuncsImpl() const override;  
    virtual bool swigIsMainInterfaceSupportedImpl() const override;  
};

} // namespace commsdsl2swig
