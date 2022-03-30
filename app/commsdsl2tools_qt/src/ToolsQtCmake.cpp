//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtCmake.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <fstream>
#include <cassert>

namespace commsdsl2tools_qt
{

namespace 
{

using ReplacementMap = commsdsl::gen::util::ReplacementMap;

} // namespace 
    

bool ToolsQtCmake::write(ToolsQtGenerator& generator)
{
    ToolsQtCmake obj(generator);
    return obj.writeInternal();
}

bool ToolsQtCmake::writeInternal() const
{
    static_cast<void>(m_generator);
    auto filePath = 
        commsdsl::gen::util::pathAddElem(
            m_generator.getOutputDir(), commsdsl::gen::strings::cmakeListsFileStr());    

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    std::string interfaceScope;
    auto allInterfaces = m_generator.getAllInterfaces();
    if (!allInterfaces.empty()) {
        auto* firstInterface = allInterfaces.front();
        assert(!firstInterface->name().empty());
        interfaceScope = commsdsl::gen::comms::scopeFor(*firstInterface, m_generator);
    }
    else {
        interfaceScope = commsdsl::gen::comms::scopeForInterface(commsdsl::gen::strings::messageClassStr(), m_generator);
    }

    auto allFrames = m_generator.getAllFrames();
    assert(!allFrames.empty());
    auto* firstFrame = allFrames.front();
    assert(!firstFrame->name().empty());


    ReplacementMap repl = {
        std::make_pair("PROJ_NAME", m_generator.schemaName()),
        std::make_pair("PROJ_NS", m_generator.mainNamespace()),
        std::make_pair("INTERFACE_SCOPE", std::move(interfaceScope)),
        std::make_pair("FRAME_SCOPE", commsdsl::gen::comms::scopeFor(*firstFrame, m_generator)),
        std::make_pair("OPTIONS_SCOPE", commsdsl::gen::comms::scopeForOptions(commsdsl::gen::strings::defaultOptionsStr(), m_generator)),
        std::make_pair("INPUT_SCOPE", commsdsl::gen::comms::scopeForInput(commsdsl::gen::strings::allMessagesStr(), m_generator)),
    };

    static const std::string Template =
        "cmake_minimum_required (VERSION 3.1)\n"
        "project (\"#^#PROJ_NAME#$#_toolsqt\")\n\n"
        "TODO";

    auto str = commsdsl::gen::util::processTemplate(Template, repl);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2tools_qt