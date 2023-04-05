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

#include "ToolsQtDefaultOptions.h"

#include "ToolsQtGenerator.h"
#include "ToolsQtMsgFactory.h"

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

std::string toolsBaseCodeInternal(const ToolsQtGenerator& generator, std::size_t idx)
{
    auto& gen = const_cast<ToolsQtGenerator&>(generator);
    assert(idx < generator.schemas().size());

    auto oldIdx = gen.currentSchemaIdx();
    gen.chooseCurrentSchema(static_cast<unsigned>(idx));
    auto scope = comms::scopeForOptions(strings::defaultOptionsClassStr(), generator);
    gen.chooseCurrentSchema(oldIdx);

    if (idx == 0U) {
        return "::" + scope;
    }

    static const std::string Templ = 
        "::#^#SCOPE#$#T<\n"
        "    #^#NEXT#$#\n"
        ">";

    util::ReplacementMap repl = {
        {"SCOPE", std::move(scope)},
        {"NEXT", toolsBaseCodeInternal(generator, idx - 1U)}
    };
    
    return util::processTemplate(Templ, repl);
}

} // namespace 
    

std::string ToolsQtDefaultOptions::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    return 
        util::strReplace(toolsScope(generator), "::", "/") + 
        strings::cppHeaderSuffixStr();
}

std::string ToolsQtDefaultOptions::toolsTemplParam(const ToolsQtGenerator& generator, const std::string& extraParams)
{
    return '<' + toolsScope(generator) + extraParams + '>';
}

std::string ToolsQtDefaultOptions::toolsScope(const ToolsQtGenerator& generator)
{
    return 
        generator.getTopNamespace() + "::" + 
        generator.protocolSchema().mainNamespace() + "::" + 
        comms::scopeForOptions(strings::defaultOptionsClassStr(), generator, false);
}    

bool ToolsQtDefaultOptions::write(ToolsQtGenerator& generator)
{
    ToolsQtDefaultOptions obj(generator);
    return obj.toolsWriteInternal();
}

bool ToolsQtDefaultOptions::toolsWriteInternal() const
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
        "/// @brief Contains definition of the default options for plugin.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#EXTRA_INCLUDES#$#\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"        
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "using #^#NAME#$#Base =\n"
        "    #^#OPTS_BASE#$#;\n\n"
        "struct #^#NAME#$##^#ORIG#$# : public #^#NAME#$#Base\n"
        "{\n"
        "    template<typename TInterface, typename TAllMessages, typename... TOptions>\n"
        "    using MsgFactory = #^#MSG_FACTORY#$##^#INTERFACE_TEMPL#$#;\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n";

    auto codePrefix = m_generator.getCodeDir() + '/' + toolsRelHeaderPath(m_generator);

    util::StringsList includes = {
        ToolsQtMsgFactory::toolsRelHeaderPath(m_generator)
    };

    auto& schemas = m_generator.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        auto& gen = const_cast<ToolsQtGenerator&>(m_generator);
        gen.chooseCurrentSchema(idx);
        includes.push_back(comms::relHeaderForOptions(strings::defaultOptionsClassStr(), gen));
    }
    assert(m_generator.isCurrentProtocolSchema());

    comms::prepareIncludeStatement(includes);

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "")},
        {"EXTRA_INCLUDES", util::readFileContents(codePrefix + strings::incFileSuffixStr())},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"PROT_NAMESPACE", m_generator.protocolSchema().mainNamespace()},
        {"NAME", strings::defaultOptionsClassStr()},
        {"DEF_OPTIONS", comms::scopeForOptions(strings::defaultOptionsClassStr(), m_generator)},
        {"EXTEND", util::readFileContents(codePrefix + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(codePrefix + strings::appendFileSuffixStr())},
        {"MSG_FACTORY", ToolsQtMsgFactory::toolsClassScope(m_generator)},
        {"OPTS_BASE", toolsBaseCodeInternal(m_generator, m_generator.schemas().size() - 1U)},
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

} // namespace commsdsl2tools_qt