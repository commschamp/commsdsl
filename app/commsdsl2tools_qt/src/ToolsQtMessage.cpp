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

#include "ToolsQtMessage.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <type_traits>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtMessage::ToolsQtMessage(ToolsQtGenerator& generator, ParseMessage parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

std::string ToolsQtMessage::toolsHeaderPath(const commsdsl::gen::GenInterface& iFace) const
{
    return toolsRelPathInternal(iFace) + strings::genCppHeaderSuffixStr();
}

ToolsQtMessage::GenStringsList ToolsQtMessage::toolsSourceFiles(const commsdsl::gen::GenInterface& iFace) const
{
    return GenStringsList{toolsRelPathInternal(iFace) + strings::genCppSourceSuffixStr()};
}

std::string ToolsQtMessage::toolsClassScope(const commsdsl::gen::GenInterface& iFace) const
{
    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
    return gen.toolsScopePrefixForInterface(iFace) + comms::genScopeFor(*this, gen);
}

bool ToolsQtMessage::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_exists =
        genGenerator().genDoesElementExist(
            genParseObj().parseSinceVersion(),
            genParseObj().parseDeprecatedSince(),
            genParseObj().parseIsDeprecatedRemoved());

    if (!m_exists) {
        return true;
    }

    return true;
}

bool ToolsQtMessage::genWriteImpl() const
{
    if (!m_exists) {
        return true;
    }

    return toolsWriteHeaderInternal() && toolsWriteSrcInternal();
}

bool ToolsQtMessage::toolsWriteHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
    auto& logger = gen.genLogger();

    auto& allInterfaces = gen.toolsGetSelectedInterfaces();

    for (auto* iFace : allInterfaces) {
        assert(iFace != nullptr);
        auto filePath = gen.genGetOutputDir() + '/' + toolsRelPathInternal(*iFace) + strings::genCppHeaderSuffixStr();

        logger.genInfo("Generating " + filePath);

        auto dirPath = util::genPathUp(filePath);
        assert(!dirPath.empty());
        if (!gen.genCreateDirectory(dirPath)) {
            return false;
        }

        auto includes = toolsHeaderIncludesInternal();
        comms::genPrepareIncludeStatement(includes);

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
            "#^#NS_END#$#\n\n"
            "#^#TOP_NS_END#$#\n"
        ;

        util::GenReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
            {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
            {"NS_END", comms::genNamespaceEndFor(*this, gen)},
            {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*iFace)},
            {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*iFace)},
            {"DEF", toolsHeaderCodeInternal()},
        };

        stream << util::genProcessTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.genError("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }

    return true;
}

bool ToolsQtMessage::toolsWriteSrcInternal() const
{
    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());
    auto& logger = gen.genLogger();

    auto& allInterfaces = gen.toolsGetSelectedInterfaces();

    for (auto* iFace : allInterfaces) {
        assert(iFace != nullptr);
        auto filePath = gen.genGetOutputDir() + '/' + toolsRelPathInternal(*iFace) + strings::genCppSourceSuffixStr();
        logger.genInfo("Generating " + filePath);

        auto includes = toolsSrcIncludesInternal(*iFace);
        comms::genPrepareIncludeStatement(includes);

        std::ofstream stream(filePath);
        if (!stream) {
            logger.genError("Failed to open \"" + filePath + "\" for writing.");
            return false;
        }

        static const std::string Templ =
            "#^#GENERATED#$#\n"
            "\n"
            "#include \"#^#CLASS_NAME#$#.h\"\n\n"
            "#^#INCLUDES#$#\n"
            "#^#TOP_NS_BEGIN#$#\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n\n"
            "#^#NS_END#$#\n"
            "#^#TOP_NS_END#$#\n"
        ;

        util::GenReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
            {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
            {"NS_END", comms::genNamespaceEndFor(*this, gen)},
            {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*iFace)},
            {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*iFace)},
            {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
            {"DEF", toolsSrcCodeInternal(*iFace)},
        };

        stream << util::genProcessTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.genError("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }
    return true;
}

std::string ToolsQtMessage::toolsRelPathInternal(const commsdsl::gen::GenInterface& iFace) const
{
    return util::genScopeToRelPath(toolsClassScope(iFace));
}

ToolsQtMessage::ToolsIncludesList ToolsQtMessage::toolsHeaderIncludesInternal() const
{
    return ToolsIncludesList {
        "<memory>",
        "cc_tools_qt/ToolsMessage.h"
    };
}

std::string ToolsQtMessage::toolsHeaderCodeInternal() const
{
    static const std::string Templ =
        "class #^#CLASS_NAME#$#Impl;\n"
        "class #^#CLASS_NAME#$# : public cc_tools_qt::ToolsMessage\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = delete;\n"
        "    #^#CLASS_NAME#$#(#^#CLASS_NAME#$#&&) = delete;\n"
        "    virtual ~#^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#& operator=(const #^#CLASS_NAME#$#& other);\n"
        "    #^#CLASS_NAME#$#& operator=(#^#CLASS_NAME#$#&&);\n"
        "\n"
        "protected:\n"
        "    virtual const char* nameImpl() const override;\n"
        "    virtual bool refreshMsgImpl() override;\n"
        "    virtual qlonglong numericIdImpl() const override;\n"
        "    virtual QString idAsStringImpl() const override;\n"
        "    virtual void resetImpl() override;\n"
        "    virtual bool assignImpl(const cc_tools_qt::ToolsMessage& other) override;\n"
        "    virtual bool isValidImpl() const override;\n"
        "    virtual DataSeq encodeDataImpl() const override;\n"
        "    virtual bool decodeDataImpl(const DataSeq& data) override;\n"
        "    virtual Ptr cloneImpl() const override;\n"
        "    virtual void assignProtMessageImpl(void* protMsg) override;\n"
        "    virtual DataSeq encodeFramedImpl(cc_tools_qt::ToolsFrame& frame) const override;\n"
        "    virtual FieldsList transportFieldsImpl() override;\n"
        "    virtual FieldsList payloadFieldsImpl() override;\n"
        "\n"
        "private:\n"
        "    using ImplPtr = std::unique_ptr<#^#CLASS_NAME#$#Impl>;\n\n"
        "    #^#CLASS_NAME#$#(ImplPtr&& impl);\n\n"
        "    ImplPtr m_pImpl;\n"
        "};";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
    };

    return util::genProcessTemplate(Templ, repl);
}

ToolsQtMessage::ToolsIncludesList ToolsQtMessage::toolsSrcIncludesInternal(const commsdsl::gen::GenInterface& iFace) const
{
    return ToolsIncludesList {
        "cc_tools_qt/ToolsMessageBase.h",
        comms::genRelHeaderPathFor(*this, genGenerator()),
        ToolsQtInterface::toolsCast(iFace).toolsHeaderFilePath(),
    };
}

std::string ToolsQtMessage::toolsSrcCodeInternal(const commsdsl::gen::GenInterface& iFace) const
{
    static const std::string Templ =
        "class #^#CLASS_NAME#$#Impl : public\n"
        "    cc_tools_qt::ToolsMessageBase<\n"
        "        #^#INTERFACE#$#,\n"
        "        ::#^#PROT_MESSAGE#$#,\n"
        "        #^#CLASS_NAME#$#Impl\n"
        "    >\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#Impl() = default;\n"
        "    #^#CLASS_NAME#$#Impl(const #^#CLASS_NAME#$#Impl&) = default;\n"
        "    #^#CLASS_NAME#$#Impl(#^#CLASS_NAME#$#Impl&&) = default;\n"
        "    virtual ~#^#CLASS_NAME#$#Impl() = default;\n"
        "    #^#CLASS_NAME#$#Impl& operator=(const #^#CLASS_NAME#$#Impl&) = default;\n"
        "    #^#CLASS_NAME#$#Impl& operator=(#^#CLASS_NAME#$#Impl&&) = default;\n\n"
        "};\n\n"
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() : m_pImpl(new #^#CLASS_NAME#$#Impl) {}\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n\n"
        "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(const #^#CLASS_NAME#$#& other)\n"
        "{\n"
        "    *m_pImpl = *other.m_pImpl;\n"
        "    return *this;\n"
        "}\n\n"
        "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(#^#CLASS_NAME#$#&& other)\n"
        "{\n"
        "    *m_pImpl = std::move(*other.m_pImpl);\n"
        "    return *this;\n"
        "}\n\n"
        "const char* #^#CLASS_NAME#$#::nameImpl() const\n"
        "{\n"
        "    return m_pImpl->name();\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$#::refreshMsgImpl()\n"
        "{\n"
        "    return m_pImpl->refreshMsg();\n"
        "}\n\n"
        "qlonglong #^#CLASS_NAME#$#::numericIdImpl() const\n"
        "{\n"
        "    return m_pImpl->numericId();\n"
        "}\n\n"
        "QString #^#CLASS_NAME#$#::idAsStringImpl() const\n"
        "{\n"
        "    return m_pImpl->idAsString();\n"
        "}\n\n"
        "void #^#CLASS_NAME#$#::resetImpl()\n"
        "{\n"
        "    m_pImpl->reset();\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$#::assignImpl(const cc_tools_qt::ToolsMessage& other)\n"
        "{\n"
        "    auto* castedOther = dynamic_cast<const #^#CLASS_NAME#$#*>(&other);\n"
        "    if (castedOther == nullptr) {\n"
        "        return false;\n"
        "    }\n"
        "    return m_pImpl->assign(*castedOther->m_pImpl);\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$#::isValidImpl() const\n"
        "{\n"
        "    return m_pImpl->isValid();\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::DataSeq #^#CLASS_NAME#$#::encodeDataImpl() const\n"
        "{\n"
        "    return m_pImpl->encodeData();\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$#::decodeDataImpl(const DataSeq& data)\n"
        "{\n"
        "    return m_pImpl->decodeData(data);\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::Ptr #^#CLASS_NAME#$#::cloneImpl() const\n"
        "{\n"
        "    ImplPtr impl(static_cast<#^#CLASS_NAME#$#Impl*>(m_pImpl->clone().release()));\n"
        "    return Ptr(new #^#CLASS_NAME#$#(std::move(impl)));\n"
        "}\n\n"
        "void #^#CLASS_NAME#$#::assignProtMessageImpl(void* protMsg)\n"
        "{\n"
        "    m_pImpl->assignProtMessage(protMsg);\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::DataSeq #^#CLASS_NAME#$#::encodeFramedImpl(cc_tools_qt::ToolsFrame& frame) const\n"
        "{\n"
        "    return m_pImpl->encodeFramed(frame);\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::FieldsList #^#CLASS_NAME#$#::transportFieldsImpl()\n"
        "{\n"
        "    return m_pImpl->transportFields();\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::FieldsList #^#CLASS_NAME#$#::payloadFieldsImpl()\n"
        "{\n"
        "    return m_pImpl->payloadFields();\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#(ImplPtr&& impl) :\n"
        "    m_pImpl(std::move(impl))\n"
        "{\n"
        "}\n\n"
        ;

    auto& gen = ToolsQtGenerator::toolsCast(genGenerator());

    util::GenReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"PROT_MESSAGE", comms::genScopeFor(*this, gen)},
        {"INTERFACE", ToolsQtInterface::toolsCast(iFace).toolsClassScope()},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt
