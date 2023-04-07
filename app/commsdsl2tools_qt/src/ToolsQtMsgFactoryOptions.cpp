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

#include "ToolsQtMsgFactoryOptions.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtMsgFactory.h"
#include "ToolsQtNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

const std::string ClassName("MsgFactoryFrameOptions");


} // namespace 
    

std::string ToolsQtMsgFactoryOptions::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    return 
        util::strReplace(toolsScope(generator), "::", "/") + 
        strings::cppHeaderSuffixStr();
}

std::string ToolsQtMsgFactoryOptions::toolsScope(const ToolsQtGenerator& generator)
{
    return 
        generator.getTopNamespace() + "::" + 
        generator.protocolSchema().mainNamespace() + "::" + 
        comms::scopeForOptions(ClassName, generator, false);
}    

bool ToolsQtMsgFactoryOptions::write(ToolsQtGenerator& generator)
{
    ToolsQtMsgFactoryOptions obj(generator);
    return obj.toolsWriteInternal();
}

bool ToolsQtMsgFactoryOptions::toolsWriteInternal() const
{
    auto filePath = m_generator.getOutputDir() + '/' + toolsRelHeaderPath(m_generator);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }      

    m_generator.logger().info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }    

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of the options for framing which force usage of message factory.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#EXTRA_INCLUDES#$#\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"        
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "struct #^#NAME#$# : public #^#DEFAULT_OPTS#$#\n"
        "{\n"
        "    template <typename TInterface, typename TAllMessages, typename... TOptions>\n"
        "    using MsgFactory = #^#MSG_FACTORY#$##^#INTERFACE_TEMPL#$#;\n\n"
        "    #^#CODE#$#\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n";

    auto codePrefix = m_generator.getCodeDir() + '/' + toolsRelHeaderPath(m_generator);

    util::StringsList includes {
        ToolsQtMsgFactory::toolsRelHeaderPath(m_generator),
        ToolsQtDefaultOptions::toolsRelHeaderPath(m_generator)
    };

    comms::prepareIncludeStatement(includes);

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "")},
        {"EXTRA_INCLUDES", util::readFileContents(codePrefix + strings::incFileSuffixStr())},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"PROT_NAMESPACE", m_generator.protocolSchema().mainNamespace()},
        {"NAME", ClassName},
        {"EXTEND", util::readFileContents(codePrefix + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(codePrefix + strings::appendFileSuffixStr())},
        {"DEFAULT_OPTS", ToolsQtDefaultOptions::toolsScope(m_generator)},
        {"MSG_FACTORY", ToolsQtMsgFactory::toolsClassScope(m_generator)},
        {"CODE", toolsOptionsCodeInternal()},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    if (m_generator.toolsHasMulitpleInterfaces()) {
        repl["INTERFACE_TEMPL"] = "<TInterface>";
    }

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

std::string ToolsQtMsgFactoryOptions::toolsOptionsCodeInternal() const
{
    auto& allNs = m_generator.currentSchema().namespaces();
    util::StringsList opts;
    for (auto& nsPtr : allNs) {
        auto elem = ToolsQtNamespace::cast(nsPtr.get())->toolsMsgFactoryOptions();
        if (!elem.empty()) {
            opts.push_back(std::move(elem));
        }
    }

    return util::strListToString(opts, "\n", "");
}


} // namespace commsdsl2tools_qt