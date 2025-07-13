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
    struct PluginInfo
    {
        std::string m_frame;
        std::string m_interface;
        std::string m_name;
        std::string m_desc;
        std::string m_pluginId;
    };
    using PluginInfosList = std::vector<PluginInfo>;
    
    ToolsQtProgramOptions();

    bool toolsQuietRequested() const;
    bool toolsVersionRequested() const;
    bool toolsWarnAsErrRequested() const;

    const std::string& toolsGetFilesListFile() const;
    const std::string& toolsGetFilesListPrefix() const;
    const GenArgsList& toolsGetFiles() const;
    const std::string& toolsGetOutputDirectory() const;
    bool toolsHasNamespaceOverride() const;
    const std::string& toolsGetNamespace() const;
    const std::string& toolsGetCodeInputDirectory() const;
    PluginInfosList toolsGetPlugins() const;
    bool toolsMultipleSchemasEnabled() const;
    bool toolsIsMainNamespaceInOptionsForced() const;
};

} // namespace commsdsl2tools_qt