//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "SwigAllMessages.h"
#include "SwigComms.h"
#include "SwigDataBuf.h"
#include "SwigFrame.h"
#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigMessage.h"
#include "SwigMsgHandler.h"
#include "SwigProtocolOptions.h"
#include "SwigSchema.h"
#include "SwigVersion.h"

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

bool Swig::swigWriteInternal()
{
    auto swigName = swigFileNameInternal();
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), swigName);
    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    do {
        auto replaceFile = util::readFileContents(m_generator.swigInputCodePathForFile(swigName + strings::replaceFileSuffixStr()));
        if (!replaceFile.empty()) {
            stream << replaceFile;
            break;
        }

        const std::string Templ = 
            "%module(directors=\"1\") #^#NS#$#\n\n"
            "#^#LANG_DEFS#$#\n"
            "#^#PREPEND#$#\n"
            "#^#CODE#$#\n"
            "#^#DEF#$#\n"
            "#^#APPEND#$#\n"
            ;      

        util::ReplacementMap repl = {
            {"NS", m_generator.protocolSchema().mainNamespace()},
            {"LANG_DEFS", swigLangDefsInternal()},
            {"CODE", swigCodeBlockInternal()},
            {"DEF", swigDefInternal()},
            {"PREPEND", swigPrependInternal()},
            {"APPEND", swigAppendInternal()},
        };

        auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
        stream << str;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Swig::swigCodeBlockInternal()
{
    util::StringsList includes = {
        "<iterator>",
        "comms/comms.h"
    };

    SwigProtocolOptions::swigAddCodeIncludes(m_generator, includes);
    SwigVersion::swigAddCodeIncludes(m_generator, includes);

    util::StringsList codeElems;

    SwigComms::swigAddCode(m_generator, codeElems);
    SwigDataBuf::swigAddCode(m_generator, codeElems);
    SwigVersion::swigAddCode(m_generator, codeElems);

    SwigProtocolOptions::swigAddCode(m_generator, codeElems);

    SwigMsgHandler::swigAddFwdCode(m_generator, codeElems);

    SwigGenerator::cast(m_generator).swigMainInterface()->swigAddCode(codeElems);

    for (auto& sPtr : m_generator.schemas()) {
        auto* schema = SwigSchema::cast(sPtr.get());
        schema->swigAddCodeIncludes(includes);
        schema->swigAddCode(codeElems);
    }

    SwigAllMessages::swigAddCode(m_generator, codeElems);

    SwigMsgHandler::swigAddClassCode(m_generator, codeElems);

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

std::string Swig::swigDefInternal()
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
    SwigVersion::swigAddDef(m_generator, defs);

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

std::string Swig::swigLangDefsInternal() const
{
    std::string pythonDefs = 
        "#ifdef SWIGPYTHON\n"
        "%{\n"
        "#define SWIG_FILE_WITH_INIT\n"
        "%}\n"
        "#endif // #ifdef SWIGPYTHON\n";

    std::string javaDefs = 
        "#ifdef SWIGJAVA\n"
        "%include \"enums.swg\"\n"
        //"%javaconst(1);\n"
        "#endif // #ifdef SWIGJAVA\n";

    std::string csharpDefs = 
        "#ifdef SWIGCSHARP\n"
        "#pragma SWIG nowarn=314\n"
        "#endif // #ifdef SWIGCSHARP\n";


    util::StringsList defs = {
        std::move(pythonDefs),
        std::move(javaDefs),
        std::move(csharpDefs)
    };

    return util::strListToString(defs, "\n", "");
}

std::string Swig::swigPrependInternal() const
{
    auto swigName = swigFileNameInternal();
    auto fromFile = util::readFileContents(m_generator.swigInputCodePathForFile(swigName + strings::prependFileSuffixStr()));
    if (!fromFile.empty()) {
        return fromFile;
    }

    const std::string Templ = 
        "// Use #^#NAME#$##^#SUFFIX#$# file to inject code here.\n";

    util::ReplacementMap repl = {
        {"NAME", swigName},
        {"SUFFIX", strings::prependFileSuffixStr()}
    };

    return util::processTemplate(Templ, repl);
}

std::string Swig::swigAppendInternal() const
{
    auto swigName = swigFileNameInternal();
    auto fromFile = util::readFileContents(m_generator.swigInputCodePathForFile(swigName + strings::appendFileSuffixStr()));
    if (!fromFile.empty()) {
        return fromFile;
    }

    const std::string Templ = 
        "// Use #^#NAME#$##^#SUFFIX#$# file to inject code here.\n";

    util::ReplacementMap repl = {
        {"NAME", swigName},
        {"SUFFIX", strings::appendFileSuffixStr()}
    };

    return util::processTemplate(Templ, repl);
}

std::string Swig::swigFileNameInternal() const
{
    auto& schema = m_generator.protocolSchema();        
    return schema.mainNamespace() + ".i";
}

} // namespace commsdsl2swig
