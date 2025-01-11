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

#include "commsdsl/gen/Namespace.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtInterface;

class ToolsQtNamespace final : public commsdsl::gen::Namespace
{
    using Base = commsdsl::gen::Namespace;
public:
    using StringsList = commsdsl::gen::util::StringsList;
    explicit ToolsQtNamespace(ToolsQtGenerator& generator, commsdsl::parse::Namespace dslObj, commsdsl::gen::Elem* parent);

    StringsList toolsSourceFiles(const ToolsQtInterface& interface) const;

    std::string toolsMsgFactoryOptions() const;

    static const ToolsQtNamespace* cast(const commsdsl::gen::Namespace* obj)
    {
        return static_cast<const ToolsQtNamespace*>(obj);
    }
};

} // namespace commsdsl2tools_qt
