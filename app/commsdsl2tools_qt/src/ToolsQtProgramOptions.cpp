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

#include "ToolsQtProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace
{

const std::string ProtocolStr("protocol");
const std::string ForceMainNamespaceInOptionsStr("force-main-ns-in-options");

} // namespace

ToolsQtProgramOptions::ToolsQtProgramOptions()
{
    genAddCommonOptions();
    genRemoveMinRemoteVersionOptions()
    (ProtocolStr,
        "Protocol information for plugin generation. Exepected to be in the following format:\n"
        "\"frame_id:interface_id:protocol_name:description:plugin_id\".\nUse comma separation for multiple plugins. If not provided, "
        "one frame and one interface from the schema will be chosen and code for only one protocol "
        "plugin will be generated.\n"
        "  * frame_id - Full reference id of the frame. Can be empty if there is only one frame.\n"
        "  * interface_id - Full reference id of the interface. Can be empty if there is only one interface.\n"
        "  * name - Name of the plugin to be desplayed in the tools.\n"
        "  * description - Description of the plugin.\n"
        "  * plugin_id - ID of the plugin to be used in the saved configuration file. When empty or "
        "omitted same as \"name\" value is assumed.\n"
        , true)
    (ForceMainNamespaceInOptionsStr, "Force having main namespace struct in generated options.")
    ;
}

ToolsQtProgramOptions::ToolsPluginInfosList ToolsQtProgramOptions::toolsGetPlugins() const
{
    ToolsPluginInfosList result;
    if (!genIsOptUsed(ProtocolStr)) {
        return result;
    }

    auto infos = util::genStrSplitByAnyChar(genValue(ProtocolStr), ",");
    for (auto& i : infos) {
        enum ValueIdx : unsigned
        {
            ValueIdx_Frame,
            ValueIdx_Interface,
            ValueIdx_Name,
            ValueIdx_Desc,
            ValueIdx_PluginId,
            ValueIdx_NumOfValues
        };

        auto values = util::genStrSplitByAnyChar(i, ":", false);
        values.resize(std::max(values.size(), std::size_t(ValueIdx_NumOfValues)));
        result.resize(result.size() + 1U);
        auto& resInfo = result.back();
        resInfo.m_frame = values[ValueIdx_Frame];
        resInfo.m_interface = values[ValueIdx_Interface];
        resInfo.m_name = values[ValueIdx_Name];
        resInfo.m_desc = values[ValueIdx_Desc];
        resInfo.m_pluginId = values[ValueIdx_PluginId];
    }
    return result;
}

bool ToolsQtProgramOptions::toolsIsMainNamespaceInOptionsForced() const
{
    return genIsOptUsed(ForceMainNamespaceInOptionsStr);
}

} // namespace commsdsl2tools_qt
