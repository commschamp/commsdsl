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
    assert(idx < generator.schemas().size());

    auto oldIdx = generator.currentSchemaIdx();
    generator.chooseCurrentSchema(static_cast<unsigned>(idx));
    if (!generator.currentSchema().hasAnyReferencedComponent()) {
        if (idx == 0U) {
            generator.chooseCurrentSchema(oldIdx);
            return strings::emptyString();
        }

        auto result = toolsBaseCodeInternal(generator, idx - 1U);
        generator.chooseCurrentSchema(oldIdx);
        return result;
    }

    if (wrapWithFactoryOpts && generator.currentSchema().hasAnyReferencedMessage()) {
        static const std::string Templ = 
            "::#^#SCOPE#$#T<\n"
            "    #^#NEXT#$#\n"
            ">";

        util::ReplacementMap repl = {
            {"SCOPE", comms::scopeForOptions(strings::allMessagesDynMemMsgFactoryDefaultOptionsClassStr(), generator)},
            {"NEXT", toolsBaseCodeInternal(generator, idx, false)}
        };        

        generator.chooseCurrentSchema(oldIdx);
        return util::processTemplate(Templ, repl);
    }

    auto scope = comms::scopeForOptions(strings::defaultOptionsClassStr(), generator);
    generator.chooseCurrentSchema(oldIdx);

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

    util::ReplacementMap repl = {
        {"SCOPE", std::move(scope)},
        {"NEXT", std::move(nextScope)}
    };
    
    return util::processTemplate(Templ, repl);
}

} // namespace 
    

std::string ToolsQtDefaultOptions::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    return 
        util::strReplace(toolsClassScope(generator), "::", "/") + 
        strings::cppHeaderSuffixStr();
}

std::string ToolsQtDefaultOptions::toolsTemplParam(const ToolsQtGenerator& generator, const std::string& extraParams)
{
    return '<' + toolsClassScope(generator) + extraParams + '>';
}

std::string ToolsQtDefaultOptions::toolsClassScope(const ToolsQtGenerator& generator)
{
    return 
        generator.toolsScopePrefix() + 
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

    auto codePrefix = m_generator.getCodeDir() + '/' + toolsRelHeaderPath(m_generator);

    util::StringsList includes {
        ToolsQtVersion::toolsRelHeaderPath(m_generator)
    };

    auto& schemas = m_generator.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        m_generator.chooseCurrentSchema(idx);
        if (!m_generator.currentSchema().hasAnyReferencedComponent()) {
            continue;
        }       

        if (m_generator.currentSchema().hasAnyReferencedMessage()) {
            includes.push_back(comms::relHeaderForOptions(strings::allMessagesDynMemMsgFactoryDefaultOptionsClassStr(), m_generator));    
        }
        
        includes.push_back(comms::relHeaderForOptions(strings::defaultOptionsClassStr(), m_generator));
    }
    assert(m_generator.isCurrentProtocolSchema());

    comms::prepareIncludeStatement(includes);

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "")},
        {"EXTRA_INCLUDES", util::readFileContents(codePrefix + strings::incFileSuffixStr())},
        {"TOP_NS_BEGIN", m_generator.toolsNamespaceBegin()},
        {"TOP_NS_END", m_generator.toolsNamespaceEnd()},
        {"PROT_NAMESPACE", m_generator.protocolSchema().mainNamespace()},
        {"NAME", strings::defaultOptionsClassStr()},
        {"EXTEND", util::readFileContents(codePrefix + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(codePrefix + strings::appendFileSuffixStr())},
        {"OPTS_BASE", toolsBaseCodeInternal(m_generator, m_generator.schemas().size() - 1U)},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

} // namespace commsdsl2tools_qt