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

#include "ToolsQtPlugin.h"
#include "ToolsQtProgramOptions.h"

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Namespace.h"
#include "commsdsl/gen/util.h"

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

    static const std::string& fileGeneratedComment();
    void setPluginInfosList(PluginInfosList&& value)
    {
        m_pluginInfos = std::move(value);
    }

    const PluginInfosList& getPluginInfosList() const
    {
        return m_pluginInfos;
    }

    StringsList toolsSourceFiles() const;

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

    static ToolsQtGenerator& cast(commsdsl::gen::Generator& generator)
    {
        return static_cast<ToolsQtGenerator&>(generator);
    }

    static const ToolsQtGenerator& cast(const commsdsl::gen::Generator& generator)
    {
        return static_cast<const ToolsQtGenerator&>(generator);
    }    

    bool toolsHasMulitpleInterfaces() const;
    bool toolsHasMainNamespaceInOptions() const;

protected:
    virtual bool prepareImpl() override;

    virtual NamespacePtr createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent) override;
    virtual InterfacePtr createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent) override;
    virtual MessagePtr createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent) override;
    virtual FramePtr createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent) override;

    virtual FieldPtr createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;
    virtual FieldPtr createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent) override;

    virtual LayerPtr createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;
    virtual LayerPtr createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent) override;

    virtual bool writeImpl() override;   

private:
    bool toolsPrepareDefaultInterfaceInternal();
    bool toolsPrepareSelectedInterfacesInternal();
    bool toolsPrepareSelectedFramesInternal();

    PluginInfosList m_pluginInfos;
    PluginsList m_plugins;
    InterfacesAccessList m_selectedInterfaces;
    FramesAccessList m_selectedFrames;
};

} // namespace commsdsl2tools_qt
