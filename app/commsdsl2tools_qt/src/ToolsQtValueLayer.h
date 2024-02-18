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

#pragma once

#include "ToolsQtLayer.h"

#include "commsdsl/gen/ValueLayer.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtValueLayer final : public commsdsl::gen::ValueLayer, public ToolsQtLayer
{
    using Base = commsdsl::gen::ValueLayer;
    using ToolsBase = ToolsQtLayer;
public:
    explicit ToolsQtValueLayer(ToolsQtGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;

    // ToolsBase overrides
    virtual std::string toolExtraFieldTemplParamsImpl() const override;
    virtual std::string toolsForcedSerHiddenStrImpl() const override;

private:
    bool toolsIsForcedPseudoInternal() const;    
};

} // namespace commsdsl2tools_qt
