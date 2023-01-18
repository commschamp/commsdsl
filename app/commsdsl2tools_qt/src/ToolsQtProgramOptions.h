//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include <vector>
#include <string>
#include <iosfwd>

#include "commsdsl/gen/ProgramOptions.h"

namespace commsdsl2tools_qt
{

class ToolsQtProgramOptions : public commsdsl::gen::ProgramOptions
{
public:
    struct PluginInfo
    {
        std::string m_frame;
        std::string m_interface;
        std::string m_name;
        std::string m_desc;
    };
    using PluginInfosList = std::vector<PluginInfo>;
    
    ToolsQtProgramOptions();

    bool quietRequested() const;
    bool versionRequested() const;
    bool warnAsErrRequested() const;

    const std::string& getFilesListFile() const;
    const std::string& getFilesListPrefix() const;
    const ArgsList& getFiles() const;
    const std::string& getOutputDirectory() const;
    bool hasNamespaceOverride() const;
    const std::string& getNamespace() const;
    const std::string& getCodeInputDirectory() const;
    PluginInfosList getPlugins() const;
    bool multipleSchemasEnabled() const;
};

} // namespace commsdsl2tools_qt