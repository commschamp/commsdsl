//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtGenerator.h"

#include "ToolsQtBitfieldField.h"
#include "ToolsQtBundleField.h"
#include "ToolsQtChecksumLayer.h"
#include "ToolsQtCmake.h"
#include "ToolsQtCustomLayer.h"
#include "ToolsQtDataField.h"
#include "ToolsQtDefaultOptions.h"
#include "ToolsQtEnumField.h"
#include "ToolsQtFloatField.h"
#include "ToolsQtFrame.h"
#include "ToolsQtIdLayer.h"
#include "ToolsQtInputMessages.h"
#include "ToolsQtIntField.h"
#include "ToolsQtInterface.h"
#include "ToolsQtListField.h"
#include "ToolsQtMessage.h"
#include "ToolsQtMsgFactory.h"
#include "ToolsQtMsgFactoryOptions.h"
#include "ToolsQtNamespace.h"
#include "ToolsQtOptionalField.h"
#include "ToolsQtPayloadLayer.h"
#include "ToolsQtPlugin.h"
#include "ToolsQtRefField.h"
#include "ToolsQtSetField.h"
#include "ToolsQtSizeLayer.h"
#include "ToolsQtStringField.h"
#include "ToolsQtSyncLayer.h"
#include "ToolsQtValueLayer.h"
#include "ToolsQtVariantField.h"
#include "ToolsQtVersion.h"

#include "commsdsl/version.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace fs = std::filesystem;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace
{

const std::string MinToolsQtVersion("5.1.0");    

} // namespace 

const std::string& ToolsQtGenerator::toolsFileGeneratedComment()
{
    static const std::string Str =
        "// Generated by commsdsl2tools_qt v" + std::to_string(commsdsl::versionMajor()) +
        '.' + std::to_string(commsdsl::versionMinor()) + '.' +
        std::to_string(commsdsl::versionPatch()) + '\n';
    return Str;
}

ToolsQtGenerator::StringsList ToolsQtGenerator::toolsSourceFiles() const
{
    StringsList result;
    for (auto& s : schemas()) {
        auto& nsList = s->namespaces();
        for (auto& nsPtr : nsList) {
            assert(nsPtr);

            auto nsResult = static_cast<const ToolsQtNamespace*>(nsPtr.get())->toolsSourceFiles();
            result.reserve(result.size() + nsResult.size());
            std::move(nsResult.begin(), nsResult.end(), std::back_inserter(result));
        }
    }

    auto interfaces = toolsGetSelectedInterfaces();
    for (auto& i : interfaces) {
        auto iResult = ToolsQtInterface::cast(i)->toolsSourceFiles();
        result.reserve(result.size() + iResult.size());
        std::move(iResult.begin(), iResult.end(), std::back_inserter(result));
    }    

    auto frames = toolsGetSelectedFrames();
    for (auto& f : frames) {
        auto fResult = ToolsQtFrame::cast(f)->toolsSourceFiles();
        result.reserve(result.size() + fResult.size());
        std::move(fResult.begin(), fResult.end(), std::back_inserter(result));
    }   

    auto factoryResult = ToolsQtMsgFactory::toolsSourceFiles(*this);
    result.reserve(result.size() + factoryResult.size());
    std::move(factoryResult.begin(), factoryResult.end(), std::back_inserter(result));       

    return result;
}

void ToolsQtGenerator::toolsSetMainNamespaceInOptionsForced(bool value)
{
    m_mainNamespaceInOptionsForced = value;
}

bool ToolsQtGenerator::toolsHasMulitpleInterfaces() const
{
    auto interfaces = toolsGetSelectedInterfaces();
    assert(!interfaces.empty());
    return (1U < interfaces.size());    
}

bool ToolsQtGenerator::toolsHasMainNamespaceInOptions() const
{
    if (1U < schemas().size()) {
        return true;
    }

    return m_mainNamespaceInOptionsForced;
}

const std::string& ToolsQtGenerator::toolsMinCcToolsQtVersion()
{
    return MinToolsQtVersion;
}

bool ToolsQtGenerator::prepareImpl() 
{
    chooseProtocolSchema();
    if ((!Base::prepareImpl()) || 
        (!toolsPrepareDefaultInterfaceInternal())) {
        return false;
    }

    auto& schema = protocolSchema();
    m_pluginInfos.resize(std::max(m_pluginInfos.size(), std::size_t(1U)));

    for (auto& info : m_pluginInfos) {
        if (info.m_interface.empty()) {
            auto allInterfaces = schema.getAllInterfaces();
            assert(!allInterfaces.empty());
            auto* interfacePtr = allInterfaces.front();
            assert(interfacePtr != nullptr);

            if (interfacePtr->dslObj().valid()) {
                info.m_interface = interfacePtr->dslObj().externalRef();    
            }
            else {
                info.m_interface = interfacePtr->name();
            }
        }

        if (info.m_frame.empty()) {
            auto allFrames = getAllFrames();
            assert(!allFrames.empty());
            info.m_frame = allFrames.front()->dslObj().externalRef();
        }

        if (info.m_name.empty()) {
            info.m_name = schema.schemaName();
        }

        if (info.m_desc.empty()) {
            info.m_desc = schema.dslObj().description();
        }

        if (info.m_desc.empty()) {
            info.m_desc = "Protocol " + schema.schemaName();
        }

        if (info.m_pluginId.empty()) {
            info.m_pluginId = info.m_name;
        }        

        m_plugins.push_back(
            std::make_unique<ToolsQtPlugin>(
                *this, info.m_frame, info.m_interface, info.m_name, info.m_desc, info.m_pluginId));
    }

    bool result = 
        toolsPrepareSelectedInterfacesInternal() &&
        toolsPrepareSelectedFramesInternal();

    if (!result) {
        return false;
    }

    return 
        std::all_of(
            m_plugins.begin(), m_plugins.end(),
            [](auto& pPtr)
            {
                return pPtr->prepare();
            });
}

ToolsQtGenerator::NamespacePtr ToolsQtGenerator::createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtNamespace>(*this, dslObj, parent);
}

ToolsQtGenerator::InterfacePtr ToolsQtGenerator::createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtInterface>(*this, dslObj, parent);
}

ToolsQtGenerator::MessagePtr ToolsQtGenerator::createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtMessage>(*this, dslObj, parent);
}

ToolsQtGenerator::FramePtr ToolsQtGenerator::createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtFrame>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtIntField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtEnumField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtSetField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtFloatField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtBitfieldField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtBundleField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtStringField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtDataField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtListField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtRefField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtOptionalField>(*this, dslObj, parent);
}

ToolsQtGenerator::FieldPtr ToolsQtGenerator::createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtVariantField>(*this, dslObj, parent);
}

ToolsQtGenerator::LayerPtr ToolsQtGenerator::createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtCustomLayer>(*this, dslObj, parent);
}

ToolsQtGenerator::LayerPtr ToolsQtGenerator::createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtSyncLayer>(*this, dslObj, parent);
}

ToolsQtGenerator::LayerPtr ToolsQtGenerator::createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtSizeLayer>(*this, dslObj, parent);
}

ToolsQtGenerator::LayerPtr ToolsQtGenerator::createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtIdLayer>(*this, dslObj, parent);
}

ToolsQtGenerator::LayerPtr ToolsQtGenerator::createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtValueLayer>(*this, dslObj, parent);
}

ToolsQtGenerator::LayerPtr ToolsQtGenerator::createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtPayloadLayer>(*this, dslObj, parent);
}

ToolsQtGenerator::LayerPtr ToolsQtGenerator::createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2tools_qt::ToolsQtChecksumLayer>(*this, dslObj, parent);
}

bool ToolsQtGenerator::writeImpl()
{
    chooseProtocolSchema();
    bool result =  
        ToolsQtCmake::write(*this) &&
        ToolsQtInputMessages::write(*this) &&
        ToolsQtMsgFactory::write(*this) &&
        ToolsQtDefaultOptions::write(*this) &&
        ToolsQtMsgFactoryOptions::write(*this) &&
        ToolsQtVersion::write(*this);

    if (!result) {
        return false;
    }

    result = 
        std::all_of(
            m_plugins.begin(), m_plugins.end(),
            [](auto& pluginPtr)
            {
                return pluginPtr->write();
            });

    if (!result) {
        return false;
    }

    return toolsWriteExtraFilesInternal();            
}

bool ToolsQtGenerator::toolsPrepareDefaultInterfaceInternal()
{
    auto& schema = protocolSchema();
    auto allInterfaces = schema.getAllInterfaces();
    if (!allInterfaces.empty()) {
        return true;
    }

    auto* defaultNamespace = schema.addDefaultNamespace();
    auto* interface = defaultNamespace->addDefaultInterface();
    if (interface == nullptr) {
        logger().error("Failed to create default interface");
        return false;
    }

    return true;
}

bool ToolsQtGenerator::toolsPrepareSelectedInterfacesInternal()
{
    std::vector<std::string> ifNames;
    for (auto& info : m_pluginInfos) {
        ifNames.push_back(info.m_interface);
    }

    std::sort(ifNames.begin(), ifNames.end());
    ifNames.erase(
        std::unique(ifNames.begin(), ifNames.end()),
        ifNames.end()
    );

    for (auto& iName : ifNames) {
        auto* iFace = findInterface(iName);
        if (iFace == nullptr) {
            logger().error("Selected interface \"" + iName + "\" cannot be found");
            return false;
        }

        m_selectedInterfaces.push_back(iFace);
    }

    if (m_selectedInterfaces.empty()) {
        m_selectedInterfaces = getAllInterfaces();
    }

    return true;    
}

bool ToolsQtGenerator::toolsPrepareSelectedFramesInternal()
{
    std::vector<std::string> frameNames;
    for (auto& info : m_pluginInfos) {
        frameNames.push_back(info.m_frame);
    }

    std::sort(frameNames.begin(), frameNames.end());
    frameNames.erase(
        std::unique(frameNames.begin(), frameNames.end()),
        frameNames.end()
    );

    for (auto& fName : frameNames) {
        auto* frame = findFrame(fName);
        if (frame == nullptr) {
            logger().error("Selected frame \"" + fName + "\" cannot be found");
            return false;
        }

        m_selectedFrames.push_back(frame);
    }

    if (m_selectedFrames.empty()) {
        m_selectedFrames = getAllFrames();
    }

    return true;    
}

bool ToolsQtGenerator::toolsWriteExtraFilesInternal() const
{
    const std::vector<std::string> ReservedExt = {
        strings::replaceFileSuffixStr(),
        strings::extendFileSuffixStr(),
        strings::publicFileSuffixStr(),
        strings::protectedFileSuffixStr(),
        strings::privateFileSuffixStr(),
        strings::incFileSuffixStr(),
        strings::appendFileSuffixStr(),
        strings::sourcesFileSuffixStr(),
    }; 

    return copyExtraSourceFiles(ReservedExt);
}

} // namespace commsdsl2tools_qt
