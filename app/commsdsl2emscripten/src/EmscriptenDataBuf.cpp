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

#include "EmscriptenDataBuf.h"

#include "EmscriptenGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

namespace 
{

const std::string ClassName("DataBuf");

} // namespace 
    

bool EmscriptenDataBuf::emscriptenWrite(EmscriptenGenerator& generator)
{
    EmscriptenDataBuf obj(generator);
    return 
        obj.emscriptenWriteHeaderInternal() && 
        obj.emscriptenWriteSrcInternal();
}

std::string EmscriptenDataBuf::emscriptenClassName(const EmscriptenGenerator& generator)
{
    return generator.emscriptenScopeNameForRoot(ClassName);
}

std::string EmscriptenDataBuf::emscriptenRelHeader(const EmscriptenGenerator& generator)
{
    return generator.emscriptenRelHeaderForRoot(ClassName);
}

const std::string& EmscriptenDataBuf::emscriptenMemViewFuncName()
{
    static const std::string Str("dataBufMemoryView");
    return Str;
}

const std::string& EmscriptenDataBuf::emscriptenJsArrayToDataBufFuncName()
{
    static const std::string Str("jsArrayToDataBuf");
    return Str;
}

void EmscriptenDataBuf::emscriptenAddSourceFiles(const EmscriptenGenerator& generator, StringsList& sources)
{
    sources.push_back(generator.emscriptenRelSourceForRoot(ClassName));
}

bool EmscriptenDataBuf::emscriptenWriteHeaderInternal() const
{
    auto filePath = m_generator.emscriptenAbsHeaderForRoot(ClassName);
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

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include <cstdint>\n"
        "#include <vector>\n\n"
        "#include <emscripten/val.h>\n\n"
        "using #^#CLASS_NAME#$# = std::vector<std::uint8_t>;\n\n"
        "emscripten::val #^#MEM_VIEW#$#(const #^#CLASS_NAME#$#& buf);\n"
        "#^#CLASS_NAME#$# #^#JS_ARRAY#$#(const emscripten::val& buf);\n"
        ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"CLASS_NAME", emscriptenClassName(m_generator)},
        {"MEM_VIEW", emscriptenMemViewFuncName()},
        {"JS_ARRAY", emscriptenJsArrayToDataBufFuncName()},
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


bool EmscriptenDataBuf::emscriptenWriteSrcInternal() const
{
    auto filePath = m_generator.emscriptenAbsSourceForRoot(ClassName);
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

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#include <emscripten/bind.h>\n\n"
        "emscripten::val #^#MEM_VIEW#$#(const #^#CLASS_NAME#$#& buf)\n"
        "{\n"
        "    return emscripten::val(emscripten::typed_memory_view(buf.size(), buf.data()));\n"
        "}\n\n"
        "#^#CLASS_NAME#$# #^#JS_ARRAY#$#(const emscripten::val& val)\n"
        "{\n"
        "    return emscripten::convertJSArrayToNumberVector<std::uint8_t>(val);\n"
        "}\n\n"
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::register_vector<std::uint8_t>(\"#^#CLASS_NAME#$#\");\n"
        "    emscripten::function(\"#^#MEM_VIEW#$#\", &#^#MEM_VIEW#$#);\n"
        "    emscripten::function(\"#^#JS_ARRAY#$#\", &#^#JS_ARRAY#$#);\n"
        "}\n"
        ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"HEADER", emscriptenRelHeader(m_generator)},
        {"CLASS_NAME", emscriptenClassName(m_generator)},
        {"MEM_VIEW", emscriptenMemViewFuncName()},
        {"JS_ARRAY", emscriptenJsArrayToDataBufFuncName()},
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
