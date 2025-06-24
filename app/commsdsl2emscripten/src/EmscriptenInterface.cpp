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

#include "EmscriptenInterface.h"

#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenNamespace.h"

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


EmscriptenInterface::EmscriptenInterface(EmscriptenGenerator& generator, commsdsl::parse::ParseInterface dslObj, commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent)
{
}   

EmscriptenInterface::~EmscriptenInterface() = default;

std::string EmscriptenInterface::emscriptenRelHeader() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    return gen.emscriptenRelHeaderFor(*this);
}

void EmscriptenInterface::emscriptenAddSourceFiles(StringsList& sources) const
{
    if (!isReferenced()) {
        return;
    }
    
    auto& gen = EmscriptenGenerator::cast(generator());
    sources.push_back(gen.emscriptenRelSourceFor(*this));
}

bool EmscriptenInterface::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    assert(isReferenced());

    m_emscriptenFields = EmscriptenField::emscriptenTransformFieldsList(fields());
    return true;
}

bool EmscriptenInterface::writeImpl() const
{
    assert(isReferenced());
    return 
        emscriptenWriteHeaderInternal() &&
        emscriptenWriteSourceInternal();
}


bool EmscriptenInterface::emscriptenWriteHeaderInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    auto filePath = gen.emscriptenAbsHeaderFor(*this);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n\n"
        "#^#FIELDS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"INCLUDES", emscriptenHeaderIncludesInternal()},
        {"FIELDS", emscriptenHeaderFieldsInternal()},
        {"DEF", emscriptenHeaderClassInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

bool EmscriptenInterface::emscriptenWriteSourceInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    auto filePath = gen.emscriptenAbsSourceFor(*this);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#include <emscripten/bind.h>\n\n"
        "#include \"#^#MSG_HANDLER#$#\"\n\n"
        "#^#FIELDS#$#\n"
        "#^#CODE#$#\n"
    ;

    auto* parentNs = parentNamespace();
    assert(parentNs != nullptr);

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"HEADER", gen.emscriptenRelHeaderFor(*this)},
        {"FIELDS", emscriptenSourceFieldsInternal()},
        {"CODE", emscriptenSourceCodeInternal()},
        {"MSG_HANDLER", EmscriptenNamespace::cast(parentNs)->emscriptenHandlerRelHeader()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

std::string EmscriptenInterface::emscriptenHeaderIncludesInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    util::StringsList includes = {
        "<iterator>",
        "<emscripten/val.h>",
        comms::relHeaderPathFor(*this, gen),
        EmscriptenDataBuf::emscriptenRelHeader(gen),
    };

    for (auto* f : m_emscriptenFields) {
        f->emscriptenHeaderAddExtraIncludes(includes);
    }

    comms::prepareIncludeStatement(includes);
    return util::strListToString(includes, "\n", "");
}

std::string EmscriptenInterface::emscriptenHeaderFieldsInternal() const
{
    util::StringsList fields;
    for (auto* f : m_emscriptenFields) {
        fields.push_back(f->emscriptenHeaderClass());
    }

    return util::strListToString(fields, "\n", "\n");
}

std::string EmscriptenInterface::emscriptenHeaderClassInternal() const
{
        auto& gen = EmscriptenGenerator::cast(generator());
        util::StringsList fields;
        for (auto* f : m_emscriptenFields) {
            static const std::string Templ = 
                "using Base::transportField_#^#NAME#$#;\n"
                "#^#FIELD_CLASS#$#* transportField_#^#NAME#$#_()\n"
                "{\n"
                "    return static_cast<#^#FIELD_CLASS#$#*>(&transportField_#^#NAME#$#());\n"
                "}\n";

            util::ReplacementMap repl = {
                {"FIELD_CLASS", gen.emscriptenClassName(f->field())},
                {"NAME", comms::accessName(f->field().dslObj().parseName())},
            };

            fields.push_back(util::processTemplate(Templ, repl));
        }

    const std::string Templ = 
        "class #^#MSG_HANDLER#$#;\n"
        "class #^#CLASS_NAME#$# : public\n"
        "    #^#BASE#$#\n"
        "{\n"
        "    using Base =\n"
        "        #^#BASE#$#;\n\n"
        "public:\n"
        "    #^#FIELDS#$#\n"
        "    comms::ErrorStatus readDataBuf(const #^#DATA_BUF#$#& buf)\n"
        "    {\n"
        "        auto iter = buf.begin();\n"
        "        return Base::read(iter, buf.size());\n"
        "    }\n\n"   
        "    comms::ErrorStatus readJsArray(const emscripten::val& jsArray)\n"
        "    {\n"
        "        auto dataBuf = #^#JS_ARRAY_FUNC#$#(jsArray);\n"
        "        return readDataBuf(dataBuf);\n"
        "    }\n\n" 
        "    comms::ErrorStatus writeDataBuf(#^#DATA_BUF#$#& buf) const\n"
        "    {\n"
        "        auto iter = std::back_inserter(buf);\n"
        "        return Base::write(iter, buf.max_size());\n"
        "    }\n\n"
        "    #^#MSG_ID#$# getId() const\n"
        "    {\n"
        "        return Base::getId();\n"
        "    }\n\n"
        "    bool refresh()\n"
        "    {\n"
        "        return Base::refresh();\n"
        "    }\n\n"
        "    std::size_t length() const\n"
        "    {\n"
        "        return Base::length();\n"
        "    }\n\n"
        "    bool valid() const\n"
        "    {\n"
        "        return Base::valid();\n"
        "    }\n\n"
        "    std::string name() const\n"
        "    {\n"
        "        return std::string(Base::name());\n"
        "    }\n\n"
        "    void dispatch(#^#MSG_HANDLER#$#& handler)\n"
        "    {\n"
        "        Base::dispatch(handler);\n"
        "    }\n"        
        "};\n";   
        
        auto* parentNs = parentNamespace();
        assert(parentNs != nullptr);
        
        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.emscriptenClassName(*this)},
            {"BASE", emscriptenHeaderBaseInternal()},
            {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(gen)},
            {"JS_ARRAY_FUNC", EmscriptenDataBuf::emscriptenJsArrayToDataBufFuncName()},
            {"MSG_ID", comms::scopeForMsgId(strings::msgIdEnumNameStr(), gen, *parentNs)},
            {"FIELDS", util::strListToString(fields, "\n", "")},
            {"MSG_HANDLER", EmscriptenNamespace::cast(parentNs)->emscriptenHandlerClassName()},
        };
    return util::processTemplate(Templ, repl);
}

std::string EmscriptenInterface::emscriptenHeaderBaseInternal() const
{
    const std::string Templ = 
        "#^#COMMS_CLASS#$#<\n"
        "    comms::option::app::ReadIterator<#^#DATA_BUF#$#::const_iterator>,\n"
        "    comms::option::app::WriteIterator<std::back_insert_iterator<#^#DATA_BUF#$#> >,\n"
        "    comms::option::app::IdInfoInterface,\n"        
        "    comms::option::app::ValidCheckInterface,\n"
        "    comms::option::app::LengthInfoInterface,\n"
        "    comms::option::app::RefreshInterface,\n"
        "    comms::option::app::NameInterface,\n"
        "    comms::option::app::Handler<#^#MSG_HANDLER#$#>\n"
        ">";

    auto* parentNs = parentNamespace();
    auto& gen = EmscriptenGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"COMMS_CLASS", comms::scopeFor(*this, gen)},
        {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(gen)},
        {"MSG_HANDLER", EmscriptenNamespace::cast(parentNs)->emscriptenHandlerClassName()},
    };
    return util::processTemplate(Templ, repl);
}

std::string EmscriptenInterface::emscriptenSourceFieldsInternal() const
{
    util::StringsList fields;
    for (auto* f : m_emscriptenFields) {
        fields.push_back(f->emscriptenSourceCode());
    }

    return util::strListToString(fields, "\n", "");
}

std::string EmscriptenInterface::emscriptenSourceCodeInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
    };

    util::StringsList fields;
    for (auto* f : m_emscriptenFields) {
        static const std::string Templ = 
            ".function(\"transportField_#^#NAME#$#\", &#^#CLASS_NAME#$#::transportField_#^#NAME#$#_, emscripten::allow_raw_pointers())";

        repl["NAME"] = comms::accessName(f->field().dslObj().parseName());
        fields.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        #^#FIELDS#$#\n"
        "        .function(\"readDataBuf\", &#^#CLASS_NAME#$#::readDataBuf)\n"
        "        .function(\"readJsArray\", &#^#CLASS_NAME#$#::readJsArray)\n"
        "        .function(\"getId\", &#^#CLASS_NAME#$#::getId)\n"
        "        .function(\"refresh\", &#^#CLASS_NAME#$#::refresh)\n"
        "        .function(\"length\", &#^#CLASS_NAME#$#::length)\n"
        "        .function(\"valid\", &#^#CLASS_NAME#$#::valid)\n"
        "        .function(\"name\", &#^#CLASS_NAME#$#::name)\n"
        "        .function(\"dispatch\", &#^#CLASS_NAME#$#::dispatch)\n"
        "        ;\n"
        "}\n";

    repl["FIELDS"] = util::strListToString(fields, "\n", "");
    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten
