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

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Namespace.h"
#include "commsdsl/gen/util.h"

#include <map>

namespace commsdsl2tools_qt 
{

class ToolsQtGenerator final : public commsdsl::gen::Generator
{
    using Base = commsdsl::gen::Generator;
public:
    using Elem = commsdsl::gen::Elem;
    using FieldPtr = commsdsl::gen::FieldPtr;
    using FramePtr = commsdsl::gen::FramePtr;
    using InterfacePtr = commsdsl::gen::InterfacePtr;
    using LayerPtr = commsdsl::gen::LayerPtr;
    using MessagePtr = commsdsl::gen::MessagePtr;
    using NamespacePtr = commsdsl::gen::NamespacePtr;
    using PluginInfo = ToolsQtProgramOptions::PluginInfo;
    using PluginInfosList = ToolsQtProgramOptions::PluginInfosList;
    using StringsList = commsdsl::gen::util::StringsList;
    using PluginsList = std::vector<ToolsQtPluginPtr>;
    using FramesPerInterfaceMap = std::map<const commsdsl::gen::Interface*, FramesAccessList>;

    static const std::string& toolsFileGeneratedComment();
    void toolsSetPluginInfosList(PluginInfosList&& value)
    {
        m_pluginInfos = std::move(value);
    }

    const PluginInfosList& toolsGetPluginInfosList() const
    {
        return m_pluginInfos;
    }

    StringsList toolsSourceFilesForInterface(const ToolsQtInterface& interface) const;

    const PluginsList& toolsPlugins() const
    {
        return m_plugins;
    }

    const InterfacesAccessList& toolsGetSelectedInterfaces() const
    {
        return m_selectedInterfaces;
    }

    const FramesAccessList& toolsGetSelectedFrames() const
    {
        return m_selectedFrames;
    }    

    const FramesPerInterfaceMap& toolsGetSelectedFramesPerInterface() const
    {
        return m_selectedFramesPerInterface;
    }

    const FramesAccessList& toolsGetSelectedFramesForInterface(const commsdsl::gen::Interface& interface);

    static ToolsQtGenerator& cast(commsdsl::gen::Generator& generator)
    {
        return static_cast<ToolsQtGenerator&>(generator);
    }

    static const ToolsQtGenerator& cast(const commsdsl::gen::Generator& generator)
    {
        return static_cast<const ToolsQtGenerator&>(generator);
    }    

    void toolsSetMainNamespaceInOptionsForced(bool value);
    bool toolsHasMulitpleInterfaces() const;
    bool toolsHasMainNamespaceInOptions() const;

    static const std::string& toolsMinCcToolsQtVersion();

    static const std::string& toolsNamespaceBegin();
    static const std::string& toolsNamespaceEnd();

    std::string toolsNamespaceBeginForInterface(const commsdsl::gen::Interface& interface) const;
    std::string toolsNamespaceEndForInterface(const commsdsl::gen::Interface& interface) const;    

    static const std::string& toolsScopePrefix();
    std::string toolsScopePrefixForInterface(const commsdsl::gen::Interface& interface) const;

protected:
    virtual bool prepareImpl() override;

    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::ParseNamespace dslObj, Elem* parent) override;
    virtual InterfacePtr createInterfaceImpl(commsdsl::parse::ParseInterface dslObj, Elem* parent) override;
    virtual MessagePtr createMessageImpl(commsdsl::parse::ParseMessage dslObj, Elem* parent) override;
    virtual FramePtr createFrameImpl(commsdsl::parse::ParseFrame dslObj, Elem* parent) override;

    virtual LayerPtr createCustomLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createSyncLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createSizeLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createIdLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createValueLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;
    virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::ParseLayer dslObj, Elem* parent) override;

    virtual bool writeImpl() override;   

private:
    bool toolsPrepareSelectedInterfacesInternal();
    bool toolsPrepareSelectedFramesInternal();
    bool toolsWriteExtraFilesInternal() const;

    PluginInfosList m_pluginInfos;
    PluginsList m_plugins;
    InterfacesAccessList m_selectedInterfaces;
    FramesAccessList m_selectedFrames;
    FramesPerInterfaceMap m_selectedFramesPerInterface;
    bool m_mainNamespaceInOptionsForced = false;
};

} // namespace commsdsl2tools_qt
