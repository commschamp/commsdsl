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

#include "ToolsQtPlugin.h"
#include "ToolsQtProgramOptions.h"

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/util.h"

#include <map>

namespace commsdsl2tools_qt 
{

class ToolsQtGenerator final : public commsdsl::gen::GenGenerator
{
    using GenBase = commsdsl::gen::GenGenerator;
public:
    using GenElem = commsdsl::gen::GenElem;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using PluginInfo = ToolsQtProgramOptions::PluginInfo;
    using PluginInfosList = ToolsQtProgramOptions::PluginInfosList;
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using PluginsList = std::vector<ToolsQtPluginPtr>;
    using FramesPerInterfaceMap = std::map<const commsdsl::gen::GenInterface*, GenFramesAccessList>;

    static const std::string& toolsFileGeneratedComment();
    void toolsSetPluginInfosList(PluginInfosList&& value)
    {
        m_pluginInfos = std::move(value);
    }

    const PluginInfosList& toolsGetPluginInfosList() const
    {
        return m_pluginInfos;
    }

    GenStringsList toolsSourceFilesForInterface(const ToolsQtInterface& interface) const;

    const PluginsList& toolsPlugins() const
    {
        return m_plugins;
    }

    const GenInterfacesAccessList& toolsGetSelectedInterfaces() const
    {
        return m_selectedInterfaces;
    }

    const GenFramesAccessList& toolsGetSelectedFrames() const
    {
        return m_selectedFrames;
    }    

    const FramesPerInterfaceMap& toolsGetSelectedFramesPerInterface() const
    {
        return m_selectedFramesPerInterface;
    }

    const GenFramesAccessList& toolsGetSelectedFramesForInterface(const commsdsl::gen::GenInterface& interface);

    static ToolsQtGenerator& toolsCast(commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<ToolsQtGenerator&>(generator);
    }

    static const ToolsQtGenerator& toolsCast(const commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<const ToolsQtGenerator&>(generator);
    }    

    void toolsSetMainNamespaceInOptionsForced(bool value);
    bool toolsHasMulitpleInterfaces() const;
    bool toolsHasMainNamespaceInOptions() const;

    static const std::string& toolsMinCcToolsQtVersion();

    static const std::string& toolsNamespaceBegin();
    static const std::string& toolsNamespaceEnd();

    std::string toolsNamespaceBeginForInterface(const commsdsl::gen::GenInterface& interface) const;
    std::string toolsNamespaceEndForInterface(const commsdsl::gen::GenInterface& interface) const;    

    static const std::string& toolsScopePrefix();
    std::string toolsScopePrefixForInterface(const commsdsl::gen::GenInterface& interface) const;

protected:
    virtual bool genPrepareImpl() override;

    virtual GenNamespacePtr genCreateNamespaceImpl(commsdsl::parse::ParseNamespace parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenInterfacePtr genCreateInterfaceImpl(commsdsl::parse::ParseInterface parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenMessagePtr genCreateMessageImpl(commsdsl::parse::ParseMessage parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenFramePtr genCreateFrameImpl(commsdsl::parse::ParseFrame parseObj, commsdsl::gen::GenElem* parent) override;

    virtual GenLayerPtr genCreateCustomLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateSyncLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateSizeLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateIdLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateValueLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreatePayloadLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    virtual GenLayerPtr genCreateChecksumLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;

    virtual bool genWriteImpl() override;   

private:
    bool toolsPrepareSelectedInterfacesInternal();
    bool toolsPrepareSelectedFramesInternal();
    bool toolsWriteExtraFilesInternal() const;

    PluginInfosList m_pluginInfos;
    PluginsList m_plugins;
    GenInterfacesAccessList m_selectedInterfaces;
    GenFramesAccessList m_selectedFrames;
    FramesPerInterfaceMap m_selectedFramesPerInterface;
    bool m_mainNamespaceInOptionsForced = false;
};

} // namespace commsdsl2tools_qt
