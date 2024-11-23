//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtInterface.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtVersion.h"

#include "commsdsl/gen/EnumField.h"
#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

unsigned getHexMsgIdWidthInternal(const commsdsl::gen::Generator& generator)
{
    auto* msgIdField = generator.currentSchema().getMessageIdField();
    if (msgIdField == nullptr) {
        return 0U;
    }

    if (msgIdField->dslObj().kind() != commsdsl::parse::Field::Kind::Enum) {
        return 0U;
    }

    auto* enumMsgIdField = static_cast<const commsdsl::gen::EnumField*>(msgIdField);
    return enumMsgIdField->hexWidth();
}

} // namespace 
    

ToolsQtInterface::ToolsQtInterface(ToolsQtGenerator& generator, commsdsl::parse::Interface dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

std::string ToolsQtInterface::toolsScope() const
{
    return generator().getTopNamespace() + "::" + comms::scopeFor(*this, generator());
}

std::string ToolsQtInterface::toolsHeaderFilePath() const
{
    return toolsRelFilePath() + strings::cppHeaderSuffixStr();
}

ToolsQtInterface::StringsList ToolsQtInterface::toolsSourceFiles() const
{
    return StringsList{toolsRelFilePath() + strings::cppSourceSuffixStr()};
}

bool ToolsQtInterface::writeImpl() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto interfaces = gen.toolsGetSelectedInterfaces();
    auto iter = std::find(interfaces.begin(), interfaces.end(), this);
    if (iter == interfaces.end()) {
        // Interface not used.
        return true;
    }

    return toolsWriteHeaderInternal() && toolsWriteSrcInternal();
}

bool ToolsQtInterface::toolsWriteHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto filePath = gen.getOutputDir() + '/' + toolsHeaderFilePath();

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
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
    ;

    util::StringsList includes {
        "cc_tools_qt/ToolsMessage.h",
        "cc_tools_qt/ToolsProtMsgInterface.h",
        comms::relHeaderPathFor(*this, gen),
        ToolsQtVersion::toolsRelHeaderPath(gen),
        ToolsQtDefaultOptions::toolsRelHeaderPath(gen)
    };

    comms::prepareIncludeStatement(includes);    

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"DEF", toolsHeaderCodeInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool ToolsQtInterface::toolsWriteSrcInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto filePath = gen.getOutputDir() + '/' + toolsRelFilePath() + strings::cppSourceSuffixStr();

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
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"CLASS_NAME", comms::className(toolsNameInternal())},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"DEF", toolsSrcCodeInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string ToolsQtInterface::toolsHeaderCodeInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public cc_tools_qt::ToolsMessage\n"
        "{\n"
        "public:\n"
        "    using ProtInterface = cc_tools_qt::ToolsProtMsgInterface<::#^#INTERFACE#$#>;\n"
        "    using ProtOptions = #^#OPTIONS#$#;\n"
        "    #^#CLASS_NAME#$#();\n"
        "    virtual ~#^#CLASS_NAME#$#() noexcept;\n\n"
        "#^#PROTECTED#$#\n"
        "    #^#ID_FUNC#$#\n"
        "};\n";

    auto& gen = ToolsQtGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(toolsNameInternal())},
        {"INTERFACE", comms::scopeFor(*this, gen)},
        {"OPTIONS", ToolsQtDefaultOptions::toolsScope(gen)},
    };

    auto hexWidth = getHexMsgIdWidthInternal(generator());
    if (0U < hexWidth) {
        repl["ID_FUNC"] = "virtual QString idAsStringImpl() const override;";
        repl["PROTECTED"] = "protected:";
    }

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtInterface::toolsSrcCodeInternal() const
{
    static const std::string Templ = 
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() = default;\n\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() noexcept = default;\n\n"
        "#^#ID_FUNC#$#\n"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(toolsNameInternal())},
    };


    auto hexWidth = getHexMsgIdWidthInternal(generator());
    if (0U < hexWidth) {
        auto func =
            "QString " + comms::className(toolsNameInternal()) + "::idAsStringImpl() const\n"
            "{\n"
            "    return QString(\"0x%1\").arg(numericIdImpl(), " + std::to_string(hexWidth) + ", 16, QChar('0')).toUpper();\n"
            "}\n";
        repl["ID_FUNC"] = std::move(func);
    }    

    return util::processTemplate(Templ, repl);         
}

const std::string& ToolsQtInterface::toolsNameInternal() const
{
    if (!dslObj().valid()) {
        return strings::messageClassStr();
    }

    return dslObj().name();
}

std::string ToolsQtInterface::toolsRelFilePath() const
{
    auto scope = comms::scopeFor(*this, generator());
    auto iFaceScope = comms::scopeFor(*this, generator(), false, true);
    return generator().getTopNamespace() + '/' + util::strReplace(iFaceScope, "::", "/") + '/' + util::strReplace(scope, "::", "/");
}

} // namespace commsdsl2tools_qt
