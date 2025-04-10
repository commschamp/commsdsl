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

#include "ToolsQtMsgFactory.h"

#include "ToolsQtGenerator.h"
#include "ToolsQtInterface.h"
#include "ToolsQtMessage.h"
#include "ToolsQtNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

const std::string MsgFactoryName = "MsgFactory";

} // namespace

ToolsQtMsgFactory::ToolsQtMsgFactory(const ToolsQtGenerator& generator, const ToolsQtNamespace& parent) :
    m_generator(generator),
    m_parent(parent)
{
}

bool ToolsQtMsgFactory::toolsWrite() const
{
    return
        toolsWriteHeaderInternal() &&
        toolsWriteSourceInternal();
}

std::string ToolsQtMsgFactory::toolsRelHeaderPath(const commsdsl::gen::Interface& iFace) const
{
    return toolsRelPathInternal(iFace) + strings::cppHeaderSuffixStr();
}

ToolsQtMsgFactory::StringsList ToolsQtMsgFactory::toolsSourceFiles(const commsdsl::gen::Interface& iFace) const
{
    StringsList result = {
        toolsRelPathInternal(iFace) + strings::cppSourceSuffixStr()
    };

    return result;
}

std::string ToolsQtMsgFactory::toolsClassScope(const commsdsl::gen::Interface& iFace) const
{
    return m_generator.toolsScopePrefixForInterface(iFace) + comms::scopeForFactory(MsgFactoryName, m_generator, m_parent);
}

std::string ToolsQtMsgFactory::toolsRelPathInternal(const commsdsl::gen::Interface& iFace) const
{
    return util::strReplace(toolsClassScope(iFace), "::", "/");
}

bool ToolsQtMsgFactory::toolsWriteHeaderInternal() const
{
    auto& logger = m_generator.logger();

    auto& allInterfaces = m_generator.toolsGetSelectedInterfaces();
    
    for (auto* iFace : allInterfaces) {
        assert(iFace != nullptr);
        auto filePath = m_generator.getOutputDir() + '/' + toolsRelHeaderPath(*iFace);
        logger.info("Generating " + filePath);

        auto dirPath = util::pathUp(filePath);
        assert(!dirPath.empty());
        if (!m_generator.createDirectory(dirPath)) {
            return false;
        }        

        util::StringsList includes = {
            "cc_tools_qt/ToolsMsgFactory.h",
        };

        comms::prepareIncludeStatement(includes);

        std::ofstream stream(filePath);
        if (!stream) {
            logger.error("Failed to open \"" + filePath + "\" for writing.");
            return false;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "#pragma once\n\n"
            "#^#INCLUDES#$#\n"
            "#^#TOP_NS_BEGIN#$#\n\n"
            "#^#NS_BEGIN#$#\n"
            "namespace #^#FACTORY_NAMESPACE#$#\n"
            "{\n\n"
            "#^#DEF#$#\n\n"
            "} // namespace #^#FACTORY_NAMESPACE#$#\n\n"
            "#^#NS_END#$#\n\n"
            "#^#TOP_NS_END#$#\n"
        ;

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"INCLUDES", util::strListToString(includes, "\n", "\n")},
            {"TOP_NS_BEGIN", m_generator.toolsNamespaceBeginForInterface(*iFace)},
            {"TOP_NS_END", m_generator.toolsNamespaceEndForInterface(*iFace)},
            {"NS_BEGIN", comms::namespaceBeginFor(m_parent, m_generator)},
            {"NS_END", comms::namespaceEndFor(m_parent, m_generator)},             
            {"DEF", toolsHeaderCodeInternal()},
            {"FACTORY_NAMESPACE", strings::factoryNamespaceStr()}
        };
        
        stream << util::processTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.error("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }

    return true;
}

bool ToolsQtMsgFactory::toolsWriteSourceInternal() const
{
    auto& logger = m_generator.logger();

    auto& allInterfaces = m_generator.toolsGetSelectedInterfaces();
    
    for (auto* iFace : allInterfaces) {
        assert(iFace != nullptr);
        auto filePath = m_generator.getOutputDir() + '/' + toolsRelPathInternal(*iFace) + strings::cppSourceSuffixStr();

        logger.info("Generating " + filePath);

        auto dirPath = util::pathUp(filePath);
        assert(!dirPath.empty());
        if (!m_generator.createDirectory(dirPath)) {
            return false;
        }        

        std::ofstream stream(filePath);
        if (!stream) {
            logger.error("Failed to open \"" + filePath + "\" for writing.");
            return false;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "\n"
            "#include \"#^#CLASS_NAME#$#.h\"\n\n"
            "#^#INCLUDES#$#\n"
            "#^#TOP_NS_BEGIN#$#\n"        
            "#^#NS_BEGIN#$#\n"
            "namespace #^#FACTORY_NAMESPACE#$#\n"
            "{\n\n"
            "#^#CODE#$#\n\n"
            "} // namespace #^#FACTORY_NAMESPACE#$#\n\n"
            "#^#NS_END#$#\n\n"
            "#^#TOP_NS_END#$#\n"
        ;

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"INCLUDES", toolsSourceIncludesInternal(*iFace)},
            {"CODE", toolsSourceCodeInternal(*iFace)},
            {"CLASS_NAME", MsgFactoryName},
            {"TOP_NS_BEGIN", m_generator.toolsNamespaceBeginForInterface(*iFace)},
            {"TOP_NS_END", m_generator.toolsNamespaceEndForInterface(*iFace)},
            {"NS_BEGIN", comms::namespaceBeginFor(m_parent, m_generator)},
            {"NS_END", comms::namespaceEndFor(m_parent, m_generator)},              
            {"FACTORY_NAMESPACE", strings::factoryNamespaceStr()}
        };
        
        stream << util::processTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.error("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }
    return true;
}

std::string ToolsQtMsgFactory::toolsHeaderCodeInternal() const
{
    const std::string Templ = 
        "class #^#CLASS_NAME#$# : public cc_tools_qt::ToolsMsgFactory\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    virtual ~#^#CLASS_NAME#$#();\n"
        "\n"
        "protected:\n"
        "    virtual MessagesListInternal createAllMessagesImpl() override;\n"
        "};\n";  

    util::ReplacementMap repl = {
        {"CLASS_NAME", MsgFactoryName},
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtMsgFactory::toolsSourceCodeInternal(const commsdsl::gen::Interface& iFace) const
{
    const std::string Templ = 
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() = default;\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n\n"
        "#^#CLASS_NAME#$#::MessagesListInternal #^#CLASS_NAME#$#::createAllMessagesImpl()\n"
        "{\n"
        "    return\n"
        "        MessagesListInternal{\n"
        "            #^#MESSAGES#$#\n"
        "        };\n"
        "}\n";


    util::StringsList scopes;
    auto allMessages = m_generator.getAllMessagesIdSorted();
    for (auto* m : allMessages) {
        assert(m != nullptr);

        scopes.push_back("cc_tools_qt::ToolsMessagePtr(new " + ToolsQtMessage::cast(*m).toolsClassScope(iFace) + ")");
    }    

    util::ReplacementMap repl = {
        {"CLASS_NAME", MsgFactoryName},
        {"MESSAGES", util::strListToString(scopes, ",\n", "")},
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtMsgFactory::toolsSourceIncludesInternal(const commsdsl::gen::Interface& iFace) const
{
    util::StringsList includes;
    auto allMessages = m_generator.getAllMessagesIdSorted();
    for (auto* m : allMessages) {
        assert(m != nullptr);
        auto& castedMsg = ToolsQtMessage::cast(*m);
        includes.push_back(castedMsg.toolsHeaderPath(iFace));
    }   

    comms::prepareIncludeStatement(includes);
    return util::strListToString(includes, "\n", "\n");
}

} // namespace commsdsl2tools_qt
