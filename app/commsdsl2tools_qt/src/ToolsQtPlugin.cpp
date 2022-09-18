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
        toolsWriteProtocolSrcInternal() &&
        toolsWritePluginHeaderInternal() &&
        toolsWritePluginSrcInternal() &&
        toolsWritePluginJsonInternal() &&
        toolsWritePluginConfigInternal() &&
        toolsWriteConfigWidgetHeaderInternal() &&
        toolsWriteConfigWidgetSrcInternal();
}

std::string ToolsQtPlugin::toolsClassName() const
{
    return comms::className(toolsAdjustedNameInternal());
}

bool ToolsQtPlugin::toolsWriteProtocolHeaderInternal() 
{
    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + toolsRelFilePath(toolsProtClassNameInternal()) + strings::cppHeaderSuffixStr();

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
        "namespace plugin\n"
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
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsProtClassNameInternal()},
    };   


    if (toolsHasConfigWidgetInternal()) {
        std::string verApi =
            "int getVersion() const;\n"
            "void setVersion(int value);\n";
        repl["VERSION_API"] = std::move(verApi);
    }         

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
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
        m_generator.getOutputDir() + '/' + toolsRelFilePath(toolsProtClassNameInternal()) + strings::cppSourceSuffixStr();

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
        "{\n\n"
        "namespace plugin\n"
        "{\n\n"
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
        "    friend class #^#TOP_NS#$#::#^#MAIN_NS#$#::plugin::#^#CLASS_NAME#$#;\n\n"
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
        "} // namespace plugin\n\n"       
        "} // namespace #^#MAIN_NS#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n\n"
    ;

    auto frameHeader = m_framePtr->toolsHeaderFilePath();
    auto transportMsgHeader = frameHeader;
    auto& suffix = strings::transportMessageSuffixStr();
    auto insertIter = transportMsgHeader.begin() + (frameHeader.size() - strings::cppHeaderSuffixStr().size());
    transportMsgHeader.insert(insertIter, suffix.begin(), suffix.end());    

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsProtClassNameInternal()},
        {"FRAME_HEADER", frameHeader},
        {"TRANSPORT_MESSAGE_HEADER", transportMsgHeader},
        {"FRAME", comms::scopeFor(*m_framePtr, m_generator)},
        {"PROT_NAME", toolsAdjustedNameInternal()},
    };        

    auto allInterfaces = m_generator.toolsGetSelectedInterfaces();
    if (1U < allInterfaces.size()) {
        assert(m_interfacePtr != nullptr);
        repl["INTERFACE_TEMPL_PARAM"] = '<' + m_generator.getTopNamespace() + "::" + comms::scopeFor(*m_interfacePtr, m_generator) + '>';
        repl["INTERFACE_INC"] = "#include \"" + m_interfacePtr->toolsHeaderFilePath() + "\"";
    }    

    if (toolsHasConfigWidgetInternal()) {
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
            {"DEFAULT_VERSION", util::numToString(m_generator.currentSchema().schemaVersion())},
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

                    if ((f->dslObj().semanticType() != commsdsl::parse::Field::SemanticType::Version) ||
                        (!f->dslObj().isPseudo())) {
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

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

bool ToolsQtPlugin::toolsWritePluginHeaderInternal() 
{
    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + toolsRelFilePath(toolsPluginClassNameInternal()) + strings::cppHeaderSuffixStr();

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#include <QtCore/QObject>\n"
        "#include <QtCore/QtPlugin>\n"
        "#include \"cc_tools_qt/Plugin.h\"\n"
        "#include \"cc_tools_qt/Protocol.h\"\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"    
        "namespace plugin\n"
        "{\n\n"    
        "class #^#CLASS_NAME#$# : public cc_tools_qt::Plugin\n"
        "{\n"
        "    Q_OBJECT\n"
        "    Q_PLUGIN_METADATA(IID \"#^#ID#$#\" FILE \"#^#CLASS_NAME#$#.json\")\n"
        "    Q_INTERFACES(cc_tools_qt::Plugin)\n\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    virtual ~#^#CLASS_NAME#$#();\n"
        "private:\n"
        "    cc_tools_qt::ProtocolPtr m_protocol;\n"
        "    #^#VERSION_STORAGE#$#\n"
        "};\n\n"
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsPluginClassNameInternal()},
        {"ID", toolsAdjustedNameInternal()},
    };        

    if (toolsHasConfigWidgetInternal()) {
        auto verStr = "int m_version = " + util::numToString(m_generator.currentSchema().schemaVersion()) + ";";
        repl["VERSION_STORAGE"] = std::move(verStr);
    }

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

bool ToolsQtPlugin::toolsWritePluginSrcInternal() 
{
    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + toolsRelFilePath(toolsPluginClassNameInternal()) + strings::cppSourceSuffixStr();

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#include \"#^#PROTOCOL_CLASS_NAME#$#.h\"\n\n"
        "#^#WIDGET_INCLUDE#$#\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"    
        "namespace plugin\n"
        "{\n\n"    
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() :\n"
        "    m_protocol(new #^#PROTOCOL_CLASS_NAME#$#())\n"
        "{\n"
        "    pluginProperties()\n"
        "        .setProtocolCreateFunc(\n"
        "            [this]() noexcept -> cc_tools_qt::ProtocolPtr\n"
        "            {\n"
        "                return m_protocol;\n"
        "            })\n"
        "        #^#CONFIG_WIDGET_FUNC#$#"
        "    ;\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n\n"
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsPluginClassNameInternal()},
        {"PROTOCOL_CLASS_NAME", toolsProtClassNameInternal()},
    };        

    if (toolsHasConfigWidgetInternal()) {
        static const std::string WidgetTempl =
            ".setConfigWidgetCreateFunc(\n"
            "    [this]() -> QWidget*\n"
            "    {\n"
            "        auto* w =\n"
            "            new #^#WIDGET_CLASS#$#(\n"
            "                static_cast<#^#PROT_CLASS#$#*>(m_protocol.get())->getVersion());\n"
            "        w->setVersionUpdateCb(\n"
            "            [this](int value) {\n"
            "                static_cast<#^#PROT_CLASS#$#*>(m_protocol.get())->setVersion(value);\n"
            "            });\n"
            "        return w;\n"
            "    })\n";

        util::ReplacementMap widgetRepl = {
            {"WIDGET_CLASS", toolsConfigWidgetClassNameInternal()},
            {"PROT_CLASS", toolsProtClassNameInternal()}
        };

        repl["WIDGET_INCLUDE"] = "#include \"" + toolsConfigWidgetClassNameInternal() + ".h\"";
        repl["CONFIG_WIDGET_FUNC"] = util::processTemplate(WidgetTempl, widgetRepl);
    }

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

bool ToolsQtPlugin::toolsWritePluginJsonInternal()
{
    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + toolsRelFilePath(toolsPluginClassNameInternal()) + ".json";

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "{\n"
        "    \"name\" : \"#^#NAME#$#\",\n"
        "    \"desc\" : [\n"
        "        #^#DESC#$#\n"
        "    ],\n"
        "    \"type\" : \"protocol\"\n"
        "}\n";

    auto name = toolsAdjustedNameInternal() + " Protocol";
    auto desc = util::strMakeMultiline(m_description);
    if (!desc.empty()) {
        desc = '\"' + desc + '\"';
        util::strReplace(desc, "\n", "\",\n\"");
    }        

    util::ReplacementMap repl = {
        {"NAME", std::move(name)},
        {"DESC", std::move(desc)},
    };        

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;   
}

bool ToolsQtPlugin::toolsWritePluginConfigInternal()
{
    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + 
        toolsRelFilePath(comms::className(util::strToName(toolsAdjustedNameInternal()))) + ".cfg";

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "{\n"
        "    \"cc_plugins_list\": [\n"
        "        \"cc.EchoSocketPlugin\",\n"
        "        \"#^#ID#$#\"\n"
        "    ]\n"
        "}\n";

    util::ReplacementMap repl = {
        {"ID", toolsAdjustedNameInternal()},
    };        

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;   
}

bool ToolsQtPlugin::toolsWriteConfigWidgetHeaderInternal() 
{
    if (!toolsHasConfigWidgetInternal()) {
        return true;
    }

    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + toolsRelFilePath(toolsConfigWidgetClassNameInternal()) + strings::cppHeaderSuffixStr();

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#include <functional>\n"
        "#include <QtWidgets/QWidget>\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"    
        "namespace plugin\n"
        "{\n\n"    
        "class #^#CLASS_NAME#$# : public QWidget\n"
        "{\n"
        "    Q_OBJECT\n"
        "public:\n"
        "    using VersionUpdateCb = std::function<void (int)>;\n\n"
        "    explicit #^#CLASS_NAME#$#(int version);\n\n"
        "    template <typename TFunc>\n"
        "    void setVersionUpdateCb(TFunc&& func)\n"
        "    {\n"
        "        m_versionUpdateCb = std::forward<TFunc>(func);\n"
        "    }\n\n"
        "private slots:\n"
        "    void versionChanged(int value);\n\n"
        "private:\n"
        "    VersionUpdateCb m_versionUpdateCb;"
        "};\n\n"
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsConfigWidgetClassNameInternal()},
    };        

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

bool ToolsQtPlugin::toolsWriteConfigWidgetSrcInternal() 
{
    if (!toolsHasConfigWidgetInternal()) {
        return true;
    }

    static_cast<void>(m_generator);
    auto filePath = 
        m_generator.getOutputDir() + '/' + toolsRelFilePath(toolsConfigWidgetClassNameInternal()) + strings::cppSourceSuffixStr();

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#include <QtWidgets/QHBoxLayout>\n"
        "#include <QtWidgets/QLabel>\n"
        "#include <QtWidgets/QSpacerItem>\n"
        "#include <QtWidgets/QSpinBox>\n"
        "#include <QtWidgets/QVBoxLayout>\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"    
        "namespace plugin\n"
        "{\n\n"    
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#(int version)"
        "{\n"
        "    auto* versionLabel = new QLabel(\"Default Version:\");\n"
        "    auto* versionSpinBox = new QSpinBox;\n"
        "    versionSpinBox->setMaximum(99999999);\n"
        "    versionSpinBox->setValue(version);\n"
        "    auto* versionSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);\n"
        "    auto* versionLayoutLayout = new QHBoxLayout();\n"
        "    versionLayoutLayout->addWidget(versionLabel);\n"
        "    versionLayoutLayout->addWidget(versionSpinBox);\n"
        "    versionLayoutLayout->addItem(versionSpacer);\n\n"
        "    auto* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);\n\n"
        "    auto* verticalLayout = new QVBoxLayout(this);\n"
        "    verticalLayout->addLayout(versionLayoutLayout);\n"
        "    verticalLayout->addItem(verticalSpacer);\n\n"
        "    setLayout(verticalLayout);\n\n"
        "    connect(\n"
        "        versionSpinBox, SIGNAL(valueChanged(int)),\n"
        "        this, SLOT(versionChanged(int)));\n"
        "}\n\n"
        "void #^#CLASS_NAME#$#::versionChanged(int value)\n"
        "{\n"
        "    if (m_versionUpdateCb) {\n"
        "        m_versionUpdateCb(value);\n"
        "    }\n"
        "}\n\n"
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsConfigWidgetClassNameInternal()},
    };        

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

const std::string& ToolsQtPlugin::toolsAdjustedNameInternal() const
{
    auto* nameToUse = &m_name;
    if (nameToUse->empty()) {
        nameToUse = &m_generator.currentSchema().schemaName();
    }
    return *nameToUse;
}

std::string ToolsQtPlugin::toolsProtClassNameInternal() const
{
    return comms::className(util::strToName(toolsAdjustedNameInternal())) + ProtSuffix;
}

std::string ToolsQtPlugin::toolsPluginClassNameInternal() const
{
    return comms::className(util::strToName(toolsAdjustedNameInternal())) + PluginSuffix;
}

std::string ToolsQtPlugin::toolsConfigWidgetClassNameInternal() const
{
    return comms::className(util::strToName(toolsAdjustedNameInternal())) + WidgetSuffix;
}

bool ToolsQtPlugin::toolsHasConfigWidgetInternal() const
{
    assert(m_interfacePtr != nullptr);
    return (m_interfacePtr->hasVersionField());
}

std::string ToolsQtPlugin::toolsRelFilePath(const std::string& name) const
{
    return 
        m_generator.getTopNamespace() + '/' + m_generator.protocolSchema().mainNamespace() + '/' + 
        strings::pluginNamespaceStr() + '/' + name;
}

} // namespace commsdsl2tools_qt