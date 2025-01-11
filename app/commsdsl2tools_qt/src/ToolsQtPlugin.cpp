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

const std::string ProtPrefix("Protocol_");
const std::string PluginPrefix("Plugin_");

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
        toolsWritePluginConfigInternal();
}

std::string ToolsQtPlugin::toolsProtocolName() const
{
    return util::strToName(toolsAdjustedNameInternal());
}

std::string ToolsQtPlugin::toolsInterfaceName() const
{
    auto iFaceScope = comms::scopeFor(*m_interfacePtr, m_generator, false, true);
    return util::strReplace(iFaceScope, "::", "_");
}

bool ToolsQtPlugin::toolsWriteProtocolHeaderInternal() 
{
    auto relPath = toolsRelFilePath(toolsProtClassNameInternal()) + strings::cppHeaderSuffixStr();
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), relPath);

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

    auto replacePath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::replaceFileSuffixStr());
    auto replaceCode = util::readFileContents(replacePath);    

    if (!replaceCode.empty()) {
        stream << replaceCode;
        stream.flush();
        return stream.good();        
    }

    auto extendPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::extendFileSuffixStr());
    auto extendCode = util::readFileContents(extendPath);

    auto incPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::incFileSuffixStr());
    auto incCode = util::readFileContents(incPath);    

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#include \"cc_tools_qt/ToolsProtocol.h\"\n\n"
        "#^#INC#$#\n\n"
        "#^#TOP_NS_BEGIN#$#\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"        
        "namespace plugin\n"
        "{\n\n"
        "class #^#CLASS_NAME#$##^#ORIG#$# : public cc_tools_qt::ToolsProtocol\n"
        "{\n"
        "    using Base = cc_tools_qt::ToolsProtocol;\n"
        "public:\n"
        "    #^#CLASS_NAME#$##^#ORIG#$#();\n"
        "    virtual ~#^#CLASS_NAME#$##^#ORIG#$#();\n\n"
        "protected:\n"
        "    virtual const QString& nameImpl() const override;\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "#^#TOP_NS_END#$#\n\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"TOP_NS_BEGIN", m_generator.toolsNamespaceBegin()},
        {"TOP_NS_END", m_generator.toolsNamespaceEnd()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsProtClassNameInternal()},
        {"EXTEND", extendCode},
        {"INC", incCode},
    };   

    if (!extendCode.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
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
    auto relPath = toolsRelFilePath(toolsProtClassNameInternal()) + strings::cppSourceSuffixStr();
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), relPath);    

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto replacePath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::replaceFileSuffixStr());
    auto replaceCode = util::readFileContents(replacePath);    

    if (!replaceCode.empty()) {
        stream << replaceCode;
        stream.flush();
        return stream.good();        
    }

    auto extendPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::extendFileSuffixStr());
    auto extendCode = util::readFileContents(extendPath);

    auto incPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::incFileSuffixStr());
    auto incCode = util::readFileContents(incPath);

    util::StringsList includes = {
        m_framePtr->toolsHeaderFilePath(*m_interfacePtr)
    };
    comms::prepareIncludeStatement(includes);

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#^#INCLUDES#$#\n"
        "\n"
        "#^#INC#$#\n\n"
        "#^#TOP_NS_BEGIN#$#\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"
        "namespace plugin\n"
        "{\n\n"
        "#^#CLASS_NAME#$##^#ORIG#$#::#^#CLASS_NAME#$##^#ORIG#$#() :\n"
        "    Base(std::make_unique<#^#FRAME#$#>())\n"
        "{\n"
        "}\n\n"
        "#^#CLASS_NAME#$##^#ORIG#$#::~#^#CLASS_NAME#$##^#ORIG#$#() = default;\n\n"
        "const QString& #^#CLASS_NAME#$##^#ORIG#$#::nameImpl() const\n"
        "{\n"
        "        static const QString Str(\"#^#PROT_NAME#$#\");\n"
        "        return Str;\n"
        "}\n\n"
        "#^#EXTEND#$#\n"      
        "} // namespace plugin\n\n"       
        "} // namespace #^#MAIN_NS#$#\n\n"
        "#^#TOP_NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"TOP_NS_BEGIN", m_generator.toolsNamespaceBegin()},
        {"TOP_NS_END", m_generator.toolsNamespaceEnd()},
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsProtClassNameInternal()},
        {"FRAME", m_framePtr->toolsClassScope(*m_interfacePtr)},
        {"PROT_NAME", toolsAdjustedNameInternal()},
        {"INCLUDES", util::strListToString(includes, "\n", "")},
        {"INC", incCode},
        {"EXTEND", extendCode},
    };    

    if (!extendCode.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
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
    auto relPath = toolsRelFilePath(toolsPluginClassNameInternal()) + strings::cppHeaderSuffixStr();
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), relPath);

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto replaceFilePath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::replaceFileSuffixStr());
    auto replaceCode = util::readFileContents(replaceFilePath);
    if (!replaceCode.empty()) {
        stream << replaceCode;
        stream.flush();
        return stream.good();
    }    

    auto extendPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::extendFileSuffixStr());
    auto extendCode = util::readFileContents(extendPath);

    auto incPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::incFileSuffixStr());
    auto incCode = util::readFileContents(incPath);

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#include \"cc_tools_qt/ToolsPlugin.h\"\n\n"
        "#include <QtCore/QObject>\n"
        "#include <QtCore/QtPlugin>\n"
        "#^#INC#$#\n\n"
        "#^#TOP_NS_BEGIN#$#\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"    
        "namespace plugin\n"
        "{\n\n"    
        "class #^#CLASS_NAME#$##^#ORIG#$# : public cc_tools_qt::ToolsPlugin\n"
        "{\n"
        "    #^#EXTEND_COMMENT#$#\n"
        "    #^#COMMENT#$#Q_OBJECT\n"
        "    #^#COMMENT#$#Q_PLUGIN_METADATA(IID \"#^#ID#$#\" FILE \"#^#CLASS_NAME#$#.json\")\n"
        "    #^#COMMENT#$#Q_INTERFACES(cc_tools_qt::ToolsPlugin)\n"
        "    using Base = cc_tools_qt::ToolsPlugin;\n\n"
        "public:\n"
        "    #^#CLASS_NAME#$##^#ORIG#$#();\n"
        "    virtual ~#^#CLASS_NAME#$##^#ORIG#$#();\n\n"
        "protected:\n"
        "    virtual cc_tools_qt::ToolsProtocolPtr createProtocolImpl() override;\n"
        "};\n\n"
        "#^#EXTEND#$#\n\n"
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "#^#TOP_NS_END#$#\n"
    ;

    assert(!m_pluginId.empty());
    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"TOP_NS_BEGIN", m_generator.toolsNamespaceBegin()},
        {"TOP_NS_END", m_generator.toolsNamespaceEnd()},        
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsPluginClassNameInternal()},
        {"ID", m_pluginId},
        {"EXTEND", extendCode},
        {"INC", incCode},
    };        

    if (!extendCode.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
        repl["EXTEND_COMMENT"] = "// Make sure to add the following lines in the actual deriving class.";
        repl["COMMENT"] = "// ";
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
    auto relPath = toolsRelFilePath(toolsPluginClassNameInternal()) + strings::cppSourceSuffixStr();
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), relPath);

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto replaceFilePath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::replaceFileSuffixStr());
    auto replaceCode = util::readFileContents(replaceFilePath);
    if (!replaceCode.empty()) {
        stream << replaceCode;
        stream.flush();
        return stream.good();
    }     

    auto extendPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::extendFileSuffixStr());
    auto extendCode = util::readFileContents(extendPath);

    auto incPath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::incFileSuffixStr());
    auto incCode = util::readFileContents(incPath);    

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#include \"#^#PROTOCOL_CLASS_NAME#$#.h\"\n\n"
        "#^#INC#$#\n\n"
        "#^#TOP_NS_BEGIN#$#\n"
        "namespace #^#MAIN_NS#$#\n"
        "{\n\n"    
        "namespace plugin\n"
        "{\n\n"    
        "#^#CLASS_NAME#$##^#ORIG#$#::#^#CLASS_NAME#$##^#ORIG#$#() :\n"
        "    Base(Type_Protocol)\n"
        "{\n"
        "}\n\n"
        "#^#CLASS_NAME#$##^#ORIG#$#::~#^#CLASS_NAME#$##^#ORIG#$#() = default;\n\n"
        "cc_tools_qt::ToolsProtocolPtr #^#CLASS_NAME#$##^#ORIG#$#::createProtocolImpl()\n"
        "{\n"
        "    return cc_tools_qt::ToolsProtocolPtr(new #^#PROTOCOL_CLASS_NAME#$#());\n"
        "}\n\n"
        "#^#EXTEND#$#\n\n"
        "} // namespace plugin\n\n"
        "} // namespace #^#MAIN_NS#$#\n\n"
        "#^#TOP_NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"TOP_NS_BEGIN", m_generator.toolsNamespaceBegin()},
        {"TOP_NS_END", m_generator.toolsNamespaceEnd()},  
        {"MAIN_NS", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", toolsPluginClassNameInternal()},
        {"PROTOCOL_CLASS_NAME", toolsProtClassNameInternal()},
        {"EXTEND", extendCode},
        {"INC", incCode},
    };        

    if (!extendCode.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
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
    auto relPath = toolsRelFilePath(toolsPluginClassNameInternal()) + ".json";
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), relPath);

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto replaceFilePath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::replaceFileSuffixStr());
    auto replaceCode = util::readFileContents(replaceFilePath);
    if (!replaceCode.empty()) {
        stream << replaceCode;
        stream.flush();
        return stream.good();
    }     

    static const std::string Templ =
        "{\n"
        "    \"name\" : \"#^#NAME#$#\",\n"
        "    \"desc\" : [\n"
        "        #^#DESC#$#\n"
        "    ],\n"
        "    \"type\" : \"protocol\"\n"
        "}\n";

    const std::string ProtocolSuffix("Protocol");
    auto name = toolsAdjustedNameInternal();
    if (!util::strEndsWith(name, ProtocolSuffix)) {
        name += ' ' + ProtocolSuffix;
    }

    auto desc = util::strReplace(m_description, "\\n", "\n");
    desc = util::strMakeMultiline(desc, 60, false);
    if (!desc.empty()) {
        desc = '\"' + desc + '\"';
        desc = util::strReplace(desc, "\n", "\",\n\"");
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
    auto relPath = toolsRelFilePath(toolsProtocolName()) + ".cfg";
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), relPath);
        
    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto replaceFilePath = util::pathAddElem(m_generator.getCodeDir(), relPath + strings::replaceFileSuffixStr());
    auto replaceCode = util::readFileContents(replaceFilePath);
    if (!replaceCode.empty()) {
        stream << replaceCode;
        stream.flush();
        return stream.good();
    }      

    static const std::string Templ =
        "{\n"
        "    \"cc_plugins_list\": [\n"
        "        \"cc.EchoSocketPlugin\",\n"
        "        \"#^#ID#$#\"\n"
        "    ]\n"
        "}\n";

    util::ReplacementMap repl = {
        {"ID", m_pluginId},
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
        nameToUse = &m_generator.protocolSchema().schemaName();
    }
    return *nameToUse;
}

std::string ToolsQtPlugin::toolsProtClassNameInternal() const
{
    return ProtPrefix + toolsProtocolName();
}

std::string ToolsQtPlugin::toolsPluginClassNameInternal() const
{
    return PluginPrefix + toolsProtocolName();
}

std::string ToolsQtPlugin::toolsRelFilePath(const std::string& name) const
{
    auto prefix = util::strReplace(m_generator.toolsScopePrefix(), "::", "/");
    return 
        prefix + m_generator.protocolSchema().mainNamespace() + '/' + 
        strings::pluginNamespaceStr() + '/' + name;
}

} // namespace commsdsl2tools_qt
