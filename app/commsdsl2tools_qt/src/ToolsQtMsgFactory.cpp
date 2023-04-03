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

#include "ToolsQtMsgFactory.h"

#include "ToolsQtGenerator.h"
#include "ToolsQtInputMessages.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

const std::string MsgFactoryName = "AllMessagesDynMemMsgFactory";

} // namespace 
    

bool ToolsQtMsgFactory::write(ToolsQtGenerator& generator)
{
    ToolsQtMsgFactory obj(generator);
    return obj.toolsWriteInternal();
}

bool ToolsQtMsgFactory::toolsWriteInternal() const
{
    auto& thisSchema = m_generator.currentSchema();
    if ((!m_generator.isCurrentProtocolSchema()) && (!thisSchema.hasAnyReferencedMessage())) {
        return true;
    }

    return
        toolsWriteHeaderInternal();
}

bool ToolsQtMsgFactory::toolsWriteHeaderInternal() const
{
    auto filePath = 
        m_generator.getOutputDir() + '/' + 
        m_generator.getTopNamespace() + '/' + 
        util::strReplace(comms::scopeForFactory(MsgFactoryName, m_generator), "::", "/") + 
        strings::cppHeaderSuffixStr();

    auto& logger = m_generator.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }        

    util::StringsList includes = {
        "<memory>",
        "comms/MsgFactoryCreateFailureReason.h",
        ToolsQtInputMessages::toolsRelHeaderPath(m_generator)
    };

    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"        
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace factory\n"
        "{\n\n"
        "#^#DEF#$#\n\n"
        "} // namespace factory\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n";
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"PROT_NAMESPACE", m_generator.protocolSchema().mainNamespace()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"DEF", toolsHeaderCodeInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string ToolsQtMsgFactory::toolsHeaderCodeInternal() const
{
    // TODO
    return std::string();
}


} // namespace commsdsl2tools_qt