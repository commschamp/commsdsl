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

#include "ToolsQtFrame.h"
#include "ToolsQtInterface.h"

#include <memory>
#include <string>

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtPlugin
{
public:
    using Ptr = std::unique_ptr<ToolsQtPlugin>;

    explicit ToolsQtPlugin(
        ToolsQtGenerator& generator,
        const std::string& frame,
        const std::string& interface,
        const std::string& name,
        const std::string& description) :
        m_generator(generator),
        m_frame(frame),
        m_interface(interface),
        m_name(name),
        m_description(description)
    {
    }        

    bool prepare();
    bool write();

    std::string toolsProtocolName() const;

    bool toolsHasConfigWidget() const
    {
        return toolsHasConfigWidgetInternal();
    }

private:

    bool toolsWriteProtocolHeaderInternal();
    bool toolsWriteProtocolSrcInternal();
    bool toolsWritePluginHeaderInternal();
    bool toolsWritePluginSrcInternal();
    bool toolsWritePluginJsonInternal();
    bool toolsWritePluginConfigInternal();
    bool toolsWriteConfigWidgetHeaderInternal();
    bool toolsWriteConfigWidgetSrcInternal();
    
    const std::string& toolsAdjustedNameInternal() const;
    std::string toolsProtClassNameInternal() const;
    std::string toolsPluginClassNameInternal() const;
    std::string toolsConfigWidgetClassNameInternal() const;
    bool toolsHasConfigWidgetInternal() const;
    std::string toolsRelFilePath(const std::string& name) const;

    ToolsQtGenerator& m_generator;
    std::string m_frame;
    std::string m_interface;
    std::string m_name;
    std::string m_description;

    const ToolsQtFrame* m_framePtr = nullptr;
    const ToolsQtInterface* m_interfacePtr = nullptr;
};

using ToolsQtPluginPtr = ToolsQtPlugin::Ptr;


} // namespace commsdsl2tools_qt