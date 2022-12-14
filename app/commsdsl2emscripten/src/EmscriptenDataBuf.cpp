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
        "#include <stdint>\n"
        "#include <vector>\n\n"
        "#include <emscripten/val.h>\n\n"
        "// Extending standard vector just to add extra functions for bindings\n"
        "class #^#CLASS_NAME#$# : public std::vector<std::uint8_t>\n"
        "{\n"
        "    using Base = std::vector<std::uint8_t>;\n\n"
        "public:\n"
        "    DataBuf() = default;\n"
        "    DataBuf(const DataBuf&) = default;\n"
        "    DataBuf(DataBuf&&) = default;\n\n"
        "    // Use this constructor to convert from js array\n"
        "    DataBuf(const emscripten::val& jsArray)\n"
        "    {\n"
        "        asVector() = emscripten::convertJSArrayToNumberVector<std::uint8_t>(jsArray);\n"
        "    }\n\n"
        "    ~DataBuf() = default;\n\n"
        "    size_type size() const\n"
        "    {\n"
        "        return Base::size();\n"
        "    }\n\n"
        "    void resize(size_type val)\n"
        "    {\n"
        "        Base::resize(val);\n"
        "    }\n\n"
        "    value_type get(std::size_t idx)\n"
        "    {\n"
        "        return Base::at(idx);\n"
        "    }\n\n"
        "    void set(std::size_t idx, value_type val)\n"
        "    {\n"
        "        Base::at(idx) = val;\n"
        "    }\n\n"
        "    void push_back(value_type val)\n"
        "    {\n"
        "        Base::push_back(val);\n"
        "    }\n\n"
        "    const value_type* data() const\n"
        "    {\n"
        "        return Base::data();\n"
        "    }\n\n"
        "    Base& asVector()\n"
        "    {\n"
        "        return *this;\n"
        "    }\n\n"
        "};\n"
        ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"CLASS_NAME", emscriptenClassName(m_generator)},
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
        "EMSCRIPTEN_BINDINGS(#^#NAME#$#) {\n"
        "    emscripten::class_<#^#NAME#$#>(\"#^#NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#NAME#$#&>()\n"
        "        .constructor<const emscripten::val&>()\n"
        "        .function(\"size\", &#^#NAME#$#::size)\n"
        "        .function(\"resize\", &#^#NAME#$#::resize)\n"
        "        .function(\"get\", &#^#NAME#$#::get)\n"
        "        .function(\"set\", &#^#NAME#$#::set)\n"
        "        .function(\"push_back\", &#^#NAME#$#::push_back)\n"
        "        .function(\"data\", &#^#NAME#$#::data)\n"
        "        ;\n"
        "}\n"
        ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"HEADER", emscriptenRelHeader(m_generator)},
        {"NAME", emscriptenClassName(m_generator)},
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
