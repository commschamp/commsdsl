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

#include "ToolsQtFrame.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtInputMessages.h"
#include "ToolsQtInterface.h"
#include "ToolsQtMsgFactory.h"
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

ToolsQtFrame::ToolsQtLayersList toolsTransformLayersList(const commsdsl::gen::Frame::LayersList& layers)
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
    

ToolsQtFrame::ToolsQtFrame(ToolsQtGenerator& generator, commsdsl::parse::Frame dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

std::string ToolsQtFrame::toolsHeaderFilePath() const
{
    return toolsRelFilePath() + strings::cppHeaderSuffixStr();
}

std::string ToolsQtFrame::toolsHeaderFilePath(const commsdsl::gen::Interface& iFace) const
{
    return toolsRelPathInternal(iFace) + strings::cppHeaderSuffixStr();
}

ToolsQtFrame::StringsList ToolsQtFrame::toolsSourceFiles(const commsdsl::gen::Interface& iFace) const
{
    auto& gen = ToolsQtGenerator::cast(generator());
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
            toolsRelPathInternal(iFace) + strings::transportMessageSuffixStr() + strings::cppSourceSuffixStr(),
            toolsRelPathInternal(iFace) + strings::cppSourceSuffixStr(),
        };
}

std::string ToolsQtFrame::toolsMsgFactoryOptions() const
{
    util::StringsList elems;
    for (auto iter = m_toolsLayers.rbegin(); iter != m_toolsLayers.rend(); ++iter) {
        auto* l = *iter;
        auto str = l->toolsMsgFactoryOptions();
        if (!str.empty()) {
            elems.push_back(std::move(str));
        }
    }

    if (elems.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "struct #^#NAME#$##^#SUFFIX#$# : public #^#DEFAULT_OPTS#$#::#^#SCOPE#$##^#SUFFIX#$#\n"
        "{\n"
        "    #^#LAYERS_OPTS#$#\n"
        "}; // struct #^#NAME#$##^#SUFFIX#$#\n";

    auto& gen = ToolsQtGenerator::cast(generator());
    bool hasMainNs = gen.toolsHasMainNamespaceInOptions();

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, gen, hasMainNs)},
        {"NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::layersSuffixStr()},
        {"LAYERS_OPTS", util::strListToString(elems, "\n", "\n")},
        {"DEFAULT_OPTS", ToolsQtDefaultOptions::toolsScope(gen)}
    };

    return util::processTemplate(Templ, repl); 
}

std::string ToolsQtFrame::toolsClassScope() const
{
    return generator().getTopNamespace() + "::" + comms::scopeFor(*this, generator());
}

bool ToolsQtFrame::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_toolsLayers = toolsTransformLayersList(Base::layers());
    return true;
}

bool ToolsQtFrame::writeImpl() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
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
    auto& gen = ToolsQtGenerator::cast(generator());
    auto& logger = gen.logger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        auto filePath = gen.getOutputDir() + '/' + toolsRelPathInternal(*info.first) + ProtTransportMsgSuffix + strings::cppHeaderSuffixStr();

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

        util::StringsList includes = {
            "<tuple>",
            "cc_tools_qt/ToolsTransportProtMessageBase.h",
            comms::relHeaderPathFor(*this, gen),
        };

        comms::prepareIncludeStatement(includes);

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "\n"
            "#pragma once\n\n"
            "#^#INCLUDES#$#\n"
            "\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n"
            "#^#NS_END#$#\n"
            ;

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
            {"NS_END", comms::namespaceEndFor(*this, gen)},
            {"INCLUDES", util::strListToString(includes, "\n", "\n")},
            {"DEF", toolsProtTransportMsgDefInternal(*info.first)},
        };

        stream << util::processTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.error("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }

    return true;
}

bool ToolsQtFrame::toolsWriteHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto& logger = gen.logger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        assert(info.first != nullptr);
        auto filePath = gen.getOutputDir() + '/' + toolsHeaderFilePath(*info.first);
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
            "#include <memory>\n\n"
            "#include \"cc_tools_qt/ToolsFrame.h\"\n"
            "\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n"
            "#^#NS_END#$#\n"
        ;

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
            {"NS_END", comms::namespaceEndFor(*this, gen)},
            {"DEF", toolsFrameHeaderDefInternal()},
        };

        stream << util::processTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.error("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }
    return true;
}

bool ToolsQtFrame::toolsWriteSrcInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto& logger = gen.logger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        assert(info.first != nullptr);
        auto filePath = gen.getOutputDir() + '/' + toolsRelPathInternal(*info.first) + strings::cppSourceSuffixStr();
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
            "#include \"#^#CLASS_NAME#$#.h\"\n\n"
            "#^#INCLUDES#$#\n"
            "\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n"
            "#^#NS_END#$#\n"
        ;

        StringsList includes {
            "cc_tools_qt/ToolsFrameBase.h",
            comms::relHeaderPathFor(*this, gen),
            toolsRelPathInternal(*info.first) + strings::transportMessageSuffixStr() + strings::cppHeaderSuffixStr(),
            ToolsQtDefaultOptions::toolsRelHeaderPath(gen),
            ToolsQtVersion::toolsRelHeaderPath(gen),
            ToolsQtMsgFactory::toolsRelHeaderPath(gen, *info.first),
            ToolsQtInterface::cast(info.first)->toolsHeaderFilePath(),
        };

        comms::prepareIncludeStatement(includes);

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
            {"NS_END", comms::namespaceEndFor(*this, gen)},
            {"DEF", toolsFrameSrcDefInternal(*info.first)},
            {"CLASS_NAME", comms::className(dslObj().name())},
            {"INCLUDES", util::strListToString(includes, "\n", "")},
        };

        stream << util::processTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.error("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }
    return true;
}

bool ToolsQtFrame::toolsWriteTransportMsgHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto& logger = gen.logger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        auto filePath = gen.getOutputDir() + '/' + toolsRelPathInternal(*info.first) + strings::transportMessageSuffixStr() + strings::cppHeaderSuffixStr();

        logger.info("Generating " + filePath);

        std::ofstream stream(filePath);
        if (!stream) {
            logger.error("Failed to open \"" + filePath + "\" for writing.");
            return false;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "\n"
            "#pragma once\n\n"
            "#include <memory>\n"
            "#include \"cc_tools_qt/ToolsMessage.h\"\n"
            "\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n\n"
            "#^#NS_END#$#\n"
        ;

        util::StringsList fields;
        for (auto* l : m_toolsLayers) {
            fields.push_back("::" + l->toolsFieldCommsScope());
        }

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
            {"NS_END", comms::namespaceEndFor(*this, gen)},
            {"DEF", toolsTransportMsgHeaderDefInternal()},
        };
        
        stream << util::processTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.error("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }

    return true;
}

bool ToolsQtFrame::toolsWriteTransportMsgSrcInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto& logger = gen.logger();

    auto& selectedFrames = gen.toolsGetSelectedFramesPerInterface();
    for (auto& info : selectedFrames) {
        auto iter = std::find(info.second.begin(), info.second.end(), this);
        if (iter == info.second.end()) {
            continue;
        }

        auto filePath = gen.getOutputDir() + '/' + toolsRelPathInternal(*info.first) + strings::transportMessageSuffixStr() + strings::cppSourceSuffixStr();

        logger.info("Generating " + filePath);

        std::ofstream stream(filePath);
        if (!stream) {
            logger.error("Failed to open \"" + filePath + "\" for writing.");
            return false;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "\n"
            "#include \"#^#CLASS_NAME#$##^#SUFFIX#$#.h\"\n\n"
            "#^#INCLUDES#$#\n"
            "\n"
            "#^#NS_BEGIN#$#\n"
            "#^#DEF#$#\n\n"
            "#^#NS_END#$#\n"
        ;

        util::StringsList includes = {
            "cc_tools_qt/ToolsTransportMessageBase.h",
            toolsRelPathInternal(*info.first) + ProtTransportMsgSuffix + strings::cppHeaderSuffixStr(),
            ToolsQtInterface::cast(*info.first).toolsHeaderFilePath(),
        };

        comms::prepareIncludeStatement(includes);

        util::ReplacementMap repl = {
            {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
            {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
            {"NS_END", comms::namespaceEndFor(*this, gen)},
            {"CLASS_NAME", comms::className(dslObj().name())},
            {"SUFFIX", strings::transportMessageSuffixStr()},
            {"INCLUDES", util::strListToString(includes, "\n", "")},
            {"DEF", toolsTransportMsgSrcDefInternal(*info.first)},
        };

        stream << util::processTemplate(Templ, repl, true);
        stream.flush();
        if (!stream.good()) {
            logger.error("Write to \"" + filePath + "\" is unsuccessful.");
            return false;
        }
    }
    
    return true;
}

std::string ToolsQtFrame::toolsTransportMessageHeaderFilePathInternal() const
{
    return toolsRelFilePath() + strings::transportMessageSuffixStr() + strings::cppHeaderSuffixStr();
}

std::string ToolsQtFrame::toolsTransportMessageSrcFilePathInternal() const
{
    return toolsRelFilePath() + strings::transportMessageSuffixStr() + strings::cppSourceSuffixStr();
}

unsigned ToolsQtFrame::toolsCalcBackPayloadOffsetInternal() const
{
    auto payloadIter =
        std::find_if(
            m_toolsLayers.rbegin(), m_toolsLayers.rend(),
            [](auto* l)
            {
                return l->layer().dslObj().kind() == commsdsl::parse::Layer::Kind::Payload;
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

std::string ToolsQtFrame::toolsRelFilePath() const
{
    auto scope = comms::scopeFor(*this, generator());
    return generator().getTopNamespace() + '/' + util::strReplace(scope, "::", "/");
}

std::string ToolsQtFrame::toolsRelPathInternal(const commsdsl::gen::Interface& iFace) const
{
    auto scope = comms::scopeFor(*this, generator());
    auto iFaceScope = comms::scopeFor(iFace, generator(), false, true);
    return generator().getTopNamespace() + '/' + util::strReplace(iFaceScope, "::", "/") + '/' + util::strReplace(scope, "::", "/");
}

std::string ToolsQtFrame::toolsProtTransportMsgDefInternal(const commsdsl::gen::Interface& iFace) const
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

    auto layersScope = "::" + comms::scopeFor(*this, generator()) + strings::layersSuffixStr();
    util::StringsList fields;
    util::StringsList names;
    for (auto* l : m_toolsLayers) {
        assert(l != nullptr);
        names.push_back(comms::accessName(l->layer().dslObj().name()));

        auto* externalField = l->layer().externalField();
        if (externalField != nullptr) {
            fields.push_back("::" + comms::scopeFor(*externalField, generator()) + "<TOpt>");
            continue;
        }

        auto lScope = "typename " + layersScope + "<TOpt>::" + comms::className(l->layer().dslObj().name());
        auto* memberField = l->layer().memberField();
        if (memberField != nullptr) {
            fields.push_back(lScope + strings::membersSuffixStr() + "::" + comms::className(memberField->dslObj().name()));
            continue;
        }

        fields.push_back(lScope + "::Field");  
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"SUFFIX", ProtTransportMsgSuffix},
        {"FIELDS_SUFFIX", strings::fieldsSuffixStr()},
        {"FIELDS_LIST", util::strListToString(fields, ",\n", "")},
        {"FIELDS_NAMES", util::strListToString(names, ",\n", "")},
        {"READ_FUNC", toolsProtTransportMsgReadFuncInternal(iFace)},
    };

    static const std::string BaseTempl = 
        "cc_tools_qt::ToolsTransportProtMessageBase<\n"
        "    TMsgBase,\n"
        "    typename #^#CLASS_NAME#$##^#SUFFIX#$##^#FIELDS_SUFFIX#$#<TOpt>::All,\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#<TMsgBase, TOpt>\n"
        ">";
    repl["BASE"] = util::processTemplate(BaseTempl, repl);

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtFrame::toolsProtTransportMsgReadFuncInternal(const commsdsl::gen::Interface& iFace) const
{
    std::string readCode;
    do {
        auto readOverrideFile = generator().getCodeDir() + '/' + toolsRelPathInternal(iFace) + strings::cppHeaderSuffixStr() + strings::readFileSuffixStr();
        readCode = util::readFileContents(readOverrideFile);
        if (!readCode.empty()) {
            break;
        }

        auto payloadIter =
            std::find_if(
                m_toolsLayers.begin(), m_toolsLayers.end(),
                [](auto* l)
                {
                    return l->layer().dslObj().kind() == commsdsl::parse::Layer::Kind::Payload;
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
            "comms::ErrorStatus read(TIter& iter, std::size_t len)\n"
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
            {"OFFSET", util::numToString(payloadOffset)},
            {"IDX", util::numToString(readUntilIdx)},
        };

        readCode = util::processTemplate(Templ, repl);  
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
        {"CLASS_NAME", comms::className(dslObj().name()) + strings::transportMessageSuffixStr()},
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtFrame::toolsTransportMsgSrcDefInternal(const commsdsl::gen::Interface& iFace) const
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

    auto& gen = ToolsQtGenerator::cast(generator());

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name()) + strings::transportMessageSuffixStr()},
        {"TRANSPORT_MESSAGE", comms::scopeFor(*this, gen) + ProtTransportMsgSuffix},
        {"INTERFACE", ToolsQtInterface::cast(iFace).toolsScope()},
    };

    auto idLayerIter = 
        std::find_if(
            m_toolsLayers.begin(), m_toolsLayers.end(),
            [](auto* l)
            {
                using Kind = commsdsl::parse::Layer::Kind;
                auto kind = l->layer().dslObj().kind();
                if (kind == Kind::Id) {
                    return true;
                }

                if (kind != Kind::Custom) {
                    return false;
                }

                auto customKind = commsdsl::parse::CustomLayer(l->layer().dslObj()).semanticLayerType();
                return (customKind == Kind::Id);
            });

    if (idLayerIter != m_toolsLayers.end()) {
        auto idName = comms::accessName((*idLayerIter)->layer().dslObj().name());
        repl["ID_FUNC"] = 
            "virtual qlonglong numericIdImpl() const override\n"
            "{\n"
            "    return static_cast<qlonglong>(msg().field_" + idName + "().value());\n"
            "}\n";
    }

    return util::processTemplate(Templ, repl);    
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
        "\n"
        "private:\n"
        "    std::unique_ptr<#^#CLASS_NAME#$#Impl> m_pImpl;\n"
        "};\n"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())}
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtFrame::toolsFrameSrcDefInternal(const commsdsl::gen::Interface& iFace) const
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
        ;

    auto& gen = ToolsQtGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"INTERFACE", ToolsQtInterface::cast(iFace).toolsScope()},
        {"MSG_FACTORY", ToolsQtMsgFactory::toolsClassScope(gen)},
        {"TRANSPORT_MSG",  generator().getTopNamespace() + "::" + comms::scopeFor(*this, gen) + strings::transportMessageSuffixStr()},
        {"SCOPE", comms::scopeFor(*this, gen)},
        {"INPUT", comms::scopeForInput(strings::allMessagesStr(), gen)},
        {"OPTS", ToolsQtDefaultOptions::toolsScope(gen)}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt
