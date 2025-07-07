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

#include "ToolsQtFrame.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtInterface.h"
#include "ToolsQtNamespace.h"
#include "ToolsQtVersion.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <numeric>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

const std::string ProtTransportMsgSuffix("ProtTransportMessage");

ToolsQtFrame::ToolsQtLayersList toolsTransformLayersList(const commsdsl::gen::GenFrame::GenLayersList& layers)
{
    ToolsQtFrame::ToolsQtLayersList result;
    result.reserve(layers.size());

    std::transform(
        layers.begin(), layers.end(), std::back_inserter(result),
        [](auto& lPtr)
        {
            assert(lPtr);
            auto* toolsLayer = 
                const_cast<ToolsQtLayer*>(
                    dynamic_cast<const ToolsQtLayer*>(lPtr.get()));

            assert(toolsLayer != nullptr);
            return toolsLayer;
        });

    return result;
}

} // namespace 
    

ToolsQtFrame::ToolsQtFrame(ToolsQtGenerator& generator, commsdsl::parse::ParseFrame dslObj, commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent)
{
}

std::string ToolsQtFrame::toolsHeaderFilePath(const commsdsl::gen::GenInterface& iFace) const
{
    return toolsRelPathInternal(iFace) + strings::genCppHeaderSuffixStr();
}

ToolsQtFrame::StringsList ToolsQtFrame::toolsSourceFiles(const commsdsl::gen::GenInterface& iFace) const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    auto iter = selectedFrames.find(&iFace);
    if (iter == selectedFrames.end()) {
        return StringsList();
    }

    auto frameIter = std::find(iter->second.begin(), iter->second.end(), this);
    if (frameIter == iter->second.end()) {
        return StringsList();
    }

    return 
        StringsList{
            toolsRelPathInternal(iFace) + strings::genTransportMessageSuffixStr() + strings::genCppSourceSuffixStr(),
            toolsRelPathInternal(iFace) + strings::genCppSourceSuffixStr(),
        };
}

std::string ToolsQtFrame::toolsClassScope(const commsdsl::gen::GenInterface& iFace) const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    return gen.toolsScopePrefixForInterface(iFace) + comms::genScopeFor(*this, gen);    
}

bool ToolsQtFrame::genPrepareImpl()
{
    if (!Base::genPrepareImpl()) {
        return false;
    }

    m_toolsLayers = toolsTransformLayersList(Base::genLayers());
    return true;
}

bool ToolsQtFrame::genWriteImpl() const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto frames = gen.toolsGetSelectedFrames();
    auto iter = std::find(frames.begin(), frames.end(), this);
    if (iter == frames.end()) {
        // Frame not used.
        return true;
    }

    return 
        toolsWriteProtTransportMsgHeaderInternal() &&
        toolsWriteTransportMsgHeaderInternal() &&
        toolsWriteTransportMsgSrcInternal() &&
        toolsWriteHeaderInternal() &&
        toolsWriteSrcInternal();
}

bool ToolsQtFrame::toolsWriteProtTransportMsgHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto& logger = gen.genLogger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        auto filePath = gen.genGetOutputDir() + '/' + toolsRelPathInternal(*info.first) + ProtTransportMsgSuffix + strings::genCppHeaderSuffixStr();

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

        util::GenStringsList includes = {
            "<tuple>",
            "cc_tools_qt/ToolsTransportProtMessageBase.h",
            comms::genRelHeaderPathFor(*this, gen),
        };

        comms::genPrepareIncludeStatement(includes);

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "\n"
            "#pragma once\n\n"
            "#^#INCLUDES#$#\n"
            "#^#INC#$#\n"
            "\n"
            "#^#TOP_NS_BEGIN#$#\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n"
            "#^#NS_END#$#\n"
            "#^#TOP_NS_END#$#\n"
            ;

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
            {"NS_END", comms::genNamespaceEndFor(*this, gen)},
            {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*info.first)},
            {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*info.first)},
            {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
            {"INC", toolsProtTransportMsgHeaderExtraIncInternal(*info.first)},
            {"DEF", toolsProtTransportMsgDefInternal(*info.first)},
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

bool ToolsQtFrame::toolsWriteHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto& logger = gen.genLogger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        assert(info.first != nullptr);
        auto filePath = gen.genGetOutputDir() + '/' + toolsHeaderFilePath(*info.first);
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
            "#include <memory>\n\n"
            "#include \"cc_tools_qt/ToolsFrame.h\"\n"
            "\n"
            "#^#TOP_NS_BEGIN#$#\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n"
            "#^#NS_END#$#\n"
            "#^#TOP_NS_END#$#\n"
        ;

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
            {"NS_END", comms::genNamespaceEndFor(*this, gen)},
            {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*info.first)},
            {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*info.first)},
            {"DEF", toolsFrameHeaderDefInternal()},
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

bool ToolsQtFrame::toolsWriteSrcInternal() const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto& logger = gen.genLogger();

    auto* parent = genGetParent();
    assert((parent != nullptr) && (parent->genElemType() == commsdsl::gen::GenElem::Type_Namespace));
    auto* parentNs = ToolsQtNamespace::cast(static_cast<const commsdsl::gen::GenNamespace*>(parent));

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        assert(info.first != nullptr);
        auto filePath = gen.genGetOutputDir() + '/' + toolsRelPathInternal(*info.first) + strings::genCppSourceSuffixStr();
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
            "#include \"#^#CLASS_NAME#$#.h\"\n\n"
            "#^#INCLUDES#$#\n"
            "\n"
            "#^#TOP_NS_BEGIN#$#\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n"
            "#^#NS_END#$#\n"
            "#^#TOP_NS_END#$#\n"
        ;

        StringsList includes {
            "cc_tools_qt/ToolsFrameBase.h",
            comms::genRelHeaderPathFor(*this, gen),
            toolsRelPathInternal(*info.first) + strings::genTransportMessageSuffixStr() + strings::genCppHeaderSuffixStr(),
            ToolsQtDefaultOptions::toolsRelHeaderPath(gen),
            ToolsQtVersion::toolsRelHeaderPath(gen),
            parentNs->toolsFactoryRelHeaderPath(*info.first),
            ToolsQtInterface::cast(info.first)->toolsHeaderFilePath(),
        };

        comms::genPrepareIncludeStatement(includes);

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
            {"NS_END", comms::genNamespaceEndFor(*this, gen)},
            {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*info.first)},
            {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*info.first)},
            {"DEF", toolsFrameSrcDefInternal(*info.first)},
            {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
            {"INCLUDES", util::genStrListToString(includes, "\n", "")},
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

bool ToolsQtFrame::toolsWriteTransportMsgHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto& logger = gen.genLogger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        auto filePath = gen.genGetOutputDir() + '/' + toolsRelPathInternal(*info.first) + strings::genTransportMessageSuffixStr() + strings::genCppHeaderSuffixStr();

        logger.genInfo("Generating " + filePath);

        std::ofstream stream(filePath);
        if (!stream) {
            logger.genError("Failed to open \"" + filePath + "\" for writing.");
            return false;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "\n"
            "#pragma once\n\n"
            "#include <memory>\n"
            "#include \"cc_tools_qt/ToolsMessage.h\"\n"
            "\n"
            "#^#TOP_NS_BEGIN#$#\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n\n"
            "#^#NS_END#$#\n"
            "#^#TOP_NS_END#$#\n"
        ;

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
            {"NS_END", comms::genNamespaceEndFor(*this, gen)},
            {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*info.first)},
            {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*info.first)},
            {"DEF", toolsTransportMsgHeaderDefInternal()},
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

bool ToolsQtFrame::toolsWriteTransportMsgSrcInternal() const
{
    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto& logger = gen.genLogger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        auto filePath = gen.genGetOutputDir() + '/' + toolsRelPathInternal(*info.first) + strings::genTransportMessageSuffixStr() + strings::genCppSourceSuffixStr();

        logger.genInfo("Generating " + filePath);

        std::ofstream stream(filePath);
        if (!stream) {
            logger.genError("Failed to open \"" + filePath + "\" for writing.");
            return false;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "\n"
            "#include \"#^#CLASS_NAME#$##^#SUFFIX#$#.h\"\n\n"
            "#^#INCLUDES#$#\n"
            "\n"
            "#^#TOP_NS_BEGIN#$#\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n\n"
            "#^#NS_END#$#\n"
            "#^#TOP_NS_END#$#\n"
        ;

        util::GenStringsList includes = {
            "cc_tools_qt/ToolsTransportMessageBase.h",
            toolsRelPathInternal(*info.first) + ProtTransportMsgSuffix + strings::genCppHeaderSuffixStr(),
            ToolsQtInterface::cast(*info.first).toolsHeaderFilePath(),
        };

        comms::genPrepareIncludeStatement(includes);

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
            {"NS_END", comms::genNamespaceEndFor(*this, gen)},
            {"TOP_NS_BEGIN", gen.toolsNamespaceBeginForInterface(*info.first)},
            {"TOP_NS_END", gen.toolsNamespaceEndForInterface(*info.first)},
            {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
            {"SUFFIX", strings::genTransportMessageSuffixStr()},
            {"INCLUDES", util::genStrListToString(includes, "\n", "")},
            {"DEF", toolsTransportMsgSrcDefInternal(*info.first)},
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


unsigned ToolsQtFrame::toolsCalcBackPayloadOffsetInternal() const
{
    auto payloadIter =
        std::find_if(
            m_toolsLayers.rbegin(), m_toolsLayers.rend(),
            [](auto* l)
            {
                return l->layer().genParseObj().parseKind() == commsdsl::parse::ParseLayer::ParseKind::Payload;
            });
    assert(payloadIter != m_toolsLayers.rend());

    return
        static_cast<unsigned>(
            std::accumulate(
                m_toolsLayers.rbegin(), payloadIter, std::size_t(0),
                [](std::size_t soFar, auto* l)
                {
                    return soFar + l->toolsMinFieldLength();
                }));
}

std::string ToolsQtFrame::toolsRelPathInternal(const commsdsl::gen::GenInterface& iFace) const
{
    return util::genStrReplace(toolsClassScope(iFace), "::", "/");
}

std::string ToolsQtFrame::toolsProtTransportMsgDefInternal(const commsdsl::gen::GenInterface& iFace) const
{
    static const std::string Templ = 
        "template <typename TOpt>\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$##^#FIELDS_SUFFIX#$#\n"
        "{\n"
        "    using All =\n"
        "        std::tuple<\n"
        "            #^#FIELDS_LIST#$#\n"
        "        >;\n"
        "};\n\n"
        "template <typename TMsgBase, typename TOpt>\n"
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public\n"
        "    #^#BASE#$#\n"
        "{\n"
        "    using Base =\n"
        "        #^#BASE#$#;\n"
        "public:\n"
        "    COMMS_MSG_FIELDS_NAMES(\n"
        "        #^#FIELDS_NAMES#$#\n"
        "    );\n\n"
        "    #^#READ_FUNC#$#\n"
        "};\n"
        ;

    auto layersScope = "::" + comms::genScopeFor(*this, genGenerator()) + strings::genLayersSuffixStr();
    util::GenStringsList fields;
    util::GenStringsList names;
    for (auto* l : m_toolsLayers) {
        assert(l != nullptr);
        names.push_back(comms::genAccessName(l->layer().genParseObj().parseName()));

        auto* externalField = l->layer().genExternalField();
        if (externalField != nullptr) {
            fields.push_back("::" + comms::genScopeFor(*externalField, genGenerator()) + "<TOpt>");
            continue;
        }

        auto lScope = "typename " + layersScope + "<TOpt>::" + comms::genClassName(l->layer().genParseObj().parseName());
        auto* memberField = l->layer().genMemberField();
        if (memberField != nullptr) {
            fields.push_back(lScope + strings::genMembersSuffixStr() + "::" + comms::genClassName(memberField->genParseObj().parseName()));
            continue;
        }

        fields.push_back(lScope + "::Field");  
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"SUFFIX", ProtTransportMsgSuffix},
        {"FIELDS_SUFFIX", strings::genFieldsSuffixStr()},
        {"FIELDS_LIST", util::genStrListToString(fields, ",\n", "")},
        {"FIELDS_NAMES", util::genStrListToString(names, ",\n", "")},
        {"READ_FUNC", toolsProtTransportMsgReadFuncInternal(iFace)},
    };

    static const std::string BaseTempl = 
        "cc_tools_qt::ToolsTransportProtMessageBase<\n"
        "    TMsgBase,\n"
        "    typename #^#CLASS_NAME#$##^#SUFFIX#$##^#FIELDS_SUFFIX#$#<TOpt>::All,\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#<TMsgBase, TOpt>\n"
        ">";
    repl["BASE"] = util::genProcessTemplate(BaseTempl, repl);

    return util::genProcessTemplate(Templ, repl);
}

std::string ToolsQtFrame::toolsProtTransportMsgHeaderExtraIncInternal(const commsdsl::gen::GenInterface& iFace) const
{
    auto incFile = genGenerator().genGetCodeDir() + '/' + toolsRelPathInternal(iFace) + ProtTransportMsgSuffix + strings::genCppHeaderSuffixStr() + strings::genIncFileSuffixStr();
    return util::genReadFileContents(incFile);
}

std::string ToolsQtFrame::toolsProtTransportMsgReadFuncInternal(const commsdsl::gen::GenInterface& iFace) const
{
    std::string readCode;
    do {
        auto readOverrideFile = genGenerator().genGetCodeDir() + '/' + toolsRelPathInternal(iFace) + ProtTransportMsgSuffix + strings::genCppHeaderSuffixStr() + strings::genReadFileSuffixStr();
        readCode = util::genReadFileContents(readOverrideFile);
        if (!readCode.empty()) {
            break;
        }

        auto payloadIter =
            std::find_if(
                m_toolsLayers.begin(), m_toolsLayers.end(),
                [](auto* l)
                {
                    return l->layer().genParseObj().parseKind() == commsdsl::parse::ParseLayer::ParseKind::Payload;
                });

        assert(payloadIter != m_toolsLayers.end());
        auto readUntilIdx = static_cast<unsigned>(std::distance(m_toolsLayers.begin(), payloadIter)) + 1U;
        if (m_toolsLayers.size() <= readUntilIdx) {
            break;
        }

        auto payloadOffset = toolsCalcBackPayloadOffsetInternal();
        if (payloadOffset == 0U) {
            assert(false); // Should not happen
            break;
        }

        static const std::string Templ = 
            "template <typename TIter>\n"
            "comms::ErrorStatus doRead(TIter& iter, std::size_t len)\n"
            "{\n"
            "    len -= #^#OFFSET#$#;\n"
            "    auto es = Base::template doReadUntilAndUpdateLen<#^#IDX#$#>(iter, len);\n"
            "    if (es == comms::ErrorStatus::Success) {\n"
            "        len += #^#OFFSET#$#;\n"
            "        es = Base::template doReadFrom<#^#IDX#$#>(iter, len);\n"
            "    }\n\n"
            "    return es;\n"
            "}\n";

        util::ReplacementMap repl = {
            {"OFFSET", util::genNumToString(payloadOffset)},
            {"IDX", util::genNumToString(readUntilIdx)},
        };

        readCode = util::genProcessTemplate(Templ, repl);  
    } while (false);

    return readCode;
}

std::string ToolsQtFrame::toolsTransportMsgHeaderDefInternal() const
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

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName()) + strings::genTransportMessageSuffixStr()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string ToolsQtFrame::toolsTransportMsgSrcDefInternal(const commsdsl::gen::GenInterface& iFace) const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$#Impl : public\n"
        "    cc_tools_qt::ToolsTransportMessageBase<\n"
        "        #^#INTERFACE#$#,\n"
        "        #^#TRANSPORT_MESSAGE#$#,\n"
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
        "protected:\n"
        "    #^#ID_FUNC#$#\n"
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

    auto& gen = ToolsQtGenerator::cast(genGenerator());

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName()) + strings::genTransportMessageSuffixStr()},
        {"TRANSPORT_MESSAGE", comms::genScopeFor(*this, gen) + ProtTransportMsgSuffix},
        {"INTERFACE", ToolsQtInterface::cast(iFace).toolsClassScope()},
    };

    auto idLayerIter = 
        std::find_if(
            m_toolsLayers.begin(), m_toolsLayers.end(),
            [](auto* l)
            {
                using Kind = commsdsl::parse::ParseLayer::ParseKind;
                auto kind = l->layer().genParseObj().parseKind();
                if (kind == Kind::Id) {
                    return true;
                }

                if (kind != Kind::Custom) {
                    return false;
                }

                auto customKind = commsdsl::parse::ParseCustomLayer(l->layer().genParseObj()).parseSemanticLayerType();
                return (customKind == Kind::Id);
            });

    if (idLayerIter != m_toolsLayers.end()) {
        auto idName = comms::genAccessName((*idLayerIter)->layer().genParseObj().parseName());
        repl["ID_FUNC"] = 
            "virtual qlonglong numericIdImpl() const override\n"
            "{\n"
            "    return static_cast<qlonglong>(msg().field_" + idName + "().getValue());\n"
            "}\n";
    }

    return util::genProcessTemplate(Templ, repl);    
}

std::string ToolsQtFrame::toolsFrameHeaderDefInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$#Impl;\n"
        "class #^#CLASS_NAME#$# : public cc_tools_qt::ToolsFrame\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    virtual ~#^#CLASS_NAME#$#();\n"
        "\n"
        "protected:\n"
        "    virtual cc_tools_qt::ToolsMessagesList readDataImpl(const cc_tools_qt::ToolsDataInfo& dataInfo, bool final) override;\n"
        "    virtual void updateMessageImpl(cc_tools_qt::ToolsMessage& msg) override;\n"
        "    virtual cc_tools_qt::ToolsMessagePtr createInvalidMessageImpl() override;\n"
        "    virtual cc_tools_qt::ToolsMessagePtr createRawDataMessageImpl() override;\n"
        "    virtual cc_tools_qt::ToolsMessagePtr createExtraInfoMessageImpl() override;\n"
        "    virtual cc_tools_qt::ToolsMessagesList createAllMessagesImpl() override;\n"
        "    virtual cc_tools_qt::ToolsMessagePtr createMessageImpl(const QString& idAsString, unsigned idx) override;\n"
        "    virtual DataSeq writeProtMsgImpl(const void* protInterface) override;\n"
        "\n"
        "private:\n"
        "    std::unique_ptr<#^#CLASS_NAME#$#Impl> m_pImpl;\n"
        "};\n"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string ToolsQtFrame::toolsFrameSrcDefInternal(const commsdsl::gen::GenInterface& iFace) const
{
    static const std::string Templ = 
        "namespace\n"
        "{\n"
        "using Prot#^#CLASS_NAME#$# =\n"
        "    ::#^#SCOPE#$#<\n"
        "        #^#INTERFACE#$#::ProtInterface,\n"
        "        ::#^#INPUT#$#<#^#INTERFACE#$#::ProtInterface, #^#OPTS#$#>,\n"
        "        #^#OPTS#$#\n"
        "     >;\n"
        "\n"
        "}// namespace\n\n"
        "class #^#CLASS_NAME#$#Impl : public\n"
        "   cc_tools_qt::ToolsFrameBase<\n"
        "       #^#INTERFACE#$#,\n"
        "       Prot#^#CLASS_NAME#$#,\n" 
        "       #^#MSG_FACTORY#$#,\n"
        "       #^#TRANSPORT_MSG#$#\n"
        "   >\n"
        "{\n"
        "};\n\n"
        "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() :\n"
        "    m_pImpl(new #^#CLASS_NAME#$#Impl)"
        "{\n"
        "}\n\n"
        "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n"
        "\n"
        "cc_tools_qt::ToolsMessagesList #^#CLASS_NAME#$#::readDataImpl(const cc_tools_qt::ToolsDataInfo& dataInfo, bool final)\n"
        "{\n"
        "    return m_pImpl->readData(dataInfo, final);\n"
        "}\n\n"
        "void #^#CLASS_NAME#$#::updateMessageImpl(cc_tools_qt::ToolsMessage& msg)\n"
        "{\n"
        "    return m_pImpl->updateMessage(msg);\n"
        "}\n\n"
        "cc_tools_qt::ToolsMessagePtr #^#CLASS_NAME#$#::createInvalidMessageImpl()\n"
        "{\n"
        "    return m_pImpl->createInvalidMessage();\n"
        "}\n\n"
        "cc_tools_qt::ToolsMessagePtr #^#CLASS_NAME#$#::createRawDataMessageImpl()\n"
        "{\n"
        "    return m_pImpl->createRawDataMessage();\n"
        "}\n\n"
        "cc_tools_qt::ToolsMessagePtr #^#CLASS_NAME#$#::createExtraInfoMessageImpl()\n"
        "{\n"
        "    return m_pImpl->createExtraInfoMessage();\n"
        "}\n\n"
        "cc_tools_qt::ToolsMessagesList #^#CLASS_NAME#$#::createAllMessagesImpl()\n"
        "{\n"
        "    return m_pImpl->createAllMessages();\n"
        "}\n\n"        
        "cc_tools_qt::ToolsMessagePtr #^#CLASS_NAME#$#::createMessageImpl(const QString& idAsString, unsigned idx)\n"
        "{\n"
        "    return m_pImpl->createMessage(idAsString, idx);\n"
        "}\n\n" 
        "#^#CLASS_NAME#$#::DataSeq #^#CLASS_NAME#$#::writeProtMsgImpl(const void* protInterface)\n"         
        "{\n"
        "    return m_pImpl->writeProtMsg(protInterface);\n"
        "}\n\n" 
        ;

    auto& gen = ToolsQtGenerator::cast(genGenerator());
    auto* parent = genGetParent();
    assert((parent != nullptr) && (parent->genElemType() == commsdsl::gen::GenElem::Type_Namespace));
    auto* parentNs = ToolsQtNamespace::cast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"INTERFACE", ToolsQtInterface::cast(iFace).toolsClassScope()},
        {"MSG_FACTORY", parentNs->toolsFactoryClassScope(iFace)},
        {"TRANSPORT_MSG",  toolsClassScope(iFace) + strings::genTransportMessageSuffixStr()},
        {"SCOPE", comms::genScopeFor(*this, gen)},
        {"INPUT", comms::genScopeForInput(strings::genAllMessagesStr(), gen, *parentNs)},
        {"OPTS", ToolsQtDefaultOptions::toolsClassScope(gen)}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt
