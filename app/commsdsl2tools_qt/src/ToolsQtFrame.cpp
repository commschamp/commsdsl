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

#include "ToolsQtFrame.h"

#include "ToolsQtGenerator.h"
#include "ToolsQtInterface.h"

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

ToolsQtFrame::ToolsQtFrame(ToolsQtGenerator& generator, commsdsl::parse::Frame dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

bool ToolsQtFrame::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    return true;
}

bool ToolsQtFrame::writeImpl()
{
    return toolsWriteHeaderInternal();
}

bool ToolsQtFrame::toolsWriteHeaderInternal()
{
    auto& gen = generator();
    auto filePath = gen.getOutputDir() + '/' + toolsHeaderFilePathInternal();

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "\n"
        "#pragma once\n\n"
        "#include \"#^#FRAME_INCLUDE#$#\"\n"
        "#include \"input/AllMessages.h\"\n"
        "#^#INTERFACE_INCLUDE#$#\n"
        "\n"
        "#^#NS_BEGIN#$#\n"
        "#^#INTERFACE_TEMPL_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    #^#FRAME_SCOPE#$#<\n"
        "        #^#INTERFACE#$#,\n"
        "        #^#TOP_NS#$#::#^#ALL_MESSAGES#$##^#INTERFACE_TEMPL#$#\n"
        "    >;\n\n"
        "#^#NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"FRAME_INCLUDE", comms::relHeaderPathFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"FRAME_SCOPE", comms::scopeFor(*this, gen)},
        {"TOP_NS", gen.getTopNamespace()},
        {"ALL_MESSAGES", comms::scopeForInput(strings::allMessagesStr(), gen)},
    };

    auto allInterfaces = gen.getAllInterfaces();
    assert(!allInterfaces.empty());
    if (1U < allInterfaces.size()) {
        repl["INTERFACE_TEMPL_PARAM"] = "template <typename TInterface>";
        repl["INTERFACE"] = "TInterface";
        repl["INTERFACE_TEMPL"] = "<TIterface>";
    }
    else {
        auto* defaultInterface = static_cast<const ToolsQtInterface*>(allInterfaces.front());
        assert(defaultInterface != nullptr);
        repl["INTERFACE_INCLUDE"] = "#include \"" + defaultInterface->toolsHeaderFilePath() + '\"';
        repl["INTERFACE"] = gen.getTopNamespace() + "::" + comms::scopeFor(*defaultInterface, gen);
    }
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

std::string ToolsQtFrame::toolsHeaderFilePathInternal() const
{
    auto scope = comms::scopeFor(*this, generator(), false);
    return util::strReplace(scope, "::", "/") + strings::cppHeaderSuffixStr();
}

} // namespace commsdsl2tools_qt
