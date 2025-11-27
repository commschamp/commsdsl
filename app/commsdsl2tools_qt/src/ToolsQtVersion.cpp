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
    return util::genScopeToRelPath(scope) + strings::genCppHeaderSuffixStr();
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
        "#^#INCLUDES#$#\n"
        "#define CC_TOOLS_QT_PLUGIN_#^#NS#$#_SPEC_VERSION (#^#VERSION#$#)\n\n"
        "static_assert(CC_TOOLS_QT_PLUGIN_#^#NS#$#_SPEC_VERSION == #^#NS#$#_SPEC_VERSION,\n"
        "    \"The spec versions don't match\");\n\n"
        "#^#CODE_VERSION#$#\n"
        "static_assert(CC_TOOLS_QT_MAKE_VERSION(#^#TOOLS_QT_MIN#$#) <= cc_tools_qt::version(),\n"
        "    \"The version of cc_tools_qt library is too old\");\n\n"
        "#^#APPEND#$#\n";

    util::GenStringsList includes = {
        "cc_tools_qt/version.h",
        comms::genRelHeaderForRoot(strings::genVersionFileNameStr(), m_toolsGenerator),
    };

    comms::genPrepareIncludeStatement(includes);

    util::GenReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"TOOLS_QT_MIN", util::genStrReplace(ToolsQtGenerator::toolsMinCcToolsQtVersion(), ".", ", ")},
        {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
        {"NS", util::genStrToUpper(m_toolsGenerator.genProtocolSchema().genMainNamespace())},
        {"VERSION", util::genNumToString(m_toolsGenerator.genProtocolSchema().genSchemaVersion())},
        {"CODE_VERSION", toolsCodeVersionInternal()},
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

std::string ToolsQtVersion::toolsCodeVersionInternal() const
{
    auto tokens = m_toolsGenerator.genGetCodeVersionTokens();
    if (tokens.empty()) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "#define CC_TOOLS_QT_PLUGIN_#^#NS#$#_MAJOR_VERSION (#^#MAJOR_VERSION#$#)\n\n"
        "#define CC_TOOLS_QT_PLUGIN_#^#NS#$#_MINOR_VERSION (#^#MINOR_VERSION#$#)\n\n"
        "#define CC_TOOLS_QT_PLUGIN_#^#NS#$#_PATCH_VERSION (#^#PATCH_VERSION#$#)\n\n"
        "#define CC_TOOLS_QT_PLUGIN_#^#NS#$#_VERSION (COMMS_MAKE_VERSION(#^#NS#$#_MAJOR_VERSION, #^#NS#$#_MINOR_VERSION, #^#NS#$#_PATCH_VERSION))\n\n"
        "static_assert(CC_TOOLS_QT_PLUGIN_#^#NS#$#_VERSION == #^#NS#$#_VERSION, \"Versions mismatch\");\n"
        ;

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_toolsGenerator.genProtocolSchema().genMainNamespace())},
        {"MAJOR_VERSION", tokens[ToolsQtGenerator::GenVersionIdx_Major]},
        {"MINOR_VERSION", tokens[ToolsQtGenerator::GenVersionIdx_Minor]},
        {"PATCH_VERSION", tokens[ToolsQtGenerator::GenVersionIdx_Patch]},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt