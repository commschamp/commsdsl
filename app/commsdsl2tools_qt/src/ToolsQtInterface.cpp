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

#include "ToolsQtInterface.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtVersion.h"

#include "commsdsl/gen/GenEnumField.h"
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

unsigned toolsGetHexMsgIdWidthInternal(const commsdsl::gen::GenInterface& interface)
{
    auto* parentNs = interface.genParentNamespace();
    auto allMsgIdFields = parentNs->genFindMessageIdFields();
    if (allMsgIdFields.empty()) {
        return 0U;
    }

    auto* msgIdField = allMsgIdFields.front();
    if (msgIdField->genParseObj().parseKind() != commsdsl::parse::ParseField::ParseKind::Enum) {
        return 0U;
    }

    auto* enumMsgIdField = static_cast<const commsdsl::gen::GenEnumField*>(msgIdField);
    return enumMsgIdField->genHexWidth();
}

} // namespace

ToolsQtInterface::ToolsQtInterface(ToolsQtGenerator& generator, ParseInterface parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

std::string ToolsQtInterface::toolsClassScope() const
{
    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
    return gen.toolsScopePrefixForInterface(*this) + comms::genScopeFor(*this, gen);
}

std::string ToolsQtInterface::toolsHeaderFilePath() const
{
    return toolsRelFilePath() + strings::genCppHeaderSuffixStr();
}

ToolsQtInterface::GenStringsList ToolsQtInterface::toolsSourceFiles() const
{
    return GenStringsList{toolsRelFilePath() + strings::genCppSourceSuffixStr()};
}

bool ToolsQtInterface::genWriteImpl() const
{
    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
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
    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
    auto filePath = gen.genGetOutputDir() + '/' + toolsHeaderFilePath();

    auto& logger = gen.genLogger();
    logger.genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#TOP_NS_BEGIN#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
        "#^#TOP_NS_END#$#\n"
    ;

    util::GenStringsList includes {
        "cc_tools_qt/ToolsMessage.h",
        "cc_tools_qt/ToolsProtMsgInterface.h",
        comms::genRelHeaderPathFor(*this, gen),
        ToolsQtVersion::toolsRelHeaderPath(gen),
        ToolsQtDefaultOptions::toolsRelHeaderPath(gen)
    };

    comms::genPrepareIncludeStatement(includes);

    util::GenReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*this)},
        {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*this)},
        {"DEF", toolsHeaderCodeInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool ToolsQtInterface::toolsWriteSrcInternal() const
{
    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
    auto filePath = gen.genGetOutputDir() + '/' + toolsRelFilePath() + strings::genCppSourceSuffixStr();

    auto& logger = gen.genLogger();
    logger.genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#CLASS_NAME#$#.h\"\n\n"
        "#^#TOP_NS_BEGIN#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
        "#^#TOP_NS_END#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"CLASS_NAME", comms::genClassName(toolsNameInternal())},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*this)},
        {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*this)},
        {"DEF", toolsSrcCodeInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
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

    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(toolsNameInternal())},
        {"INTERFACE", comms::genScopeFor(*this, gen)},
        {"OPTIONS", ToolsQtDefaultOptions::toolsClassScope(gen)},
    };

    auto hexWidth = toolsGetHexMsgIdWidthInternal(*this);
    if (0U < hexWidth) {
        repl["ID_FUNC"] = "virtual QString idAsStringImpl() const override;";
        repl["PROTECTED"] = "protected:";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string ToolsQtInterface::toolsSrcCodeInternal() const
{
    static const std::string Templ =
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() = default;\n\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() noexcept = default;\n\n"
        "#^#ID_FUNC#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(toolsNameInternal())},
    };

    auto hexWidth = toolsGetHexMsgIdWidthInternal(*this);
    if (0U < hexWidth) {
        auto func =
            "QString " + comms::genClassName(toolsNameInternal()) + "::idAsStringImpl() const\n"
            "{\n"
            "    return QString(\"0x%1\").arg(numericIdImpl(), " + std::to_string(hexWidth) + ", 16, QChar('0')).toUpper();\n"
            "}\n";
        repl["ID_FUNC"] = std::move(func);
    }

    return util::genProcessTemplate(Templ, repl);
}

const std::string& ToolsQtInterface::toolsNameInternal() const
{
    if (!genParseObj().parseValid()) {
        return strings::genMessageClassStr();
    }

    return genParseObj().parseName();
}

std::string ToolsQtInterface::toolsRelFilePath() const
{
    return util::genScopeToRelPath(toolsClassScope());
}

} // namespace commsdsl2tools_qt
