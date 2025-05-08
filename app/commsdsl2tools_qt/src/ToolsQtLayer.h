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

#include "commsdsl/gen/Layer.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtLayer
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;

    explicit ToolsQtLayer(commsdsl::gen::Layer& layer);
    virtual ~ToolsQtLayer() = default;

    bool prepare();

    unsigned toolsMinFieldLength() const;

    const commsdsl::gen::Layer& layer() const
    {
        return m_layer;
    }

private:
    commsdsl::gen::Layer& m_layer ;
};

} // namespace commsdsl2tools_qt
