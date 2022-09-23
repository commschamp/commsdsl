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

#pragma once

#include "SwigLayer.h"

#include "commsdsl/gen/ValueLayer.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigValueLayer final : public commsdsl::gen::ValueLayer, public SwigLayer
{
    using Base = commsdsl::gen::ValueLayer;
    using SwigBase = SwigLayer;
public:
    SwigValueLayer(SwigGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent);

protected:
    // SwigBase overrides
    virtual std::string swigDeclFuncsImpl() const override;    
    virtual std::string swigCodeFuncsImpl() const override;  
    virtual bool swigIsMainInterfaceSupportedImpl() const override;  
};

} // namespace commsdsl2swig
