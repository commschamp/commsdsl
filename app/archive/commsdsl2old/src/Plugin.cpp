//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "Plugin.h"

#include <cassert>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "Generator.h"

namespace ba = boost::algorithm;
namespace bf = boost::filesystem;

namespace commsdsl2old
{

namespace
{

const std::string ProtSuffix("Protocol");
const std::string PluginSuffix("Plugin");
const std::string WidgetSuffix("ConfigWidget");

} // namespace

bool Plugin::prepare()
{
    m_framePtr = m_generator.findFrame(m_frame);
    if (m_framePtr == nullptr) {
        m_generator.logger().error("Frame \"" + m_frame + "\" hasn't been defined.");
        return false;
    }

    m_interfacePtr = m_generator.findInterface(m_interface);
    if (m_interfacePtr == nullptr) {
        m_generator.logger().error("Interface \"" + m_interface + "\" hasn't been defined.");
        return false;
    }

    return true;
}

bool Plugin::write()
{
    return
        writeProtocolHeader() &&
        writeProtocolSrc() &&
        writePluginHeader() &&
        writePluginSrc() &&
        writePluginJson() &&
        writePluginConfig() &&
        writeVersionConfigWidgetHeader() &&
        writeVersionConfigWidgetSrc();
}

const std::string& Plugin::adjustedName() const
{
    auto* nameToUse = &m_name;
    if (nameToUse->empty()) {
        nameToUse = &m_generator.schemaName();
    }
    return *nameToUse;
}

bool Plugin::hasConfigWidget() const
{
    assert(m_interfacePtr != nullptr);
    return (m_interfacePtr->hasVersion());
}

bool Plugin::writeProtocolHeader()
{
    auto startInfo = m_generator.startProtocolPluginHeaderWrite(protClassName());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "#pragma once\n\n"
        "#include \"cc_tools_qt/Protocol.h\"\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
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
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n"
    ;

    auto namespaces = m_generator.namespacesForPluginDef(className);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForPluginHeaderInPlugin(protClassName())));

    if (hasConfigWidget()) {
        static const std::string VerApi =
            "int getVersion() const;\n"
            "void setVersion(int value);\n";
        replacements.insert(std::make_pair("VERSION_API", VerApi));
    }

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Plugin::writeProtocolSrc()
{
    auto startInfo = m_generator.startProtocolPluginSrcWrite(protClassName());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
    "#^#GEN_COMMENT#$#\n"
    "#include \"#^#CLASS_NAME#$#.h\"\n\n"
    "#include <cassert>\n"
    "#include \"cc_tools_qt/ProtocolBase.h\"\n"
    "#^#INTERFACE_INC#$#\n"
    "#include \"#^#FRAME_HEADER#$#\"\n"
    "#include \"#^#TRANSPORT_MESSAGE_HEADER#$#\"\n\n"
    "namespace cc = cc_tools_qt;\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "class #^#CLASS_NAME#$#Impl : public\n"
    "    cc::ProtocolBase<\n"
    "        #^#FRAME#$##^#INTERFACE_TEMPL_PARAM#$#,\n"
    "        #^#FRAME#$#TransportMessage#^#INTERFACE_TEMPL_PARAM#$#\n"
    "    >\n"
    "{\n"
    "    using Base =\n"
    "        cc::ProtocolBase<\n"
    "            #^#FRAME#$##^#INTERFACE_TEMPL_PARAM#$#,\n"
    "            #^#FRAME#$#TransportMessage#^#INTERFACE_TEMPL_PARAM#$#\n"
    "        >;\n"
    "public:\n"
    "    friend class #^#PROT_NAMESPACE#$#::cc_plugin::plugin::#^#CLASS_NAME#$#;\n\n"
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
    "#^#CLASS_NAME#$#::MessagesList #^#CLASS_NAME#$#::readImpl(const cc::DataInfo& dataInfo, bool final)\n"
    "{\n"
    "    return m_pImpl->read(dataInfo, final);\n"
    "}\n\n"
    "cc::DataInfoPtr #^#CLASS_NAME#$#::writeImpl(cc::Message& msg)\n"
    "{\n"
    "    return m_pImpl->write(msg);\n"
    "}\n\n"
    "#^#CLASS_NAME#$#::MessagesList #^#CLASS_NAME#$#::createAllMessagesImpl()\n"
    "{\n"
    "    return m_pImpl->createAllMessages();\n"
    "}\n\n"
    "cc::MessagePtr #^#CLASS_NAME#$#::createMessageImpl(const QString& idAsString, unsigned idx)\n"
    "{\n"
    "    return static_cast<cc::Protocol*>(m_pImpl.get())->createMessage(idAsString, idx);\n"
    "}\n\n"
    "#^#CLASS_NAME#$#::UpdateStatus #^#CLASS_NAME#$#::updateMessageImpl(cc::Message& msg)\n"
    "{\n"
    "    return m_pImpl->updateMessage(msg);\n"
    "}\n\n"
    "cc::MessagePtr #^#CLASS_NAME#$#::cloneMessageImpl(const cc::Message& msg)\n"
    "{\n"
    "    return m_pImpl->cloneMessage(msg);\n"
    "}\n\n"
    "cc::MessagePtr #^#CLASS_NAME#$#::createInvalidMessageImpl()\n"
    "{\n"
    "    return m_pImpl->createInvalidMessageImpl();\n"
    "}\n\n"
    "cc::MessagePtr #^#CLASS_NAME#$#::createRawDataMessageImpl()\n"
    "{\n"
    "    return m_pImpl->createRawDataMessageImpl();\n"
    "}\n\n"
    "cc::MessagePtr #^#CLASS_NAME#$#::createExtraInfoMessageImpl()\n"
    "{\n"
    "    return m_pImpl->createExtraInfoMessageImpl();\n"
    "}\n\n"
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n";

    assert(m_framePtr != nullptr);
    auto namespaces = m_generator.namespacesForPluginDef(className);
    auto frameHeader = m_generator.headerfileForFrameInPlugin(m_framePtr->externalRef(), false);
    assert(ba::ends_with(frameHeader, common::headerSuffix()));
    auto transportMsgHeader = frameHeader;
    auto& suffix = common::transportMessageSuffixStr();
    auto insertIter = transportMsgHeader.begin() + (frameHeader.size() - common::headerSuffix().size());
    transportMsgHeader.insert(insertIter, suffix.begin(), suffix.end());

    const auto* protName = &m_name;
    if (protName->empty()) {
        protName = &m_generator.schemaName();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("FRAME_HEADER", std::move(frameHeader)));
    replacements.insert(std::make_pair("TRANSPORT_MESSAGE_HEADER", std::move(transportMsgHeader)));
    replacements.insert(std::make_pair("FRAME", m_generator.scopeForFrameInPlugin(m_framePtr->externalRef())));
    replacements.insert(std::make_pair("PROT_NAME", *protName));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForPluginSrcInPlugin(protClassName())));

    auto* defaultInterface = m_generator.getDefaultInterface();
    if (defaultInterface == nullptr) {
        assert(m_interfacePtr != nullptr);
        replacements.insert(std::make_pair("INTERFACE_TEMPL_PARAM", '<' + m_generator.scopeForInterfaceInPlugin(m_interfacePtr->externalRef()) + '>'));
        replacements.insert(std::make_pair("INTERFACE_INC", "#include " + m_generator.headerfileForInterfaceInPlugin(m_interfacePtr->externalRef(), true)));
    }

    if (hasConfigWidget()) {
        static const std::string VerImplPubTempl =
            "int getVersion() const\n"
            "{\n"
            "    return m_version;\n"
            "}\n\n"
            "void setVersion(int value)\n"
            "{\n"
            "    m_version = value;\n"
            "    #^#UPDATE_FRAME#$#\n"
            "}\n";

        static const std::string VerImplProtected =
            "virtual MessagesList createAllMessagesImpl() override\n"
            "{\n"
            "    auto list = Base::createAllMessagesImpl();\n"
            "    for (auto& mPtr : list) {\n"
            "        updateMessageWithVersion(*mPtr);\n"
            "    }\n"
            "    return list;\n"
            "}\n\n"
            "virtual cc::MessagePtr createMessageImpl(const QString& idAsString, unsigned idx) override\n"
            "{\n"
            "    auto mPtr = Base::createMessageImpl(idAsString, idx);\n"
            "    updateMessageWithVersion(*mPtr);\n"
            "    return mPtr;\n"
            "}\n\n";

        static const std::string VerImplPrivateTempl =
            "private:\n"
            "    void updateMessageWithVersion(cc::Message& msg)\n"
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

        static const std::string VerApiTempl =
            "int #^#CLASS_NAME#$#::getVersion() const\n"
            "{\n"
            "    return m_pImpl->getVersion();\n"
            "}\n\n"
            "void #^#CLASS_NAME#$#::setVersion(int value)\n"
            "{\n"
            "    m_pImpl->setVersion(value);\n"
            "}\n";

        common::ReplacementMap replVerImplPubTempl;

        common::ReplacementMap replVerImplPrivateTempl;
        replVerImplPrivateTempl.insert(std::make_pair("DEFAULT_VERSION", common::numToString(m_generator.schemaVersion())));
        replVerImplPrivateTempl.insert(std::make_pair("INTERFACE_TYPE", m_generator.scopeForInterfaceInPlugin(m_interfacePtr->externalRef())));

        auto versionFields = m_interfacePtr->getVersionFields();
        auto pseudoLayers = m_framePtr->getPseudoVersionLayers(versionFields);
        common::StringsList pseudoUpdates;
        for (auto& l : pseudoLayers) {
            auto layerTypeStr = "LayerType_" + common::nameToClassCopy(l);
            auto layerAccStr = "layer_" + common::nameToAccessCopy(l);
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

            common::ReplacementMap replUpdateFrameTempl;
            replUpdateFrameTempl.insert(std::make_pair("UPDATES", common::listToString(pseudoUpdates, common::emptyString(), common::emptyString())));
            replVerImplPrivateTempl.insert(std::make_pair("UPDATE_FRAME", common::processTemplate(UpdateFrameTempl, replUpdateFrameTempl)));
            replVerImplPubTempl.insert(std::make_pair("UPDATE_FRAME", "updateFrame();"));
        }

        common::ReplacementMap replVerApiTempl;
        replVerApiTempl.insert(std::make_pair("CLASS_NAME", className));

        replacements.insert(std::make_pair("VERSION_IMPL_PUBLIC", common::processTemplate(VerImplPubTempl, replVerImplPubTempl)));
        replacements.insert(std::make_pair("VERSION_IMPL_PROTECTED", VerImplProtected));
        replacements.insert(std::make_pair("VERSION_IMPL_PRIVATE", common::processTemplate(VerImplPrivateTempl, replVerImplPrivateTempl)));
        replacements.insert(std::make_pair("VERSION_API", common::processTemplate(VerApiTempl, replVerApiTempl)));
    }

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Plugin::writePluginHeader()
{
    auto startInfo = m_generator.startProtocolPluginHeaderWrite(pluginClassName());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "#pragma once\n\n"
        "#include <QtCore/QObject>\n"
        "#include <QtCore/QtPlugin>\n"
        "#include \"cc_tools_qt/Plugin.h\"\n"
        "#include \"cc_tools_qt/Protocol.h\"\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "class #^#CLASS_NAME#$# : public cc_tools_qt::Plugin\n"
        "{\n"
        "    Q_OBJECT\n"
        "    Q_PLUGIN_METADATA(IID \"#^#ID#$#\" FILE \"#^#ORIG_CLASS_NAME#$#.json\")\n"
        "    Q_INTERFACES(cc_tools_qt::Plugin)\n\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    virtual ~#^#CLASS_NAME#$#();\n"
        "private:\n"
        "    cc_tools_qt::ProtocolPtr m_protocol;\n"
        "    #^#VERSION_STORAGE#$#\n"
        "};\n\n"
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n";

    auto namespaces = m_generator.namespacesForPluginDef(className);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("ORIG_CLASS_NAME", common::nameToClassCopy(pluginClassName())));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("ID", pluginId()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForPluginHeaderInPlugin(pluginClassName())));

    if (hasConfigWidget()) {
        auto verStr = "int m_version = " + common::numToString(m_generator.schemaVersion()) + ";";
        replacements.insert(std::make_pair("VERSION_STORAGE", std::move(verStr)));
    }

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Plugin::writePluginSrc()
{
    auto startInfo = m_generator.startProtocolPluginSrcWrite(pluginClassName());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#include \"#^#PROTOCOL_CLASS_NAME#$#.h\"\n\n"
        "#^#WIDGET_INCLUDE#$#\n"
        "namespace cc = cc_tools_qt;\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#()\n"
        "  : m_protocol(new #^#PROTOCOL_CLASS_NAME#$#())\n"
        "{\n"
        "    pluginProperties()\n"
        "        .setProtocolCreateFunc(\n"
        "            [this]() noexcept -> cc::ProtocolPtr\n"
        "            {\n"
        "                return m_protocol;\n"
        "            })\n"
        "        #^#CONFIG_WIDGET_FUNC#$#"
        "    ;\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n\n"
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n";

    auto namespaces = m_generator.namespacesForPluginDef(className);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("PROTOCOL_CLASS_NAME", protClassName()));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForPluginSrcInPlugin(pluginClassName())));

    if (hasConfigWidget()) {
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

        common::ReplacementMap widgetRepl;
        widgetRepl.insert(std::make_pair("WIDGET_CLASS", configWidgetClassName()));
        widgetRepl.insert(std::make_pair("PROT_CLASS", protClassName()));

        replacements.insert(std::make_pair("WIDGET_INCLUDE", "#include \"" + configWidgetClassName() + ".h\""));
        replacements.insert(std::make_pair("CONFIG_WIDGET_FUNC", common::processTemplate(WidgetTempl, widgetRepl)));
    }

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Plugin::writePluginJson()
{
    auto filePath = m_generator.startProtocolPluginCommonWrite(pluginClassName(), ".json");

    if (filePath.empty()) {
        return true;
    }

    static const std::string Templ =
        "{\n"
        "    \"name\" : \"#^#NAME#$#\",\n"
        "    \"desc\" : [\n"
        "        #^#DESC#$#\n"
        "    ],\n"
        "    \"type\" : \"protocol\"\n"
        "}\n";

    auto name = adjustedName() + " Protocol";
    auto desc = common::makeMultilineCopy(m_description);
    if (!desc.empty()) {
        desc = '\"' + desc + '\"';
        ba::replace_all(desc, "\n", "\",\n\"");
    }
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", std::move(name)));
    replacements.insert(std::make_pair("DESC", std::move(desc)));

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Plugin::writePluginConfig()
{
    auto filePath = m_generator.startProtocolPluginCommonWrite(common::nameToClassCopy(common::updateNameCopy(adjustedName())), ".cfg");

    if (filePath.empty()) {
        return true;
    }

    static const std::string Templ =
        "{\n"
        "    \"cc_plugins_list\": [\n"
        "        \"cc.EchoSocketPlugin\",\n"
        "        \"#^#ID#$#\"\n"
        "    ]\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ID", pluginId()));
    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Plugin::writeVersionConfigWidgetHeader()
{
    if (!hasConfigWidget()) {
        return true;
    }

    auto startInfo = m_generator.startProtocolPluginHeaderWrite(configWidgetClassName());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "#pragma once\n\n"
        "#include <functional>\n"
        "#include <QtWidgets/QWidget>\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
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
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n"
    ;

    auto namespaces = m_generator.namespacesForPluginDef(className);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForPluginHeaderInPlugin(configWidgetClassName())));

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Plugin::writeVersionConfigWidgetSrc()
{
    if (!hasConfigWidget()) {
        return true;
    }

    auto startInfo = m_generator.startProtocolPluginSrcWrite(configWidgetClassName());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#include <QtWidgets/QHBoxLayout>\n"
        "#include <QtWidgets/QLabel>\n"
        "#include <QtWidgets/QSpacerItem>\n"
        "#include <QtWidgets/QSpinBox>\n"
        "#include <QtWidgets/QVBoxLayout>\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
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
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n"
    ;

    auto namespaces = m_generator.namespacesForPluginDef(className);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForPluginHeaderInPlugin(configWidgetClassName())));

    std::string str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Plugin::protClassName() const
{
    return common::nameToClassCopy(common::updateNameCopy(adjustedName())) + ProtSuffix;
}

std::string Plugin::pluginClassName() const
{
    return common::nameToClassCopy(common::updateNameCopy(adjustedName())) + PluginSuffix;
}

std::string Plugin::configWidgetClassName() const
{
    return common::nameToClassCopy(common::updateNameCopy(adjustedName())) + WidgetSuffix;
}

std::string Plugin::pluginId() const
{
    auto id = m_generator.schemaName();
    if (!m_name.empty()) {
        id += '.' + m_name;
    }

    return id;
}

} // namespace commsdsl2old
