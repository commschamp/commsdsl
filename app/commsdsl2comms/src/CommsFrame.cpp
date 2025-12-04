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

#include "CommsFrame.h"

#include "CommsCustomLayer.h"
#include "CommsGenerator.h"
#include "CommsNamespace.h"

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

namespace commsdsl2comms
{

namespace
{

bool commsHasIdLayerInternal(const CommsFrame::CommsLayersList& commsLayers)
{
    return
        std::any_of(
            commsLayers.begin(), commsLayers.end(),
            [](auto* l)
            {
                if (l->commsGenLayer().genParseObj().parseKind() == commsdsl::parse::ParseLayer::ParseKind::Id) {
                    return true;
                }

                if (l->commsGenLayer().genParseObj().parseKind() != commsdsl::parse::ParseLayer::ParseKind::Custom) {
                    return false;
                }

                using LayerKind = commsdsl::parse::ParseLayer::ParseKind;
                return (static_cast<const CommsCustomLayer&>(l->commsGenLayer()).genCustomLayerParseObj().parseSemanticLayerType() == LayerKind::Id);
    });
}

} // namespace

CommsFrame::CommsFrame(CommsGenerator& generator, ParseFrame parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

CommsFrame::~CommsFrame() = default;

std::string CommsFrame::commsDefaultOptions() const
{
    return
        commsCustomizationOptionsInternal(
            &CommsLayer::commsDefaultOptions,
            false);
}

std::string CommsFrame::commsDataViewDefaultOptions() const
{
    return
        commsCustomizationOptionsInternal(
            &CommsLayer::commsDataViewDefaultOptions,
            true);
}

std::string CommsFrame::commsBareMetalDefaultOptions() const
{
    return
        commsCustomizationOptionsInternal(
            &CommsLayer::commsBareMetalDefaultOptions,
            true);
}

std::string CommsFrame::commsMsgFactoryDefaultOptions() const
{
    return
        commsCustomizationOptionsInternal(
            &CommsLayer::commsMsgFactoryDefaultOptions,
            true);
}

bool CommsFrame::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    bool success = true;
    auto reorderedLayers = getCommsOrderOfLayers(success);
    if (!success) {
        return false;
    }

    for (auto* l : reorderedLayers) {
        auto* commsLayer = CommsLayer::commsCast(l);
        assert(commsLayer != nullptr);
        m_commsLayers.push_back(const_cast<CommsLayer*>(commsLayer));
    }

    m_hasCommonCode =
        std::any_of(
            m_commsLayers.begin(), m_commsLayers.end(),
            [](const auto* l)
            {
                return l->commsMemberField() != nullptr;
            });

    m_hasIdLayer = commsHasIdLayerInternal(m_commsLayers);
    return true;
}

bool CommsFrame::genWriteImpl() const
{
    return
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

bool CommsFrame::commsWriteCommonInternal() const
{
    if (!m_hasCommonCode) {
        return true;
    }

    auto& gen = genGenerator();
    auto filePath = comms::genCommonHeaderPathFor(*this, gen);

    gen.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        gen.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains common template parameters independent functionality of\n"
        "///    fields used in definition of @ref #^#SCOPE#$# frame.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "/// @brief Common types and functions of fields using in definition of\n"
        "///     @ref #^#SCOPE#$# frame.\n"
        "/// @see #^#SCOPE#$##^#LAYERS_SUFFIX#$#\n"
        "struct #^#CLASS_NAME#$##^#LAYERS_SUFFIX#$##^#COMMON_SUFFIX#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "#^#NS_END#$#\n";

    util::GenReplacementMap repl =  {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"INCLUDES", commsCommonIncludesInternal()},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"SCOPE", comms::genScopeFor(*this, gen)},
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"LAYERS_SUFFIX", strings::genLayersSuffixStr()},
        {"COMMON_SUFFIX", strings::genCommonSuffixStr()},
        {"BODY", commsCommonBodyInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool CommsFrame::commsWriteDefInternal() const
{
    auto& gen = genGenerator();
    auto filePath = comms::genHeaderPathFor(*this, gen);

    gen.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        gen.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto inputCodePrefix = comms::genInputCodePathFor(*this, gen);
    auto replaceCode = util::genReadFileContents(inputCodePrefix + strings::genReplaceFileSuffixStr());
    if (!replaceCode.empty()) {
        stream << replaceCode;
        stream.flush();
        return stream.good();
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$#\"</b> frame class.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "/// @brief Layers definition of @ref #^#CLASS_NAME#$# frame class.\n"
        "/// @tparam TOpt Protocol options.\n"
        "/// @see @ref #^#CLASS_NAME#$#\n"
        "/// @headerfile #^#HEADERFILE#$#\n"
        "template <typename TOpt = #^#OPTIONS#$#>\n"
        "struct #^#CLASS_NAME#$#Layers\n"
        "{\n"
        "    #^#LAYERS_DEF#$#\n"
        "};\n\n"
        "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"</b> frame class.\n"
        "#^#DOC_DETAILS#$#\n"
        "/// @tparam TMessage Common interface class of all the messages\n"
        "#^#INPUT_MESSAGES_DOC#$#\n"
        "/// @tparam TOpt Frame definition options\n"
        "/// @headerfile #^#HEADERFILE#$#\n"
        "template <\n"
        "   typename TMessage,\n"
        "   #^#INPUT_MESSAGES#$#\n"
        "   typename TOpt = #^#OPTIONS#$#\n"
        ">\n"
        "class #^#CLASS_NAME#$##^#ORIG#$# : public\n"
        "    #^#FRAME_DEF#$#\n"
        "{\n"
        "    using Base = typename\n"
        "        #^#FRAME_DEF#$#;\n"
        "public:\n"
        "    /// @brief Allow access to frame definition layers.\n"
        "    /// @details See definition of @b COMMS_FRAME_LAYERS_NAMES macro\n"
        "    ///     from COMMS library for details.\n"
        "    ///\n"
        "    ///     The generated types and functions are:\n"
        "    #^#ACCESS_FUNCS_DOC#$#\n"
        "    COMMS_FRAME_LAYERS_NAMES(\n"
        "        #^#LAYERS_ACCESS_LIST#$#\n"
        "    );\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#NS_END#$#\n"
        "#^#APPEND#$#\n";

    auto extendCode = util::genReadFileContents(inputCodePrefix + strings::genExtendFileSuffixStr());
    util::GenReplacementMap repl =  {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"INCLUDES", commsDefIncludesInternal()},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"OPTIONS", comms::genScopeForOptions(strings::genDefaultOptionsStr(), gen)},
        {"HEADERFILE", comms::genRelHeaderPathFor(*this, gen)},
        {"LAYERS_DEF", commsDefLayersDefInternal()},
        {"FRAME_DEF", commsDefFrameBaseInternal()},
        {"INPUT_MESSAGES_DOC", commsDefInputMessagesDocInternal()},
        {"INPUT_MESSAGES", commsDefInputMessagesParamInternal()},
        {"ACCESS_FUNCS_DOC", commsDefAccessDocInternal()},
        {"LAYERS_ACCESS_LIST", commsDefAccessListInternal()},
        {"PUBLIC", util::genReadFileContents(inputCodePrefix + strings::genPublicFileSuffixStr())},
        {"PROTECTED", commsDefProtectedInternal()},
        {"PRIVATE", commsDefPrivateInternal()},
        {"EXTEND", extendCode},
        {"APPEND", util::genReadFileContents(comms::genInputCodePathFor(*this, gen) + strings::genAppendFileSuffixStr())}
    };

    if (!extendCode.empty()) {
        repl["ORIG"] = strings::genOrigSuffixStr();
    }

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string CommsFrame::commsCommonIncludesInternal() const
{
    util::GenStringsList includes;

    for (auto* commsLayer : m_commsLayers) {
        assert(commsLayer != nullptr);

        auto fIncludes = commsLayer->commsCommonIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CommsFrame::commsCommonBodyInternal() const
{
    util::GenStringsList fragments;
    for (auto* l : m_commsLayers) {
        auto code = l->commsCommonCode();
        if (!code.empty()) {
            fragments.push_back(std::move(code));
        }
    }

    return util::genStrListToString(fragments, "\n", "");
}

std::string CommsFrame::commsDefIncludesInternal() const
{
    assert(genGetParent()->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto& gen = genGenerator();
    util::GenStringsList includes = {
        comms::genRelHeaderForOptions(strings::genDefaultOptionsClassStr(), gen),
        comms::genRelHeaderForInput(strings::genAllMessagesStr(), gen, *(static_cast<const commsdsl::gen::GenNamespace*>(genGetParent())))
    };

    if (m_hasCommonCode) {
        includes.push_back(comms::genRelCommonHeaderPathFor(*this, gen));
    }

    for (auto* commsLayer : m_commsLayers) {
        assert(commsLayer != nullptr);

        auto fIncludes = commsLayer->commsDefIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CommsFrame::commsDefLayersDefInternal() const
{
    util::GenStringsList defs;
    defs.reserve(m_commsLayers.size() + 1);

    CommsLayer* prevLayer = nullptr;
    bool hasInputMessages = false;
    for (auto iter = m_commsLayers.rbegin(); iter != m_commsLayers.rend(); ++iter) {
        auto* layer = *iter;
        defs.push_back(layer->commsDefType(prevLayer, hasInputMessages));
        prevLayer = layer;
    }

    static const std::string StackDefTempl =
        "/// @brief Final framing layers stack definition.\n"
        "#^#STACK_PARAMS#$#\n"
        "using Stack = #^#LAST_LAYER#$##^#LAST_LAYER_PARAMS#$#;\n";

    assert(prevLayer != nullptr);
    util::GenReplacementMap repl = {
        {"LAST_LAYER", comms::genClassName(prevLayer->commsGenLayer().genParseObj().parseName())}
    };

    if (hasInputMessages) {
        std::string stackParams =
            "template<typename TMessage, typename TAllMessages>";
        std::string lastLayerParams = "<TMessage, TAllMessages>";
        repl.insert({
            {"STACK_PARAMS", "template<typename TMessage, typename TAllMessages>"},
            {"LAST_LAYER_PARAMS", "<TMessage, TAllMessages>"},
        });
    }
    defs.push_back(util::genProcessTemplate(StackDefTempl, repl));

    return util::genStrListToString(defs, "\n", "");
}

std::string CommsFrame::commsDefFrameBaseInternal() const
{
    auto str = comms::genClassName(genParseObj().parseName()) + strings::genLayersSuffixStr() + "<TOpt>::";
    if (m_hasIdLayer) {
        str += "template Stack<TMessage, TAllMessages>";
    }
    else {
        str += "Stack";
    }
    return str;
}

std::string CommsFrame::commsDefInputMessagesDocInternal() const
{
    if (!m_hasIdLayer) {
        return strings::genEmptyString();
    }

    return "/// @tparam TAllMessages All supported input messages.";
}

std::string CommsFrame::commsDefInputMessagesParamInternal() const
{
    if (!m_hasIdLayer) {
        return strings::genEmptyString();
    }

    assert(genGetParent()->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto& ns = *(static_cast<const commsdsl::gen::GenNamespace*>(genGetParent()));
    return
        "typename TAllMessages = " + comms::genScopeForInput(strings::genAllMessagesStr(), genGenerator(), ns) + "<TMessage>,";
}

std::string CommsFrame::commsDefAccessDocInternal() const
{
    util::GenStringsList lines;
    auto className = comms::genClassName(genParseObj().parseName());
    lines.reserve(m_commsLayers.size());
    std::transform(
        m_commsLayers.rbegin(), m_commsLayers.rend(), std::back_inserter(lines),
        [&className](auto& l)
        {
            auto accName = comms::genAccessName(l->commsGenLayer().genParseObj().parseName());
            return
                "///     @li @b Layer_" + accName + " type and @b layer_" + accName + "() function\n"
                "///         for @ref " + className +
                strings::genLayersSuffixStr() + "::" + comms::genClassName(l->commsGenLayer().genParseObj().parseName()) + " layer.";
        });
    return util::genStrListToString(lines, "\n", "");
}

std::string CommsFrame::commsDefAccessListInternal() const
{
    util::GenStringsList names;
    names.reserve(m_commsLayers.size());
    std::transform(
        m_commsLayers.rbegin(), m_commsLayers.rend(), std::back_inserter(names),
        [](auto& l)
        {
            return comms::genAccessName(l->commsGenLayer().genParseObj().parseName());
        });
    return util::genStrListToString(names, ",\n", "");
}

std::string CommsFrame::commsDefProtectedInternal() const
{
    auto code = util::genReadFileContents(comms::genInputCodePathFor(*this, genGenerator()) + strings::genProtectedFileSuffixStr());
    if (code.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
    "protected:\n"
    "    #^#CODE#$#\n";

    util::GenReplacementMap repl = {
        {"CODE", std::move(code)},
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsFrame::commsDefPrivateInternal() const
{
    auto code = util::genReadFileContents(comms::genInputCodePathFor(*this, genGenerator()) + strings::genPrivateFileSuffixStr());
    if (code.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
    "private:\n"
    "    #^#CODE#$#\n";

    util::GenReplacementMap repl = {
        {"CODE", std::move(code)},
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsFrame::commsCustomizationOptionsInternal(
    CommsLayerOptsFunc layerOptsFunc,
    bool hasBase) const
{
    util::GenStringsList elems;
    for (auto iter = m_commsLayers.rbegin(); iter != m_commsLayers.rend(); ++iter) {
        auto* l = *iter;
        auto str = (l->*layerOptsFunc)();
        if (!str.empty()) {
            elems.push_back(std::move(str));
        }
    }

    if (elems.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "/// @brief Extra options for layers of\n"
        "///     @ref #^#SCOPE#$# frame.\n"
        "struct #^#NAME#$##^#SUFFIX#$##^#EXT#$#\n"
        "{\n"
        "    #^#LAYERS_OPTS#$#\n"
        "}; // struct #^#NAME#$##^#SUFFIX#$#\n";

    util::GenReplacementMap repl = {
        {"SCOPE", comms::genScopeFor(*this, genGenerator())},
        {"NAME", comms::genClassName(genParseObj().parseName())},
        {"SUFFIX", strings::genLayersSuffixStr()},
        {"LAYERS_OPTS", util::genStrListToString(elems, "\n", "\n")}
    };

    if (hasBase) {
        auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
        bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();
        repl["EXT"] = " : public TBase::" + comms::genScopeFor(*this, genGenerator(), hasMainNs) + strings::genLayersSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2comms
