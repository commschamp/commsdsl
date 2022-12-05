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

#include "EmscriptenComms.h"

#include "EmscriptenGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

namespace 
{

const std::string ErrorStatusScopeStr("comms::ErrorStatus");
const std::string OptionalModeScopeStr("comms::field::OptionalMode");

} // namespace 
    

bool EmscriptenComms::emscriptenWrite(EmscriptenGenerator& generator)
{
    EmscriptenComms obj(generator);
    return 
        obj.emscriptenWriteErrorStatusInternal() && 
        obj.emscriptenWriteOptionalModeInternal();
}

bool EmscriptenComms::emscriptenWriteErrorStatusInternal() const
{
    auto name = m_generator.emscriptenScopeToName(ErrorStatusScopeStr);
    auto filePath = m_generator.emscriptenOutputCodeSrcPathForRoot(name);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }       

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Values[] = {
        "Success",
        "UpdateRequired",
        "NotEnoughData",
        "ProtocolError",
        "BufferOverflow",
        "InvalidMsgId",
        "InvalidMsgData",
        "MsgAllocFailure",
        "NotSupported",
        "NumOfErrorStatuses"
    };

    util::StringsList binds;
    for (auto& v : Values) {
        static const std::string Templ = 
            ".value(\"#^#VAL#$#\", #^#SCOPE#$#::#^#VAL#$#)";

        util::ReplacementMap repl = {
            {"VAL", v},
            {"SCOPE", ErrorStatusScopeStr}
        };

        binds.push_back(util::processTemplate(Templ, repl));
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include <emscripten/bind.h>\n\n"
        "#include \"comms/ErrorStatus.h\"\n\n"
        "EMSCRIPTEN_BINDINGS(#^#NAME#$#) {\n"
        "    enum_<#^#SCOPE#$#>(\"#^#NAME#$#\")\n"
        "        #^#BINDS#$#\n"
        "        ;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"NAME", name},
        {"SCOPE", ErrorStatusScopeStr},
        {"BINDS", util::strListToString(binds, "\n", "")}
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


bool EmscriptenComms::emscriptenWriteOptionalModeInternal() const
{
    auto name = m_generator.emscriptenScopeToName(OptionalModeScopeStr);
    auto filePath = m_generator.emscriptenOutputCodeSrcPathForRoot(name);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }       

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Values[] = {
        "Tentative",
        "Exists",
        "Missing",
        "NumOfModes"
    };

    util::StringsList binds;
    for (auto& v : Values) {
        static const std::string Templ = 
            ".value(\"#^#VAL#$#\", #^#SCOPE#$#::#^#VAL#$#)";

        util::ReplacementMap repl = {
            {"VAL", v},
            {"SCOPE", ErrorStatusScopeStr}
        };

        binds.push_back(util::processTemplate(Templ, repl));
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include <emscripten/bind.h>\n\n"
        "#include \"comms/field/OptionalMode.h\"\n\n"
        "EMSCRIPTEN_BINDINGS(#^#NAME#$#) {\n"
        "    enum_<#^#SCOPE#$#>(\"#^#NAME#$#\")\n"
        "        #^#BINDS#$#\n"
        "        ;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"NAME", name},
        {"SCOPE", OptionalModeScopeStr},
        {"BINDS", util::strListToString(binds, "\n", "")}
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

} // namespace commsdsl2emscripten
