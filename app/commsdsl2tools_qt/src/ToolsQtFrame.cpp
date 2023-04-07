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

#include "ToolsQtFrame.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtMsgFactoryOptions.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtInputMessages.h"
#include "ToolsQtInterface.h"

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

ToolsQtFrame::StringsList ToolsQtFrame::toolsSourceFiles() const
{
    return StringsList{toolsTransportMessageSrcFilePathInternal()};
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
        toolsWriteHeaderInternal() &&
        toolsWriteTransportMsgHeaderInternal() &&
        toolsWriteTransportMsgSrcInternal();
}

bool ToolsQtFrame::toolsWriteHeaderInternal() const
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
        "\n"
        "#^#NS_BEGIN#$#\n"
        "#^#INTERFACE_TEMPL_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    ::#^#FRAME_SCOPE#$#<\n"
        "        #^#INTERFACE#$#,\n"
        "        #^#TOP_NS#$#::#^#ALL_MESSAGES#$##^#INTERFACE_TEMPL#$#,\n"
        "        #^#OPTS#$#\n"
        "    >;\n\n"
        "#^#NS_END#$#\n"
    ;

    StringsList includes {
        comms::relHeaderPathFor(*this, gen),
        ToolsQtInputMessages::toolsRelHeaderPath(gen),
        ToolsQtMsgFactoryOptions::toolsRelHeaderPath(gen),
    };

    auto allInterfaces = gen.toolsGetSelectedInterfaces();    
    assert(!allInterfaces.empty());
    if (allInterfaces.size() == 1U) {
        auto* defaultInterface = static_cast<const ToolsQtInterface*>(allInterfaces.front());
        includes.push_back(defaultInterface->toolsHeaderFilePath());
    }

    comms::prepareIncludeStatement(includes);

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"FRAME_SCOPE", comms::scopeFor(*this, gen)},
        {"TOP_NS", gen.getTopNamespace()},
        {"MAIN_NS", gen.protocolSchema().mainNamespace()},
        {"ALL_MESSAGES", comms::scopeForInput(strings::allMessagesStr(), gen)},
        {"OPTS", ToolsQtMsgFactoryOptions::toolsScope(gen)}
    };

    if (1U < allInterfaces.size()) {
        repl["INTERFACE_TEMPL_PARAM"] = "template <typename TInterface>";
        repl["INTERFACE"] = "TInterface";
        repl["INTERFACE_TEMPL"] = "<TInterface>";
    }
    else {
        auto* defaultInterface = static_cast<const ToolsQtInterface*>(allInterfaces.front());
        assert(defaultInterface != nullptr);
        repl["INTERFACE"] = gen.getTopNamespace() + "::" + comms::scopeFor(*defaultInterface, gen);
    }
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool ToolsQtFrame::toolsWriteTransportMsgHeaderInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto filePath = gen.getOutputDir() + '/' + toolsTransportMessageHeaderFilePathInternal();

    auto& logger = gen.logger();
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
        "#include <tuple>\n"
        "#include <QtCore/QVariantList>\n"
        "#include \"cc_tools_qt/TransportMessageBase.h\"\n"
        "#include \"#^#DEF_OPETIONS_INC#$#\"\n"
        "#include \"#^#FRAME_INCLUDE#$#\"\n"
        "#^#INTERFACE_INCLUDE#$#\n"
        "\n"
        "#^#NS_BEGIN#$#\n"
        "struct #^#CLASS_NAME#$##^#SUFFIX#$#Fields\n"
        "{\n"
        "    using All =\n"
        "        std::tuple<\n"
        "            #^#FIELDS#$#\n"
        "        >;\n"
        "    static const QVariantList& props();\n"
        "    #^#READ_FUNC#$#\n"
        "};\n\n"
        "#^#INTERFACE_TEMPL_PARAM#$#\n"
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public\n"
        "    cc_tools_qt::TransportMessageBase<\n"
        "        #^#INTERFACE#$#,\n"
        "        #^#CLASS_NAME#$##^#SUFFIX#$#Fields::All\n"
        "    >\n"
        "{\n"
        "    #^#BASE_DEF#$#\n"
        "protected:\n"
        "    virtual const QVariantList& fieldsPropertiesImpl() const override#^#SEMICOLON#$#\n"
        "    #^#PROPS_BODY#$#\n"
        "    #^#READ_FUNC_DECL#$#\n"
        "};\n\n"
        "#^#NS_END#$#\n"
    ;

    util::StringsList fields;
    for (auto* l : m_toolsLayers) {
        fields.push_back("::" + l->toolsFieldCommsScope());
    }

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"FRAME_INCLUDE", comms::relHeaderPathFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::transportMessageSuffixStr()},
        {"FIELDS", util::strListToString(fields, ",\n", "")},
        {"DEF_OPETIONS_INC", ToolsQtDefaultOptions::toolsRelHeaderPath(gen)},
    };

    auto allInterfaces = gen.toolsGetSelectedInterfaces();
    assert(!allInterfaces.empty());
    auto payloadOffset = toolsCalcBackPayloadOffsetInternal();

    if (1U < allInterfaces.size()) {
        std::string propsBody =
            "{\n"
            "    return " + comms::className(dslObj().name()) + strings::transportMessageSuffixStr() + strings::fieldsSuffixStr() + "::props();\n"
            "}\n";

        std::string baseDef = 
            "using Base =\n"
            "    cc_tools_qt::TransportMessageBase<\n"
            "        TInterface,\n"
            "        " + comms::className(dslObj().name())  + strings::transportMessageSuffixStr() + strings::fieldsSuffixStr() + "::All\n"
            "    >;";

        repl.insert({
            {"INTERFACE_TEMPL_PARAM", "template <typename TInterface>"},
            {"INTERFACE", "TInterface"},
            {"INTERFACE_TEMPL", "<TInterface>"},
            {"BASE_DEF", std::move(baseDef)},
            {"PROPS_BODY", std::move(propsBody)}
        });

        if (payloadOffset != 0U) {
            std::string readMemFunc =
                "using typename Base::ReadIterator;\n"
                "virtual comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t len) override\n"
                "{\n"
                "    return " + comms::className(dslObj().name()) + strings::transportMessageSuffixStr() + strings::fieldsSuffixStr() + "::read(Base::fields(), iter, len);\n"
                "}\n";
            repl["READ_FUNC"] = "static comms::ErrorStatus read(All& fields, const std::uint8_t*& iter, std::size_t len);";
            repl["READ_FUNC_DECL"] = std::move(readMemFunc);
        }        
    }
    else {
        auto* defaultInterface = static_cast<const ToolsQtInterface*>(allInterfaces.front());
        assert(defaultInterface != nullptr);
        repl["INTERFACE_INCLUDE"] = "#include \"" + defaultInterface->toolsHeaderFilePath() + '\"';
        repl["INTERFACE"] = gen.getTopNamespace() + "::" + comms::scopeFor(*defaultInterface, gen);

        if (payloadOffset != 0U) {
            repl["READ_FUNC_DECL"] = "virtual comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t len) override;";
        }        
    }

    if (repl["PROPS_BODY"].empty()) {
        repl["SEMICOLON"] = ";";
    }
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool ToolsQtFrame::toolsWriteTransportMsgSrcInternal() const
{
    auto& gen = ToolsQtGenerator::cast(generator());
    auto filePath = gen.getOutputDir() + '/' + toolsTransportMessageSrcFilePathInternal();

    auto& logger = gen.logger();
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
        "#^#FIELDS_PROPS#$#\n"
        "namespace\n"
        "{\n\n"
        "QVariantList createProps()\n"
        "{\n"
        "     QVariantList props;\n"
        "     #^#APPENDS#$#\n"
        "     return props;\n"
        "}\n\n"
        "} // namespace\n\n"
        "const QVariantList& #^#CLASS_NAME#$##^#SUFFIX#$#Fields::props()\n"
        "{\n"
        "    static const QVariantList Props = createProps();\n"
        "    return Props;\n"
        "}\n\n"
        "#^#PROPS_FUNC#$#\n"
        "#^#READ_FUNC#$#\n"
        "#^#PROPS_IMPL_FUNC#$#\n"
        "#^#READ_IMPL_FUNC#$#\n"
        "#^#NS_END#$#\n"
    ;

    util::StringsList includes = {
        "cc_tools_qt/property/field.h"
    };

    for (auto* l : m_toolsLayers) {
        auto lIncs = l->toolsSrcIncludes();
        includes.reserve(lIncs.size());
        std::move(lIncs.begin(), lIncs.end(), std::back_inserter(includes));
    }
    comms::prepareIncludeStatement(includes);

    util::StringsList fieldsProps;
    util::StringsList appends;
    fieldsProps.reserve(m_toolsLayers.size());
    appends.reserve(m_toolsLayers.size());
    for (auto* l : m_toolsLayers) {
        fieldsProps.push_back(l->toolsPropsFunc());
        appends.push_back("props.append(" + l->toolsCreatePropsInvocation() + ");");
    }  

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::transportMessageSuffixStr()},
        {"INCLUDES", util::strListToString(includes, "\n", "")},
        {"FIELDS_PROPS", util::strListToString(fieldsProps, "\n", "")},
        {"APPENDS", util::strListToString(appends, "\n", "")},
    };

    auto allInterfaces = gen.toolsGetSelectedInterfaces();
    assert(!allInterfaces.empty());
    auto payloadOffset = toolsCalcBackPayloadOffsetInternal();
    auto readIdxCalcFunc = 
        [this]()
        {
            auto payloadIter =
                std::find_if(
                    m_toolsLayers.begin(), m_toolsLayers.end(),
                    [](auto* l)
                    {
                        return l->layer().dslObj().kind() == commsdsl::parse::Layer::Kind::Payload;
                    });
            assert(payloadIter != m_toolsLayers.end());
            return static_cast<unsigned>(std::distance(m_toolsLayers.begin(), payloadIter)) + 1U;
        };        

    auto readOverrideFile = gen.getCodeDir() + '/' + toolsTransportMessageSrcFilePathInternal() + strings::readFileSuffixStr();
    auto readCode = util::readFileContents(readOverrideFile);

    do {
        // Handle multiple interfaces;
        if (allInterfaces.size() <= 1U) {
            break;
        }

        if (!readCode.empty()) {
            repl["READ_FUNC"] = std::move(readCode);  
            break;
        }

        if (payloadOffset == 0U) {
            // Nothing to do
            break;
        }

        auto readUntilIdx = readIdxCalcFunc();
        std::string readFunc = 
            "comms::ErrorStatus " + comms::className(dslObj().name()) + strings::transportMessageSuffixStr() + "Fields::read(All& fields, const std::uint8_t*& iter, std::size_t len)\n"
            "{\n"
            "    len -= " + util::numToString(payloadOffset) + ";\n"
            "    auto es = comms::ErrorStatus::NumOfErrorStatuses;\n";
        auto addToReadFunc = 
            [&readFunc](unsigned idx)
            {
                auto idxStr = std::to_string(idx);
                readFunc += 
                    "    auto& field" + idxStr + " = std::get<" + idxStr + ">(fields);\n"
                    "    es = field" + idxStr + ".read(iter, len);\n"
                    "    if (es != comms::ErrorStatus::Success) {\n"
                    "        return es;\n"
                    "    }\n"
                    "    len -= field" + idxStr + ".length();\n\n";

            };
        for (auto idx = 0U; idx < readUntilIdx; ++idx) {
            addToReadFunc(idx);
        }
        readFunc += "    len += " + util::numToString(payloadOffset) + ";\n";
        for (auto idx = readUntilIdx; idx < m_toolsLayers.size(); ++idx) {
            addToReadFunc(idx);
        }

        readFunc += "    return comms::ErrorStatus::Success;\n}\n";
        repl["READ_FUNC"] = std::move(readFunc);          
    } while (false);

    do {
        // Handle single interface
        if (1U < allInterfaces.size()) {
            break;
        }

        const std::string PropsImplFuncTempl = 
            "const QVariantList& #^#CLASS_NAME#$##^#SUFFIX#$#::fieldsPropertiesImpl() const\n"
            "{\n"
            "    return #^#CLASS_NAME#$##^#SUFFIX#$#Fields::props();\n"
            "}\n";  

        repl["PROPS_IMPL_FUNC"] = util::processTemplate(PropsImplFuncTempl, repl);             

        if (!readCode.empty()) {
            repl["READ_IMPL_FUNC"] = std::move(readCode);  
            break;
        }

        if (payloadOffset == 0U) {        
            break;
        }

        auto readUntilIdx = readIdxCalcFunc();

        std::string readFunc = 
            "comms::ErrorStatus " + comms::className(dslObj().name()) + strings::transportMessageSuffixStr() + "::readImpl(ReadIterator& iter, std::size_t len)\n"
            "{\n"
            "    len -= " + util::numToString(payloadOffset) + ";\n"
            "    auto es = doReadUntilAndUpdateLen<" + util::numToString(readUntilIdx) + ">(iter, len);\n"
            "    if (es == comms::ErrorStatus::Success) {\n"
            "        len += " + util::numToString(payloadOffset) + ";\n"
            "        es = doReadFrom<" + util::numToString(readUntilIdx) + ">(iter, len);\n"
            "    }\n\n"
            "    return es;\n"
            "}\n";
        repl["READ_IMPL_FUNC"] = std::move(readFunc);        
    } while (false);

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
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


} // namespace commsdsl2tools_qt
