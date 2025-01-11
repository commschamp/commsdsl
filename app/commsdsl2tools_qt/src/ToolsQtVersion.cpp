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

#include "ToolsQtVersion.h"

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

bool ToolsQtVersion::write(ToolsQtGenerator& generator)
{
    ToolsQtVersion obj(generator);
    return obj.writeInternal();
}

std::string ToolsQtVersion::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    auto scope = generator.toolsScopePrefix() + comms::scopeForRoot(strings::versionFileNameStr(), generator);
    return util::strReplace(scope, "::", "/") + strings::cppHeaderSuffixStr();
}

bool ToolsQtVersion::writeInternal() const
{
    auto filePath = m_generator.getOutputDir() + '/' + toolsRelHeaderPath(m_generator);

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#include \"cc_tools_qt/version.h\"\n\n"
        "static_assert(CC_TOOLS_QT_MAKE_VERSION(#^#TOOLS_QT_MIN#$#) <= cc_tools_qt::version(),\n"
        "    \"The version of cc_tools_qt library is too old\");\n\n"
        "#^#APPEND#$#\n";


    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"TOOLS_QT_MIN", util::strReplace(ToolsQtGenerator::toolsMinCcToolsQtVersion(), ".", ", ")},
        {"APPEND", util::readFileContents(m_generator.getCodeDir() + '/' + toolsRelHeaderPath(m_generator) + strings::appendFileSuffixStr())},
    };        
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2tools_qt