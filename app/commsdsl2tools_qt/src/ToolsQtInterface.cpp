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

const std::string& toolsAliasTemplateInternal()
{
    static const std::string Templ = 
        "using #^#CLASS_NAME#$# =\n"
        "    cc_tools_qt::MessageBase<\n"
        "        ::#^#INTERFACE#$#\n"
        "    >;\n";
    return Templ;
}

const std::string& toolsClassTemplateInternal()
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public\n"
        "    cc_tools_qt::MessageBase<\n"
        "        ::#^#INTERFACE#$#\n"
        "    >\n"
        "{\n"
        "protected:\n"
        "    #^#ID_FUNC#$#\n"
        "    virtual const QVariantList& extraTransportFieldsPropertiesImpl() const override;\n"
        "};\n";
    return Templ;
}

} // namespace 
    

ToolsQtInterface::ToolsQtInterface(ToolsQtGenerator& generator, commsdsl::parse::Interface dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

std::string ToolsQtInterface::toolsHeaderFilePath() const
{
    return toolsRelFilePath() + strings::cppHeaderSuffixStr();
}

ToolsQtInterface::StringsList ToolsQtInterface::toolsSourceFiles() const
{
    return StringsList{toolsRelFilePath() + strings::cppSourceSuffixStr()};
}

bool ToolsQtInterface::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_toolsFields = ToolsQtField::toolsTransformFieldsList(fields());
    return true;
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
        "cc_tools_qt/MessageBase.h",
        comms::relHeaderPathFor(*this, gen),
        ToolsQtVersion::toolsRelHeaderPath(gen),
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
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
    ;

    util::StringsList includes;

    for (auto* f : m_toolsFields) {
        auto incs = f->toolsSrcIncludes();
        includes.reserve(includes.size() + incs.size());
        std::move(incs.begin(), incs.end(), std::back_inserter(includes));
    }
    comms::prepareIncludeStatement(includes);

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"CLASS_NAME", comms::className(toolsNameInternal())},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"DEF", toolsSrcCodeInternal()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")}
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string ToolsQtInterface::toolsHeaderCodeInternal() const
{
    auto hexWidth = getHexMsgIdWidthInternal(generator());
    auto* templ = &toolsAliasTemplateInternal();
    if ((!m_toolsFields.empty()) || (0U < hexWidth)) {
        templ = &toolsClassTemplateInternal();
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(toolsNameInternal())},
        {"INTERFACE", comms::scopeFor(*this, generator())}
    };

    if (0U < hexWidth) {
        repl["ID_FUNC"] = "virtual QString idAsStringImpl() const override;";
    }

    return util::processTemplate(*templ, repl);
}

std::string ToolsQtInterface::toolsSrcCodeInternal() const
{
    auto hexWidth = getHexMsgIdWidthInternal(generator());
    if ((m_toolsFields.empty()) && (hexWidth == 0U)) {
        return strings::emptyString();
    }    

    static const std::string Templ = 
        "namespace\n"
        "{\n\n"    
        "#^#FIELDS_PROPS#$#\n"
        "QVariantList createProps()\n"
        "{\n"
        "    QVariantList props;\n"
        "    #^#PROPS_APPENDS#$#\n"
        "    return props;\n"
        "}\n\n"
        "} // namespace \n\n"
        "#^#ID_FUNC#$#\n"
        "const QVariantList& #^#CLASS_NAME#$#::extraTransportFieldsPropertiesImpl() const\n"
        "{\n"
        "    static const QVariantList Props = createProps();\n"
        "    return Props;\n"
        "}\n";   

    util::StringsList fieldsProps;
    util::StringsList appends;
    fieldsProps.reserve(m_toolsFields.size());
    appends.reserve(m_toolsFields.size());
    for (auto* f : m_toolsFields) {
        auto membersStr = f->toolsDefMembers();
        if (!membersStr.empty()) {
            fieldsProps.push_back(std::move(membersStr));
        }

        fieldsProps.push_back(f->toolsDefFunc());
        appends.push_back("props.append(createProps_" + comms::accessName(f->field().dslObj().name()) + "(true));");
    }   

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(toolsNameInternal())},
        {"FIELDS_PROPS", util::strListToString(fieldsProps, "\n", "")},
        {"PROPS_APPENDS", util::strListToString(appends, "\n", "")}
    };

    if (0U < hexWidth) {
        auto func =
            "QString " + comms::className(toolsNameInternal()) + "::idAsStringImpl() const\n"
            "{\n"
            "    return \"0x\" + QString(\"%1\").arg(static_cast<unsigned long long>(getId()), " +
            std::to_string(hexWidth) + ", 16, QChar('0')).toUpper();\n"
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
    return generator().getTopNamespace() + '/' + util::strReplace(scope, "::", "/");
}

} // namespace commsdsl2tools_qt
