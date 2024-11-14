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

#include "ToolsQtField.h"

#include "commsdsl/gen/VariantField.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtVariantField final : public commsdsl::gen::VariantField, public ToolsQtField
{
    using Base = commsdsl::gen::VariantField;
    using ToolsBase = ToolsQtField;
public:
    explicit ToolsQtVariantField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);
};

} // namespace commsdsl2tools_qt
