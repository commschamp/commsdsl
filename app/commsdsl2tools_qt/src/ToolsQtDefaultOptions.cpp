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

std::string ToolsQtDefaultOptions::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    return 
        util::strReplace(toolsScope(generator), "::", "/") + 
        strings::cppHeaderSuffixStr();
}

std::string ToolsQtDefaultOptions::toolsTemplParam(const ToolsQtGenerator& generator)
{
    return '<' + toolsScope(generator) + '>';
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
    return obj.testWriteInternal();
}

bool ToolsQtDefaultOptions::testWriteInternal() const
{
    auto filePath = m_generator.getOutputDir() + '/' + toolsRelHeaderPath(m_generator);

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
        "/// @file\n"
        "/// @brief Contains definition of the default options for plugin.\n\n"
        "#pragma once\n\n"
        "#include \"#^#DEF_OPTIONS_INC#$#\"\n"
        "#^#INCLUDES#$#\n\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"        
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "template <typename TBase = ::#^#DEF_OPTIONS#$#>\n"
        "struct #^#NAME#$##^#ORIG#$#T : public TBase {};\n\n"
        "using  #^#NAME#$##^#ORIG#$# =  #^#NAME#$##^#ORIG#$#T<>;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n";

    auto codePrefix = m_generator.getCodeDir() + '/' + toolsRelHeaderPath(m_generator);

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"DEF_OPTIONS_INC", comms::relHeaderForOptions(strings::defaultOptionsClassStr(), m_generator)},
        {"INCLUDES", util::readFileContents(codePrefix + strings::incFileSuffixStr())},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"PROT_NAMESPACE", m_generator.protocolSchema().mainNamespace()},
        {"NAME", strings::defaultOptionsClassStr()},
        {"DEF_OPTIONS", comms::scopeForOptions(strings::defaultOptionsClassStr(), m_generator)},
        {"EXTEND", util::readFileContents(codePrefix + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(codePrefix + strings::appendFileSuffixStr())},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

} // namespace commsdsl2tools_qt