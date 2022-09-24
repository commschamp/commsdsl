//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "Swig.h"

#include "SwigComms.h"
#include "SwigDataBuf.h"
#include "SwigFrame.h"
#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigMessage.h"
#include "SwigMsgHandler.h"
#include "SwigMsgId.h"
#include "SwigProtocolOptions.h"
#include "SwigSchema.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{


bool Swig::swigWrite(SwigGenerator& generator)
{
    Swig obj(generator);
    return obj.swigWriteInternal();
}

bool Swig::swigWriteInternal() const
{

    auto& schema = m_generator.protocolSchema();        
    auto swigName = schema.mainNamespace() + ".i";
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), swigName);
    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "%module(directors=\"1\") #^#NS#$#\n"
        "#^#PREPEND#$#\n"
        "#^#CODE#$#\n"
        "#^#DEF#$#\n"
        "#^#APPEND#$#\n"
        ;      

    util::ReplacementMap repl = {
        {"NS", schema.mainNamespace()},
        {"CODE", swigCodeBlockInternal()},
        {"DEF", swigDefInternal()},
        {"PREPEND", util::readFileContents(m_generator.swigInputCodePathForFile(swigName + strings::prependFileSuffixStr()))},
        {"APPEND", util::readFileContents(m_generator.swigInputCodePathForFile(swigName + strings::appendFileSuffixStr()))},
    };

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Swig::swigCodeBlockInternal() const
{
    util::StringsList includes = {
        "comms/comms.h"
    };


    SwigProtocolOptions::swigAddCodeIncludes(m_generator, includes);

    util::StringsList codeElems;

    SwigComms::swigAddCode(codeElems);
    SwigDataBuf::swigAddCode(m_generator, codeElems);
    SwigMsgId::swigAddCode(m_generator, codeElems);
    SwigProtocolOptions::swigAddCode(m_generator, codeElems);

    SwigMsgHandler::swigAddFwdCode(m_generator, codeElems);

    SwigGenerator::cast(m_generator).swigMainInterface()->swigAddCode(codeElems);

    for (auto* m : m_generator.getAllMessagesFromAllSchemas()) {
        SwigMessage::cast(m)->swigAddFwdCode(codeElems);
    }  

    SwigMsgHandler::swigAddClassCode(m_generator, codeElems);

    for (auto& sPtr : m_generator.schemas()) {
        auto* schema = SwigSchema::cast(sPtr.get());
        schema->swigAddCodeIncludes(includes);
        schema->swigAddCode(codeElems);
    }

    SwigMsgHandler::swigAddFuncsCode(m_generator, codeElems);

    auto allFrames = m_generator.getAllFramesFromAllSchemas();
    for (auto* fPtr : allFrames) {
        auto* frame = SwigFrame::cast(fPtr);
        frame->swigAddCode(codeElems);
    }

    static const std::string Templ = 
        "%{\n"
        "#^#INCLUDES#$#\n"
        "#^#CODE#$#\n"
        "%}\n"
        ;

    comms::prepareIncludeStatement(includes);
    util::ReplacementMap repl = {
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"CODE", util::strListToString(codeElems, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string Swig::swigDefInternal() const
{
    auto includeWrap = 
        [](const std::string& str)
        {
            return "%include \"" + str + '\"';
        };

    util::StringsList stdIncludes = {
        includeWrap("std_array.i"),
        includeWrap("std_pair.i"),
        includeWrap("std_string.i"),
        includeWrap("std_vector.i")
    };

    util::StringsList defs;

    SwigComms::swigAddDef(defs);
    SwigDataBuf::swigAddDef(m_generator, defs);
    SwigMsgId::swigAddDef(m_generator, defs);

    for (auto& sPtr : m_generator.schemas()) {
        SwigSchema::cast(sPtr.get())->swigAddDef(defs);
    }    

    SwigMsgHandler::swigAddDef(m_generator, defs);

    auto allFrames = m_generator.getAllFrames();
    for (auto* fPtr : allFrames) {
        auto* frame = SwigFrame::cast(fPtr);
        frame->swigAddDef(defs);
    } 

    static const std::string Templ = 
        "#^#STD_INCLUDES#$#\n"
        "#^#DEFS#$#\n";

    util::ReplacementMap repl = {
        {"STD_INCLUDES", util::strListToString(stdIncludes, "\n", "\n")},
        {"DEFS", util::strListToString(defs, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig
