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

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

bool ToolsQtVersion::toolsWrite(ToolsQtGenerator& generator)
{
    ToolsQtVersion obj(generator);
    return obj.toolsWriteInternal();
}

std::string ToolsQtVersion::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    auto scope = generator.toolsScopePrefix() + comms::genScopeForRoot(strings::genVersionFileNameStr(), generator);
    return util::genStrReplace(scope, "::", "/") + strings::genCppHeaderSuffixStr();
}

bool ToolsQtVersion::toolsWriteInternal() const
{
    auto filePath = m_toolsGenerator.genGetOutputDir() + '/' + toolsRelHeaderPath(m_toolsGenerator);

    m_toolsGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_toolsGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#include \"cc_tools_qt/version.h\"\n\n"
        "static_assert(CC_TOOLS_QT_MAKE_VERSION(#^#TOOLS_QT_MIN#$#) <= cc_tools_qt::version(),\n"
        "    \"The version of cc_tools_qt library is too old\");\n\n"
        "#^#APPEND#$#\n";


    util::GenReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"TOOLS_QT_MIN", util::genStrReplace(ToolsQtGenerator::toolsMinCcToolsQtVersion(), ".", ", ")},
        {"APPEND", util::genReadFileContents(m_toolsGenerator.genGetCodeDir() + '/' + toolsRelHeaderPath(m_toolsGenerator) + strings::genAppendFileSuffixStr())},
    };        
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_toolsGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2tools_qt