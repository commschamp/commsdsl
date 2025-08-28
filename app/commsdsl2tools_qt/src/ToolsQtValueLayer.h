//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenValueLayer.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtValueLayer final : public commsdsl::gen::GenValueLayer, public ToolsQtLayer
{
    using GenBase = commsdsl::gen::GenValueLayer;
    using ToolsBase = ToolsQtLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    explicit ToolsQtValueLayer(ToolsQtGenerator& generator, ParseLayer parseObj, GenElem* parent);

protected:
    // GenBase overrides
    virtual bool genPrepareImpl() override;
};

} // namespace commsdsl2tools_qt
