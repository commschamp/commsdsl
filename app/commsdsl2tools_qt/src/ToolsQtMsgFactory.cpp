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

const std::string ToolsMsgFactoryName = "MsgFactory";

} // namespace

ToolsQtMsgFactory::ToolsQtMsgFactory(const ToolsQtGenerator& generator, const ToolsQtNamespace& parent) :
    m_toolsGenerator(generator),
    m_parent(parent)
{
}

bool ToolsQtMsgFactory::toolsWrite() const
{
    return
        toolsWriteHeaderInternal() &&
        toolsWriteSourceInternal();
}

std::string ToolsQtMsgFactory::toolsRelHeaderPath(const commsdsl::gen::GenInterface& iFace) const
{
    return toolsRelPathInternal(iFace) + strings::genCppHeaderSuffixStr();
}

ToolsQtMsgFactory::GenStringsList ToolsQtMsgFactory::toolsSourceFiles(const commsdsl::gen::GenInterface& iFace) const
{
    GenStringsList result = {
        toolsRelPathInternal(iFace) + strings::genCppSourceSuffixStr()
    };

    return result;
}

std::string ToolsQtMsgFactory::toolsClassScope(const commsdsl::gen::GenInterface& iFace) const
{
    return m_toolsGenerator.toolsScopePrefixForInterface(iFace) + comms::genScopeForFactory(ToolsMsgFactoryName, m_toolsGenerator, m_parent);
}

std::string ToolsQtMsgFactory::toolsRelPathInternal(const commsdsl::gen::GenInterface& iFace) const
{
    return util::genScopeToRelPath(toolsClassScope(iFace));
}

bool ToolsQtMsgFactory::toolsWriteHeaderInternal() const
{
    auto& logger = m_toolsGenerator.genLogger();

    auto& allInterfaces = m_toolsGenerator.toolsGetSelectedInterfaces();

    for (auto* iFace : allInterfaces) {
        assert(iFace != nullptr);
        auto filePath = m_toolsGenerator.genGetOutputDir() + '/' + toolsRelHeaderPath(*iFace);
        logger.genInfo("Generating " + filePath);

        auto dirPath = util::genPathUp(filePath);
        assert(!dirPath.empty());
        if (!m_toolsGenerator.genCreateDirectory(dirPath)) {
            return false;
        }

        util::GenStringsList includes = {
            "cc_tools_qt/ToolsMsgFactory.h",
        };

        comms::genPrepareIncludeStatement(includes);

        std::ofstream stream(filePath);
        if (!stream) {
            logger.genError("Failed to open \"" + filePath + "\" for writing.");
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

        util::GenReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
            {"TOP_NS_BEGIN", m_toolsGenerator.toolsNamespaceBeginForInterface(*iFace)},
            {"TOP_NS_END", m_toolsGenerator.toolsNamespaceEndForInterface(*iFace)},
            {"NS_BEGIN", comms::genNamespaceBeginFor(m_parent, m_toolsGenerator)},
            {"NS_END", comms::genNamespaceEndFor(m_parent, m_toolsGenerator)},
            {"DEF", toolsHeaderCodeInternal()},
            {"FACTORY_NAMESPACE", strings::genFactoryNamespaceStr()}
        };

        stream << util::genProcessTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.genError("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }

    return true;
}

bool ToolsQtMsgFactory::toolsWriteSourceInternal() const
{
    auto& logger = m_toolsGenerator.genLogger();

    auto& allInterfaces = m_toolsGenerator.toolsGetSelectedInterfaces();

    for (auto* iFace : allInterfaces) {
        assert(iFace != nullptr);
        auto filePath = m_toolsGenerator.genGetOutputDir() + '/' + toolsRelPathInternal(*iFace) + strings::genCppSourceSuffixStr();

        logger.genInfo("Generating " + filePath);

        auto dirPath = util::genPathUp(filePath);
        assert(!dirPath.empty());
        if (!m_toolsGenerator.genCreateDirectory(dirPath)) {
            return false;
        }

        std::ofstream stream(filePath);
        if (!stream) {
            logger.genError("Failed to open \"" + filePath + "\" for writing.");
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

        util::GenReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"INCLUDES", toolsSourceIncludesInternal(*iFace)},
            {"CODE", toolsSourceCodeInternal(*iFace)},
            {"CLASS_NAME", ToolsMsgFactoryName},
            {"TOP_NS_BEGIN", m_toolsGenerator.toolsNamespaceBeginForInterface(*iFace)},
            {"TOP_NS_END", m_toolsGenerator.toolsNamespaceEndForInterface(*iFace)},
            {"NS_BEGIN", comms::genNamespaceBeginFor(m_parent, m_toolsGenerator)},
            {"NS_END", comms::genNamespaceEndFor(m_parent, m_toolsGenerator)},
            {"FACTORY_NAMESPACE", strings::genFactoryNamespaceStr()}
        };

        stream << util::genProcessTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.genError("Write to \"" + filePath + "\" is unsuccessful.");
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

    util::GenReplacementMap repl = {
        {"CLASS_NAME", ToolsMsgFactoryName},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string ToolsQtMsgFactory::toolsSourceCodeInternal(const commsdsl::gen::GenInterface& iFace) const
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

    util::GenStringsList scopes;
    auto allMessages = m_toolsGenerator.genGetAllMessagesIdSorted();
    for (auto* m : allMessages) {
        assert(m != nullptr);

        scopes.push_back("cc_tools_qt::ToolsMessagePtr(new " + ToolsQtMessage::toolsCast(*m).toolsClassScope(iFace) + ")");
    }

    util::GenReplacementMap repl = {
        {"CLASS_NAME", ToolsMsgFactoryName},
        {"MESSAGES", util::genStrListToString(scopes, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string ToolsQtMsgFactory::toolsSourceIncludesInternal(const commsdsl::gen::GenInterface& iFace) const
{
    util::GenStringsList includes;
    auto allMessages = m_toolsGenerator.genGetAllMessagesIdSorted();
    for (auto* m : allMessages) {
        assert(m != nullptr);
        auto& castedMsg = ToolsQtMessage::toolsCast(*m);
        includes.push_back(castedMsg.toolsHeaderPath(iFace));
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

} // namespace commsdsl2tools_qt
