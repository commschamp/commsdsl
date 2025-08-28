//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenProgramOptions.h"

#include <iosfwd>
#include <string>
#include <vector>

namespace commsdsl2tools_qt
{

class ToolsQtProgramOptions : public commsdsl::gen::GenProgramOptions
{
public:
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;

    struct ToolsPluginInfo
    {
        std::string m_frame;
        std::string m_interface;
        std::string m_name;
        std::string m_desc;
        std::string m_pluginId;
    };
    using ToolsPluginInfosList = std::vector<ToolsPluginInfo>;
    
    ToolsQtProgramOptions();

    static const ToolsQtProgramOptions& toolsCast(const GenProgramOptions& options)
    {
        return static_cast<const ToolsQtProgramOptions&>(options);
    }

    ToolsPluginInfosList toolsGetPlugins() const;
    bool toolsIsMainNamespaceInOptionsForced() const;
};

} // namespace commsdsl2tools_qt