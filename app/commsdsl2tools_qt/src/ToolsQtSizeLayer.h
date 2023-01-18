//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/SizeLayer.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtSizeLayer final : public commsdsl::gen::SizeLayer, public ToolsQtLayer
{
    using Base = commsdsl::gen::SizeLayer;
    using ToolsBase = ToolsQtLayer;
public:
    explicit ToolsQtSizeLayer(ToolsQtGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;

    // ToolsBase overrides
};

} // namespace commsdsl2tools_qt
