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

#include "ToolsQtDefaultOptions.h"

#include "ToolsQtGenerator.h"
#include "ToolsQtVersion.h"

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

std::string toolsBaseCodeInternal(const ToolsQtGenerator& generator, std::size_t idx, bool wrapWithFactoryOpts = true)
{
    assert(idx < generator.genSchemas().size());

    auto oldIdx = generator.genCurrentSchemaIdx();
    generator.genChooseCurrentSchema(static_cast<unsigned>(idx));
    if (!generator.genCurrentSchema().genHasAnyReferencedComponent()) {
        if (idx == 0U) {
            generator.genChooseCurrentSchema(oldIdx);
            return strings::genEmptyString();
        }

        auto result = toolsBaseCodeInternal(generator, idx - 1U);
        generator.genChooseCurrentSchema(oldIdx);
        return result;
    }

    if (wrapWithFactoryOpts && generator.genCurrentSchema().genHasAnyReferencedMessage()) {
        static const std::string Templ =
            "::#^#SCOPE#$#T<\n"
            "    #^#NEXT#$#\n"
            ">";

        util::GenReplacementMap repl = {
            {"SCOPE", comms::genScopeForOptions(strings::genAllMessagesDynMemMsgFactoryDefaultOptionsClassStr(), generator)},
            {"NEXT", toolsBaseCodeInternal(generator, idx, false)}
        };

        generator.genChooseCurrentSchema(oldIdx);
        return util::genProcessTemplate(Templ, repl);
    }

    auto scope = comms::genScopeForOptions(strings::genDefaultOptionsClassStr(), generator);
    generator.genChooseCurrentSchema(oldIdx);

    if (idx == 0U) {
        return "::" + scope;
    }

    auto nextScope = toolsBaseCodeInternal(generator, idx - 1U);
    if (nextScope.empty()) {
        return "::" + scope;
    }

    static const std::string Templ =
        "::#^#SCOPE#$#T<\n"
        "    #^#NEXT#$#\n"
        ">";

    util::GenReplacementMap repl = {
        {"SCOPE", std::move(scope)},
        {"NEXT", std::move(nextScope)}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace

std::string ToolsQtDefaultOptions::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    return
        util::genScopeToRelPath(toolsClassScope(generator)) +
        strings::genCppHeaderSuffixStr();
}

std::string ToolsQtDefaultOptions::toolsTemplParam(const ToolsQtGenerator& generator, const std::string& extraParams)
{
    return '<' + toolsClassScope(generator) + extraParams + '>';
}

std::string ToolsQtDefaultOptions::toolsClassScope(const ToolsQtGenerator& generator)
{
    return
        generator.toolsScopePrefix() +
        generator.genProtocolSchema().genMainNamespace() + "::" +
        comms::genScopeForOptions(strings::genDefaultOptionsClassStr(), generator, false);
}

bool ToolsQtDefaultOptions::toolsWrite(ToolsQtGenerator& generator)
{
    ToolsQtDefaultOptions obj(generator);
    return obj.toolsWriteInternal();
}

bool ToolsQtDefaultOptions::toolsWriteInternal() const
{
    auto filePath = m_toolsGenerator.genGetOutputDir() + '/' + toolsRelHeaderPath(m_toolsGenerator);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_toolsGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    m_toolsGenerator.genLogger().genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        m_toolsGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of the default options for plugin.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#EXTRA_INCLUDES#$#\n\n"
        "#^#TOP_NS_BEGIN#$#\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "using #^#NAME#$# =\n"
        "    #^#OPTS_BASE#$#;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "#^#TOP_NS_END#$#\n"
    ;

    auto codePrefix = m_toolsGenerator.genGetCodeDir() + '/' + toolsRelHeaderPath(m_toolsGenerator);

    util::GenStringsList includes {
        ToolsQtVersion::toolsRelHeaderPath(m_toolsGenerator)
    };

    auto& schemas = m_toolsGenerator.genSchemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        m_toolsGenerator.genChooseCurrentSchema(idx);
        if (!m_toolsGenerator.genCurrentSchema().genHasAnyReferencedComponent()) {
            continue;
        }

        if (m_toolsGenerator.genCurrentSchema().genHasAnyReferencedMessage()) {
            includes.push_back(comms::genRelHeaderForOptions(strings::genAllMessagesDynMemMsgFactoryDefaultOptionsClassStr(), m_toolsGenerator));
        }

        includes.push_back(comms::genRelHeaderForOptions(strings::genDefaultOptionsClassStr(), m_toolsGenerator));
    }
    assert(m_toolsGenerator.genIsCurrentProtocolSchema());

    comms::genPrepareIncludeStatement(includes);

    util::GenReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"INCLUDES", util::genStrListToString(includes, "\n", "")},
        {"EXTRA_INCLUDES", util::genReadFileContents(codePrefix + strings::genIncFileSuffixStr())},
        {"TOP_NS_BEGIN", m_toolsGenerator.toolsNamespaceBegin()},
        {"TOP_NS_END", m_toolsGenerator.toolsNamespaceEnd()},
        {"PROT_NAMESPACE", m_toolsGenerator.genProtocolSchema().genMainNamespace()},
        {"NAME", strings::genDefaultOptionsClassStr()},
        {"EXTEND", util::genReadFileContents(codePrefix + strings::genExtendFileSuffixStr())},
        {"APPEND", util::genReadFileContents(codePrefix + strings::genAppendFileSuffixStr())},
        {"OPTS_BASE", toolsBaseCodeInternal(m_toolsGenerator, m_toolsGenerator.genSchemas().size() - 1U)},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::genOrigSuffixStr();
    }

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

} // namespace commsdsl2tools_qt