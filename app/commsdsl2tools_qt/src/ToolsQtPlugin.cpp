//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtPlugin.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <fstream>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace
{

const std::string ProtSuffix("Protocol");
const std::string PluginSuffix("Plugin");
const std::string WidgetSuffix("ConfigWidget");

} // namespace    

bool ToolsQtPlugin::prepare()
{
    m_framePtr = static_cast<const ToolsQtFrame*>(m_generator.findFrame(m_frame));
    if (m_framePtr == nullptr) {
        m_generator.logger().error("Frame \"" + m_frame + "\" hasn't been defined.");
        return false;
    }    

    m_interfacePtr = static_cast<const ToolsQtInterface*>(m_generator.findInterface(m_interface));
    if (m_interfacePtr == nullptr) {
        m_generator.logger().error("Interface \"" + m_interface + "\" hasn't been defined.");
        return false;
    }    

    return true;
}

bool ToolsQtPlugin::write()
{
    return 
        toolsWriteProtocolHeaderInternal() &&
        toolsWriteProtocolSrcInternal();
}

bool ToolsQtPlugin::toolsWriteProtocolHeaderInternal() 
{
    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + strings::pluginNamespaceStr() + '/' + 
        toolsProtClassName() + strings::cppHeaderSuffixStr();

    m_generator.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#include <memory>\n\n"
        "#include \"cc_tools_qt/Protocol.h\"\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"        
        "class #^#CLASS_NAME#$#Impl;\n"
        "class #^#CLASS_NAME#$# : public cc_tools_qt::Protocol\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    virtual ~#^#CLASS_NAME#$#();\n\n"
        "    #^#VERSION_API#$#\n"
        "protected:\n"
        "    virtual const QString& nameImpl() const override;\n"
        "    virtual MessagesList readImpl(const cc_tools_qt::DataInfo& dataInfo, bool final) override;\n"
        "    virtual cc_tools_qt::DataInfoPtr writeImpl(cc_tools_qt::Message& msg) override;\n"
        "    virtual MessagesList createAllMessagesImpl() override;\n"
        "    virtual cc_tools_qt::MessagePtr createMessageImpl(const QString& idAsString, unsigned idx) override;\n"
        "    virtual UpdateStatus updateMessageImpl(cc_tools_qt::Message& msg) override;\n"
        "    virtual cc_tools_qt::MessagePtr cloneMessageImpl(const cc_tools_qt::Message& msg) override;\n"
        "    virtual cc_tools_qt::MessagePtr createInvalidMessageImpl() override;\n"
        "    virtual cc_tools_qt::MessagePtr createRawDataMessageImpl() override;\n"
        "    virtual cc_tools_qt::MessagePtr createExtraInfoMessageImpl() override;\n\n"
        "private:\n"
        "    std::unique_ptr<#^#CLASS_NAME#$#Impl> m_pImpl;\n"
        "};\n\n"
        "} // namespace #^#MAIN_NS#$#\n"
        "} // namespace #^#TOP_NS#$#\n"
        "#^#APPEND#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.mainNamespace()},
        {"CLASS_NAME", toolsProtClassName()},
    };        

    auto str = commsdsl::gen::util::processTemplate(Templ, repl);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

bool ToolsQtPlugin::toolsWriteProtocolSrcInternal() 
{
    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + strings::pluginNamespaceStr() + '/' + 
        toolsProtClassName() + strings::cppSourceSuffixStr();

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#include <cassert>\n"
        "#include \"cc_tools_qt/ProtocolBase.h\"\n"
        "#^#INTERFACE_INC#$#\n"
        "#include \"#^#FRAME_HEADER#$#\"\n"
        "#include \"#^#TRANSPORT_MESSAGE_HEADER#$#\"\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"
        "namespace #^#MAIN_NS#$#\n"
        "class #^#CLASS_NAME#$#Impl : public\n"
        "    cc_tools_qt::ProtocolBase<\n"
        "        #^#TOP_NS#$#::#^#FRAME#$##^#INTERFACE_TEMPL_PARAM#$#,\n"
        "        #^#TOP_NS#$#::#^#FRAME#$#TransportMessage#^#INTERFACE_TEMPL_PARAM#$#\n"
        "    >\n"
        "{\n"
        "    using Base =\n"
        "        cc_tools_qt::ProtocolBase<\n"
        "            #^#TOP_NS#$#::#^#FRAME#$##^#INTERFACE_TEMPL_PARAM#$#,\n"
        "            #^#TOP_NS#$#::#^#FRAME#$#TransportMessage#^#INTERFACE_TEMPL_PARAM#$#\n"
        "        >;\n"
        "public:\n"
        "    friend class #^#TOP_NS#$#::plugin::#^#CLASS_NAME#$#;\n\n"
        "    #^#CLASS_NAME#$#Impl() = default;\n"
        "    virtual ~#^#CLASS_NAME#$#Impl() = default;\n\n"
        "    #^#VERSION_IMPL_PUBLIC#$#\n"
        "protected:\n"
        "    virtual const QString& nameImpl() const override\n"
        "    {\n"
        "        static const QString Str(\"#^#PROT_NAME#$#\");\n"
        "        return Str;\n"
        "    }\n\n"
        "    #^#VERSION_IMPL_PROTECTED#$#\n"
        "    using Base::createInvalidMessageImpl;\n"
        "    using Base::createRawDataMessageImpl;\n"
        "    using Base::createExtraInfoMessageImpl;\n\n"
        "#^#VERSION_IMPL_PRIVATE#$#\n"
        "};\n\n"
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#()\n"
        "  : m_pImpl(new #^#CLASS_NAME#$#Impl())\n"
        "{\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n\n"
        "#^#VERSION_API#$#\n"
        "const QString& #^#CLASS_NAME#$#::nameImpl() const\n"
        "{\n"
        "    return m_pImpl->name();\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::MessagesList #^#CLASS_NAME#$#::readImpl(const cc_tools_qt::DataInfo& dataInfo, bool final)\n"
        "{\n"
        "    return m_pImpl->read(dataInfo, final);\n"
        "}\n\n"
        "cc_tools_qt::DataInfoPtr #^#CLASS_NAME#$#::writeImpl(cc_tools_qt::Message& msg)\n"
        "{\n"
        "    return m_pImpl->write(msg);\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::MessagesList #^#CLASS_NAME#$#::createAllMessagesImpl()\n"
        "{\n"
        "    return m_pImpl->createAllMessages();\n"
        "}\n\n"
        "cc_tools_qt::MessagePtr #^#CLASS_NAME#$#::createMessageImpl(const QString& idAsString, unsigned idx)\n"
        "{\n"
        "    return static_cast<cc_tools_qt::Protocol*>(m_pImpl.get())->createMessage(idAsString, idx);\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::UpdateStatus #^#CLASS_NAME#$#::updateMessageImpl(cc_tools_qt::Message& msg)\n"
        "{\n"
        "    return m_pImpl->updateMessage(msg);\n"
        "}\n\n"
        "cc_tools_qt::MessagePtr #^#CLASS_NAME#$#::cloneMessageImpl(const cc_tools_qt::Message& msg)\n"
        "{\n"
        "    return m_pImpl->cloneMessage(msg);\n"
        "}\n\n"
        "cc_tools_qt::MessagePtr #^#CLASS_NAME#$#::createInvalidMessageImpl()\n"
        "{\n"
        "    return m_pImpl->createInvalidMessageImpl();\n"
        "}\n\n"
        "cc_tools_qt::MessagePtr #^#CLASS_NAME#$#::createRawDataMessageImpl()\n"
        "{\n"
        "    return m_pImpl->createRawDataMessageImpl();\n"
        "}\n\n"
        "cc_tools_qt::MessagePtr #^#CLASS_NAME#$#::createExtraInfoMessageImpl()\n"
        "{\n"
        "    return m_pImpl->createExtraInfoMessageImpl();\n"
        "}\n\n"        
        "{\n\n"        
        "} // namespace #^#MAIN_NS#$#\n"
        "} // namespace #^#TOP_NS#$#\n"
        "#^#APPEND#$#\n"
    ;

    auto frameHeader = m_framePtr->toolsHeaderFilePath();
    auto transportMsgHeader = frameHeader;
    auto& suffix = strings::transportMessageSuffixStr();
    auto insertIter = transportMsgHeader.begin() + (frameHeader.size() - strings::cppHeaderSuffixStr().size());
    transportMsgHeader.insert(insertIter, suffix.begin(), suffix.end());    

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.mainNamespace()},
        {"CLASS_NAME", toolsProtClassName()},
        {"FRAME_HEADER", frameHeader},
        {"TRANSPORT_MESSAGE_HEADER", transportMsgHeader},
        {"FRAME", comms::scopeFor(*m_framePtr, m_generator)},
        {"PROT_NAME", toolsAdjustedName()},
    };        

    auto allInterfaces = m_generator.getAllInterfaces();
    if (1U < allInterfaces.size()) {
        assert(m_interfacePtr != nullptr);
        repl["INTERFACE_TEMPL_PARAM"] = '<' + m_generator.getTopNamespace() + "::" + comms::scopeFor(*m_interfacePtr, m_generator) + '>';
        repl["INTERFACE_INC"] = "#include \"" + m_interfacePtr->toolsHeaderFilePath() + "\"";
    }    

    if (toolsHasConfigWidget()) {
        const std::string VerImplPubTempl =
            "int getVersion() const\n"
            "{\n"
            "    return m_version;\n"
            "}\n\n"
            "void setVersion(int value)\n"
            "{\n"
            "    m_version = value;\n"
            "    #^#UPDATE_FRAME#$#\n"
            "}\n";

        const std::string verImplProtected =
            "virtual MessagesList createAllMessagesImpl() override\n"
            "{\n"
            "    auto list = Base::createAllMessagesImpl();\n"
            "    for (auto& mPtr : list) {\n"
            "        updateMessageWithVersion(*mPtr);\n"
            "    }\n"
            "    return list;\n"
            "}\n\n"
            "virtual cc_tools_qt::MessagePtr createMessageImpl(const QString& idAsString, unsigned idx) override\n"
            "{\n"
            "    auto mPtr = Base::createMessageImpl(idAsString, idx);\n"
            "    updateMessageWithVersion(*mPtr);\n"
            "    return mPtr;\n"
            "}\n\n";

        const std::string VerImplPrivateTempl =
            "private:\n"
            "    void updateMessageWithVersion(cc_tools_qt::Message& msg)\n"
            "    {\n"
            "        assert(dynamic_cast<#^#INTERFACE_TYPE#$#*>(&msg) != nullptr);\n"
            "        static_assert(#^#INTERFACE_TYPE#$#::hasVersionInTransportFields(),\n"
            "            \"Interface type is expected to has version in transport fields\");\n"
            "        static const std::size_t VersionIdx = \n"
            "            #^#INTERFACE_TYPE#$#::InterfaceOptions::VersionInExtraTransportFields;\n"
            "        auto& castedMsg = static_cast<#^#INTERFACE_TYPE#$#&>(msg);\n"
            "        std::get<VersionIdx>(castedMsg.transportFields()).value() =\n"
            "            static_cast<#^#INTERFACE_TYPE#$#::VersionType>(m_version);\n"
            "        castedMsg.refresh();\n"
            "        updateMessage(msg);\n"
            "    }\n\n"
            "    #^#UPDATE_FRAME#$#\n"
            "    int m_version = #^#DEFAULT_VERSION#$#;\n";

        const std::string VerApiTempl =
            "int #^#CLASS_NAME#$#::getVersion() const\n"
            "{\n"
            "    return m_pImpl->getVersion();\n"
            "}\n\n"
            "void #^#CLASS_NAME#$#::setVersion(int value)\n"
            "{\n"
            "    m_pImpl->setVersion(value);\n"
            "}\n"; 

        util::ReplacementMap replVerImplPub;

        util::ReplacementMap replVerImplPrivate = {
            {"DEFAULT_VERSION", util::numToString(m_generator.schemaVersion())},
            {"INTERFACE_TYPE", m_generator.getTopNamespace() + "::" + comms::scopeFor(*m_interfacePtr, m_generator)}
        };

        std::vector<std::string> versionFields;
        for (auto& fPtr : m_interfacePtr->fields()) {
            assert(fPtr);
            if (fPtr->dslObj().semanticType() == commsdsl::parse::Field::SemanticType::Version) {
                versionFields.push_back(fPtr->dslObj().name());
            }
        }

        std::vector<std::string> pseudoLayers;
        for (auto& lPtr : m_framePtr->layers()) {
            assert(lPtr);

            auto checkVersionField = 
                [&pseudoLayers, &lPtr](const commsdsl::gen::Field* f)
                {
                    if (f == nullptr) {
                        return;
                    }

                    if (f->dslObj().semanticType() != commsdsl::parse::Field::SemanticType::Version) {
                        return;
                    }

                    pseudoLayers.push_back(lPtr->dslObj().name());
                };

            checkVersionField(lPtr->externalField());
            checkVersionField(lPtr->memberField());
        }  

        util::StringsList pseudoUpdates;
        for (auto& l : pseudoLayers) {
            auto layerTypeStr = "LayerType_" + comms::className(l);
            auto layerAccStr = "layer_" + comms::accessName(l);
            auto str =
                "auto& " + layerAccStr + " = protocolStack()." + layerAccStr + "();\n"
                "using " + layerTypeStr + " = typename std::decay<decltype(" + layerAccStr + ")>::type;\n" +
                layerAccStr + ".pseudoField().value() =\n"
                "    static_cast<" + layerTypeStr + "::Field::ValueType>(m_version);\n";
            pseudoUpdates.push_back(std::move(str));
        }    

        if (!pseudoUpdates.empty()) {
            static const std::string UpdateFrameTempl =
                "void updateFrame()\n"
                "{\n"
                "    #^#UPDATES#$#\n"
                "}\n";

            util::ReplacementMap replUpdateFrame = {
                {"UPDATES", util::strListToString(pseudoUpdates, "", "")}
            };

            replVerImplPrivate["UPDATE_FRAME"] = util::processTemplate(UpdateFrameTempl, replUpdateFrame);
            replVerImplPub["UPDATE_FRAME"] = "updateFrame();";
        }          

        repl["VERSION_IMPL_PUBLIC"] = util::processTemplate(VerImplPubTempl, replVerImplPub);
        repl["VERSION_IMPL_PROTECTED"] = std::move(verImplProtected);
        repl["VERSION_IMPL_PRIVATE"] = util::processTemplate(VerImplPrivateTempl, replVerImplPrivate);
        repl["VERSION_API"] = util::processTemplate(VerApiTempl, repl);
    }

    auto str = commsdsl::gen::util::processTemplate(Templ, repl);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

const std::string& ToolsQtPlugin::toolsAdjustedName() const
{
    auto* nameToUse = &m_name;
    if (nameToUse->empty()) {
        nameToUse = &m_generator.schemaName();
    }
    return *nameToUse;
}

std::string ToolsQtPlugin::toolsProtClassName() const
{
    return comms::className(util::strToName(toolsAdjustedName())) + ProtSuffix;
}

std::string ToolsQtPlugin::toolsPluginClassName() const
{
    return comms::className(util::strToName(toolsAdjustedName())) + PluginSuffix;
}

std::string ToolsQtPlugin::toolsConfigWidgetClassName() const
{
    return comms::className(util::strToName(toolsAdjustedName())) + WidgetSuffix;
}

bool ToolsQtPlugin::toolsHasConfigWidget() const
{
    assert(m_interfacePtr != nullptr);
    return (m_interfacePtr->hasVersionField());
}

} // namespace commsdsl2tools_qt