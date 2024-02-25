//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <vector>

namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace
{

const std::string QuietStr("quiet");
const std::string FullQuietStr("q," + QuietStr);
const std::string VersionStr("version");
const std::string OutputDirStr("output-dir");
const std::string FullOutputDirStr("o," + OutputDirStr);
const std::string InputFilesListStr("input-files-list");
const std::string FullInputFilesListStr("i," + InputFilesListStr);
const std::string InputFilesPrefixStr("input-files-prefix");
const std::string FullInputFilesPrefixStr("p," + InputFilesPrefixStr);
const std::string NamespaceStr("namespace");
const std::string FullNamespaceStr("n," + NamespaceStr);
const std::string WarnAsErrStr("warn-as-err");
const std::string CodeInputDirStr("code-input-dir");
const std::string FullCodeInputDirStr("c," + CodeInputDirStr);
const std::string ProtocolStr("protocol");
const std::string MultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string FullMultipleSchemasEnabledStr("s," + MultipleSchemasEnabledStr);
const std::string ForceMainNamespaceInOptionsStr("force-main-ns-in-options");


} // namespace

ToolsQtProgramOptions::ToolsQtProgramOptions()
{
    addHelpOption()
    (VersionStr, "Print version string and exit.")
    (FullQuietStr, "Quiet, show only warnings and errors.")
    (FullOutputDirStr, "Output directory path. When not provided current is used.", true)        
    (FullInputFilesListStr, "File containing list of input files.", true)        
    (FullInputFilesPrefixStr, "Prefix for the values from the list file.", true)
    (FullNamespaceStr, 
        "Force main namespace change. Defaults to schema name. "
        "In case of having multiple schemas the renaming happends to the last protocol one. "
        "Renaming of non-protocol or multiple schemas is allowed using <orig_name>:<new_name> comma separated pairs.",
        true) 
    (WarnAsErrStr, "Treat warning as error.")
    (FullCodeInputDirStr, "Directory with code updates.", true)
    (ProtocolStr, 
        "Protocol information for plugin generation. Exepected to be in the following format:\n"
        "\"frame_id:interface_id:protocol_name:description\".\nUse comma separation for multiple plugins. If not provided, "
        "one frame and one interface from the schema will be chosen and code for only one protocol "
        "plugin will be generated.\n"
        "  * frame_id - Full reference id of the frame. Can be empty if there is only one frame.\n"
        "  * interface_id - Full reference id of the interface. Can be empty if there is only one interface.\n"
        "  * name - Name of the plugin to be desplayed in the tools.\n"
        "  * description - Description of the plugin.\n"
        , true)    
    (FullMultipleSchemasEnabledStr, "Allow having multiple schemas with different names.")            
    (ForceMainNamespaceInOptionsStr, "Force having main namespace struct in generated options.")
    ;
}

bool ToolsQtProgramOptions::quietRequested() const
{
    return isOptUsed(QuietStr);
}

bool ToolsQtProgramOptions::versionRequested() const
{
    return isOptUsed(VersionStr);
}

bool ToolsQtProgramOptions::warnAsErrRequested() const
{
    return isOptUsed(WarnAsErrStr);
}

const std::string& ToolsQtProgramOptions::getFilesListFile() const
{
    return value(InputFilesListStr);
}

const std::string& ToolsQtProgramOptions::getFilesListPrefix() const
{
    return value(InputFilesPrefixStr);
}

const ToolsQtProgramOptions::ArgsList& ToolsQtProgramOptions::getFiles() const
{
    return args();
}

const std::string& ToolsQtProgramOptions::getOutputDirectory() const
{
    return value(OutputDirStr);
}

bool ToolsQtProgramOptions::hasNamespaceOverride() const
{
    return isOptUsed(NamespaceStr);
}

const std::string& ToolsQtProgramOptions::getNamespace() const
{
    return value(NamespaceStr);
}

const std::string& ToolsQtProgramOptions::getCodeInputDirectory() const
{
    return value(CodeInputDirStr);
}

ToolsQtProgramOptions::PluginInfosList ToolsQtProgramOptions::getPlugins() const
{
    PluginInfosList result;
    if (!isOptUsed(ProtocolStr)) {
        return result;
    }

    auto infos = util::strSplitByAnyChar(value(ProtocolStr), ",");
    for (auto& i : infos) {
        enum ValueIdx : unsigned
        {
            ValueIdx_Frame,
            ValueIdx_Interface,
            ValueIdx_Name,
            ValueIdx_Desc,
            ValueIdx_NumOfValues
        };

        auto values = util::strSplitByAnyChar(i, ":", false);
        values.resize(std::max(values.size(), std::size_t(ValueIdx_NumOfValues)));
        result.resize(result.size() + 1U);
        auto& resInfo = result.back();
        resInfo.m_frame = values[ValueIdx_Frame];
        resInfo.m_interface = values[ValueIdx_Interface];
        resInfo.m_name = values[ValueIdx_Name];
        resInfo.m_desc = values[ValueIdx_Desc];
    }
    return result;
}

bool ToolsQtProgramOptions::multipleSchemasEnabled() const
{
    return isOptUsed(MultipleSchemasEnabledStr);
}

bool ToolsQtProgramOptions::isMainNamespaceInOptionsForced() const
{
    return isOptUsed(ForceMainNamespaceInOptionsStr);
}


} // namespace commsdsl2tools_qt
