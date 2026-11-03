//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenElem = commsdsl::gen::GenElem;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenInterface = commsdsl::gen::GenInterface;
    using GenInterfacePtr = commsdsl::gen::GenInterfacePtr;
    using GenLayerPtr = commsdsl::gen::GenLayerPtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;
    using ToolsPluginInfo = ToolsQtProgramOptions::ToolsPluginInfo;
    using ToolsPluginInfosList = ToolsQtProgramOptions::ToolsPluginInfosList;
    using ToolsPluginsList = std::vector<ToolsQtPluginPtr>;
    using ToolsFramesPerInterfaceMap = std::map<const GenInterface*, GenFramesAccessList>;

    static const std::string& toolsFileGeneratedComment();
    void toolsSetPluginInfosList(ToolsPluginInfosList&& value)
    {
        m_pluginInfos = std::move(value);
    }

    [[nodiscard]] const ToolsPluginInfosList& toolsGetPluginInfosList() const
    {
        return m_pluginInfos;
    }

    [[nodiscard]] GenStringsList toolsSourceFilesForInterface(const ToolsQtInterface& interface) const;

    [[nodiscard]] const ToolsPluginsList& toolsPlugins() const
    {
        return m_plugins;
    }

    [[nodiscard]] const GenInterfacesAccessList& toolsGetSelectedInterfaces() const
    {
        return m_selectedInterfaces;
    }

    [[nodiscard]] const GenFramesAccessList& toolsGetSelectedFrames() const
    {
        return m_selectedFrames;
    }

    [[nodiscard]] const ToolsFramesPerInterfaceMap& toolsGetSelectedFramesPerInterface() const
    {
        return m_selectedFramesPerInterface;
    }

    [[nodiscard]] const GenFramesAccessList& toolsGetSelectedFramesForInterface(const commsdsl::gen::GenInterface& interface);

    [[nodiscard]] static ToolsQtGenerator& toolsCast(commsdsl::gen::GenGenerator& obj)
    {
        return static_cast<ToolsQtGenerator&>(obj);
    }

    [[nodiscard]] static const ToolsQtGenerator& toolsCast(const commsdsl::gen::GenGenerator& obj)
    {
        return static_cast<const ToolsQtGenerator&>(obj);
    }

    void toolsSetMainNamespaceInOptionsForced(bool value);
    [[nodiscard]] bool toolsHasMulitpleInterfaces() const;
    [[nodiscard]] bool toolsHasMainNamespaceInOptions() const;

    [[nodiscard]] static const std::string& toolsMinCcToolsQtVersion();

    [[nodiscard]] static const std::string& toolsNamespaceBegin();
    [[nodiscard]] static const std::string& toolsNamespaceEnd();

    [[nodiscard]] std::string toolsNamespaceBeginForInterface(const commsdsl::gen::GenInterface& interface) const;
    [[nodiscard]] std::string toolsNamespaceEndForInterface(const commsdsl::gen::GenInterface& interface) const;

    [[nodiscard]] static const std::string& toolsScopePrefix();
    [[nodiscard]] std::string toolsScopePrefixForInterface(const commsdsl::gen::GenInterface& interface) const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;

    [[nodiscard]] virtual GenNamespacePtr genCreateNamespaceImpl(commsdsl::parse::ParseNamespace parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenInterfacePtr genCreateInterfaceImpl(commsdsl::parse::ParseInterface parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenMessagePtr genCreateMessageImpl(commsdsl::parse::ParseMessage parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenFramePtr genCreateFrameImpl(commsdsl::parse::ParseFrame parseObj, commsdsl::gen::GenElem* parent) override;

    [[nodiscard]] virtual GenLayerPtr genCreateCustomLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSyncLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateSizeLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateIdLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateValueLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreatePayloadLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;
    [[nodiscard]] virtual GenLayerPtr genCreateChecksumLayerImpl(commsdsl::parse::ParseLayer parseObj, commsdsl::gen::GenElem* parent) override;

    [[nodiscard]] virtual bool genWriteImpl() override;
    [[nodiscard]] virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    [[nodiscard]] bool toolsPrepareSelectedInterfacesInternal();
    [[nodiscard]] bool toolsPrepareSelectedFramesInternal();
    [[nodiscard]] bool toolsWriteExtraFilesInternal() const;

    ToolsPluginInfosList m_pluginInfos;
    ToolsPluginsList m_plugins;
    GenInterfacesAccessList m_selectedInterfaces;
    GenFramesAccessList m_selectedFrames;
    ToolsFramesPerInterfaceMap m_selectedFramesPerInterface;
    bool m_mainNamespaceInOptionsForced = false;
};

} // namespace commsdsl2tools_qt
