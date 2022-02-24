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

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

// #include <algorithm>
#include <cassert>
#include <fstream>
// #include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2new
{
   

CommsFrame::CommsFrame(CommsGenerator& generator, commsdsl::parse::Frame dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

CommsFrame::~CommsFrame() = default;

bool CommsFrame::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    for (auto& lPtr : layers()) {
        assert(lPtr);
        auto* commsLayer = dynamic_cast<const CommsLayer*>(lPtr.get());

        // TODO: remove condition
        if (commsLayer == nullptr) {
            continue;
        }

        assert(commsLayer != nullptr);
        m_commsLayers.push_back(const_cast<CommsLayer*>(commsLayer));
    }

    assert(!m_commsLayers.empty());
    // TODO: re-arrange layers
    return true;
}

bool CommsFrame::writeImpl()
{
    return 
    //     commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

bool CommsFrame::commsWriteDefInternal()
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
        "class #^#CLASS_NAME#$# : public\n"
        "    #^#FRAME_DEF#$#\n"
        "{\n"
        "    using Base =\n"
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
        "};\n\n"
        "#^#NS_END#$#\n"
        "#^#APPEND#$#\n";

    util::ReplacementMap repl =  {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"INCLUDES", commsDefIncludesInternal()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"OPTIONS", comms::scopeForOptions(strings::defaultOptionsStr(), gen)},
        {"HEADERFILE", comms::relHeaderPathFor(*this, gen)},
        // {"CODE", commsCommonLayersCodeInternal()},
        {"APPEND", util::readFileContents(comms::inputCodePathFor(*this, gen) + strings::appendFileSuffixStr())}

        // TODO:
    };

    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

std::string CommsFrame::commsDefIncludesInternal() const
{
    util::StringsList includes;

    for (auto* commsLayer : m_commsLayers) {
        assert(commsLayer != nullptr);

        auto fIncludes = commsLayer->commsDefIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }

    comms::prepareIncludeStatement(includes); 
    return util::strListToString(includes, "\n", "\n");
}

} // namespace commsdsl2new
