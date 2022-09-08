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
#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigMsgId.h"
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

    util::StringsList codeElems;

    SwigComms::swigAddCode(codeElems);
    SwigMsgId::swigAddCode(m_generator, codeElems);
    SwigGenerator::cast(m_generator).swigMainInterface()->swigAddCode(codeElems);

    for (auto& sPtr : m_generator.schemas()) {
        auto* schema = SwigSchema::cast(sPtr.get());
        schema->swigAddCodeIncludes(includes);
        schema->swigAddCode(codeElems);
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
    util::StringsList result;

    SwigComms::swigAddDef(result);
    SwigMsgId::swigAddDef(m_generator, result);

    for (auto& sPtr : m_generator.schemas()) {
        SwigSchema::cast(sPtr.get())->swigAddDef(result);
    }    

    return util::strListToString(result, "\n", "");
}

} // namespace commsdsl2swig
