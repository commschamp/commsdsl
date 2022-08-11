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

#include "CommsFrame.h"

#include "CommsCustomLayer.h"
#include "CommsGenerator.h"

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

bool hasIdLayerInternal(const CommsFrame::CommsLayersList& commsLayers)
{
    return
        std::any_of(
            commsLayers.begin(), commsLayers.end(),
            [](auto* l)
            {
                if (l->layer().dslObj().kind() == commsdsl::parse::Layer::Kind::Id) {
                    return true;
                }

                if (l->layer().dslObj().kind() != commsdsl::parse::Layer::Kind::Custom) {
                    return false;
                }

                using LayerKind = commsdsl::parse::Layer::Kind;
                return (static_cast<const CommsCustomLayer&>(l->layer()).customDslObj().semanticLayerType() == LayerKind::Id);
    });
}

} // namespace 
   

CommsFrame::CommsFrame(CommsGenerator& generator, commsdsl::parse::Frame dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
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

bool CommsFrame::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    for (auto& lPtr : layers()) {
        assert(lPtr);
        auto* commsLayer = dynamic_cast<const CommsLayer*>(lPtr.get());

        assert(commsLayer != nullptr);
        m_commsLayers.push_back(const_cast<CommsLayer*>(commsLayer));
    }

    assert(!m_commsLayers.empty());
    while (true) {
        bool rearanged = false;
        for (auto& l : m_commsLayers) {
            bool success = false;
            rearanged = l->commsReorder(m_commsLayers, success);

            if (!success) {
                return false;
            }

            if (rearanged) {
                // Order has changed restart from the beginning
                break;
            }
        }

        if (!rearanged) {
            // reordering is complete
            break;
        }
    }

    m_hasCommonCode = 
        std::any_of(
            m_commsLayers.begin(), m_commsLayers.end(),
            [](const auto* l)
            {
                return l->commsMemberField() != nullptr;
            });

    m_hasIdLayer = hasIdLayerInternal(m_commsLayers);
    return true;
}

bool CommsFrame::writeImpl() const
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

    auto& gen = generator();
    auto filePath = comms::commonHeaderPathFor(*this, gen);

    gen.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }    

    std::ofstream stream(filePath);
    if (!stream) {
        gen.logger().error("Failed to open \"" + filePath + "\" for writing.");
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

    util::ReplacementMap repl =  {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"INCLUDES", commsCommonIncludesInternal()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"SCOPE", comms::scopeFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"LAYERS_SUFFIX", strings::layersSuffixStr()},
        {"COMMON_SUFFIX", strings::commonSuffixStr()},
        {"BODY", commsCommonBodyInternal()},
    };      

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();      
}

bool CommsFrame::commsWriteDefInternal() const
{
    auto& gen = generator();
    auto filePath = comms::headerPathFor(*this, gen);

    gen.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }    

    std::ofstream stream(filePath);
    if (!stream) {
        gen.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }    

    auto inputCodePrefix = comms::inputCodePathFor(*this, gen);
    auto replaceCode = util::readFileContents(inputCodePrefix + strings::replaceFileSuffixStr());
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
        "    /// @details See definition of @b COMMS_PROTOCOL_LAYERS_ACCESS macro\n"
        "    ///     from COMMS library for details.\n"
        "    ///\n"
        "    ///     The generated functions are:\n"
        "    #^#ACCESS_FUNCS_DOC#$#\n"
        "    COMMS_PROTOCOL_LAYERS_ACCESS(\n"
        "        #^#LAYERS_ACCESS_LIST#$#\n"
        "    );\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#NS_END#$#\n"
        "#^#APPEND#$#\n";

    auto extendCode = util::readFileContents(inputCodePrefix + strings::extendFileSuffixStr());
    util::ReplacementMap repl =  {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"INCLUDES", commsDefIncludesInternal()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"OPTIONS", comms::scopeForOptions(strings::defaultOptionsStr(), gen)},
        {"HEADERFILE", comms::relHeaderPathFor(*this, gen)},
        {"LAYERS_DEF", commsDefLayersDefInternal()},
        {"FRAME_DEF", commsDefFrameBaseInternal()},
        {"INPUT_MESSAGES_DOC", commsDefInputMessagesDocInternal()},
        {"INPUT_MESSAGES", commsDefInputMessagesParamInternal()},
        {"ACCESS_FUNCS_DOC", commsDefAccessDocInternal()},
        {"LAYERS_ACCESS_LIST", commsDefAccessListInternal()},
        {"PUBLIC", util::readFileContents(inputCodePrefix + strings::publicFileSuffixStr())},
        {"PROTECTED", commsDefProtectedInternal()},
        {"PRIVATE", commsDefPrivateInternal()},
        {"EXTEND", extendCode},
        {"APPEND", util::readFileContents(comms::inputCodePathFor(*this, gen) + strings::appendFileSuffixStr())}
    };

    if (!extendCode.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string CommsFrame::commsCommonIncludesInternal() const
{
    util::StringsList includes;

    for (auto* commsLayer : m_commsLayers) {
        assert(commsLayer != nullptr);

        auto fIncludes = commsLayer->commsCommonIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }

    comms::prepareIncludeStatement(includes); 
    return util::strListToString(includes, "\n", "\n");
}

std::string CommsFrame::commsCommonBodyInternal() const
{
    util::StringsList fragments;
    for (auto* l : m_commsLayers) {
        auto code = l->commsCommonCode();
        if (!code.empty()) {
            fragments.push_back(std::move(code));
        }
    }

    return util::strListToString(fragments, "\n", "");
}

std::string CommsFrame::commsDefIncludesInternal() const
{
    auto& gen = generator();
    util::StringsList includes = {
        comms::relHeaderForOptions(strings::defaultOptionsClassStr(), gen),
        comms::relHeaderForInput(strings::allMessagesStr(), gen)
    };

    if (m_hasCommonCode) {
        includes.push_back(comms::relCommonHeaderPathFor(*this, gen));
    }

    for (auto* commsLayer : m_commsLayers) {
        assert(commsLayer != nullptr);

        auto fIncludes = commsLayer->commsDefIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }

    comms::prepareIncludeStatement(includes); 
    return util::strListToString(includes, "\n", "\n");
}

std::string CommsFrame::commsDefLayersDefInternal() const
{
    util::StringsList defs;
    defs.reserve(m_commsLayers.size() + 1);

    CommsLayer* prevLayer = nullptr;
    bool hasInputMessages = false;
    for (auto iter = m_commsLayers.rbegin(); iter != m_commsLayers.rend(); ++iter) {
        auto* layer = *iter;
        defs.push_back(layer->commsDefType(prevLayer, hasInputMessages));
        prevLayer = layer;
    }

    static const std::string StackDefTempl =
        "/// @brief Final protocol stack definition.\n"
        "#^#STACK_PARAMS#$#\n"
        "using Stack = #^#LAST_LAYER#$##^#LAST_LAYER_PARAMS#$#;\n";

    assert(prevLayer != nullptr);
    util::ReplacementMap repl = {
        {"LAST_LAYER", comms::className(prevLayer->layer().dslObj().name())}
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
    defs.push_back(util::processTemplate(StackDefTempl, repl));

    return util::strListToString(defs, "\n", "");
}

std::string CommsFrame::commsDefFrameBaseInternal() const
{
    auto str = comms::className(dslObj().name()) + strings::layersSuffixStr() + "<TOpt>::";
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
        return strings::emptyString();
    }

    return "/// @tparam TAllMessages All supported input messages.";
}

std::string CommsFrame::commsDefInputMessagesParamInternal() const
{
    if (!m_hasIdLayer) {
        return strings::emptyString();
    }

    return
        "typename TAllMessages = " + comms::scopeForInput(strings::allMessagesStr(), generator()) + "<TMessage>,";
}

std::string CommsFrame::commsDefAccessDocInternal() const
{
    util::StringsList lines;
    auto className = comms::className(dslObj().name());
    lines.reserve(m_commsLayers.size());
    std::transform(
        m_commsLayers.rbegin(), m_commsLayers.rend(), std::back_inserter(lines),
        [&className](auto& l)
        {
            return
                "///     @li layer_" + comms::accessName(l->layer().dslObj().name()) +
                "() for @ref " + className + 
                strings::layersSuffixStr() + "::" + comms::className(l->layer().dslObj().name()) + " layer.";
        });
    return util::strListToString(lines, "\n", "");
}

std::string CommsFrame::commsDefAccessListInternal() const
{
    util::StringsList names;
    names.reserve(m_commsLayers.size());
    std::transform(
        m_commsLayers.rbegin(), m_commsLayers.rend(), std::back_inserter(names),
        [](auto& l)
        {
            return comms::accessName(l->layer().dslObj().name());
        });
    return util::strListToString(names, ",\n", "");
}

std::string CommsFrame::commsDefProtectedInternal() const
{
    auto code = util::readFileContents(comms::inputCodePathFor(*this, generator()) + strings::protectedFileSuffixStr());
    if (code.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
    "protected:\n"
    "    #^#CODE#$#\n";

    util::ReplacementMap repl = {
        {"CODE", std::move(code)},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsFrame::commsDefPrivateInternal() const
{
    auto code = util::readFileContents(comms::inputCodePathFor(*this, generator()) + strings::privateFileSuffixStr());
    if (code.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
    "private:\n"
    "    #^#CODE#$#\n";

    util::ReplacementMap repl = {
        {"CODE", std::move(code)},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsFrame::commsCustomizationOptionsInternal(
    LayerOptsFunc layerOptsFunc,
    bool hasBase) const
{
    util::StringsList elems;
    for (auto iter = m_commsLayers.rbegin(); iter != m_commsLayers.rend(); ++iter) {
        auto* l = *iter;
        auto str = (l->*layerOptsFunc)();
        if (!str.empty()) {
            elems.push_back(std::move(str));
        }
    }

    if (elems.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "/// @brief Extra options for layers of\n"
        "///     @ref #^#SCOPE#$# frame.\n"
        "struct #^#NAME#$##^#SUFFIX#$##^#EXT#$#\n"
        "{\n"
        "    #^#LAYERS_OPTS#$#\n"
        "}; // struct #^#NAME#$##^#SUFFIX#$#\n";

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, generator())},
        {"NAME", comms::className(dslObj().name())},
        {"SUFFIX", strings::layersSuffixStr()},
        {"LAYERS_OPTS", util::strListToString(elems, "\n", "\n")}
    };

    if (hasBase) {
        auto& commsGen = static_cast<const CommsGenerator&>(generator());
        bool hasMainNs = commsGen.hasMainNamespaceInOptions();
        repl["EXT"] = " : public TBase::" + comms::scopeFor(*this, generator(), hasMainNs) + strings::layersSuffixStr();
    }

    return util::processTemplate(Templ, repl);    
}

} // namespace commsdsl2comms
