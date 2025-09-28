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

#include "EmscriptenMessage.h"

#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"
#include "EmscriptenNamespace.h"
#include "EmscriptenProtocolOptions.h"

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

EmscriptenMessage::EmscriptenMessage(EmscriptenGenerator& generator, ParseMessage parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

EmscriptenMessage::~EmscriptenMessage() = default;

std::string EmscriptenMessage::emscriptenRelHeader() const
{
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    return gen.emscriptenRelHeaderFor(*this);
}

void EmscriptenMessage::emscriptenAddSourceFiles(GenStringsList& sources) const
{
    if (!genIsReferenced()) {
        return;
    }

    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    sources.push_back(gen.emscriptenRelSourceFor(*this));
}

bool EmscriptenMessage::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    assert(genIsReferenced());

    m_emscriptenFields = EmscriptenField::emscriptenTransformFieldsList(genFields());
    return true;
}

bool EmscriptenMessage::genWriteImpl() const
{
    assert(genIsReferenced());
    return
        emscriptenWriteHeaderInternal() &&
        emscriptenWriteSourceInternal();
}

bool EmscriptenMessage::emscriptenWriteHeaderInternal() const
{
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    auto filePath = gen.emscriptenAbsHeaderFor(*this);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    auto& logger = gen.genLogger();
    logger.genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n\n"
        "#^#FIELDS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::emscriptenFileGeneratedComment()},
        {"INCLUDES", emscriptenHeaderIncludesInternal()},
        {"FIELDS", emscriptenHeaderFieldsInternal()},
        {"DEF", emscriptenHeaderClassInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool EmscriptenMessage::emscriptenWriteSourceInternal() const
{
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    auto filePath = gen.emscriptenAbsSourceFor(*this);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    auto& logger = gen.genLogger();
    logger.genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#include <emscripten/bind.h>\n\n"
        "#^#FIELDS#$#\n"
        "#^#CODE#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::emscriptenFileGeneratedComment()},
        {"HEADER", gen.emscriptenRelHeaderFor(*this)},
        {"FIELDS", emscriptenSourceFieldsInternal()},
        {"CODE", emscriptenSourceCodeInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string EmscriptenMessage::emscriptenHeaderIncludesInternal() const
{
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);
    auto* parentNs = iFace->genParentNamespace();

    util::GenStringsList includes = {
        comms::genRelHeaderPathFor(*this, gen),
        iFace->emscriptenRelHeader(),
        EmscriptenNamespace::emscriptenCast(parentNs)->emscriptenHandlerRelHeader(),
    };

    EmscriptenProtocolOptions::emscriptenAddInclude(gen, includes);

    if (!m_emscriptenFields.empty()) {
        includes.push_back("<iterator>");
        includes.push_back(EmscriptenDataBuf::emscriptenRelHeader(gen));
    }

    for (auto* f : m_emscriptenFields) {
        f->emscriptenHeaderAddExtraIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "");
}

std::string EmscriptenMessage::emscriptenHeaderFieldsInternal() const
{
    util::GenStringsList fields;
    for (auto* f : m_emscriptenFields) {
        fields.push_back(f->emscriptenHeaderClass());
    }

    return util::genStrListToString(fields, "\n", "\n");
}

std::string EmscriptenMessage::emscriptenHeaderClassInternal() const
{
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    util::GenStringsList fields;
    for (auto* f : m_emscriptenFields) {
        static const std::string Templ =
            "using Base::field_#^#NAME#$#;\n"
            "#^#FIELD_CLASS#$#* field_#^#NAME#$#_()\n"
            "{\n"
            "    return static_cast<#^#FIELD_CLASS#$#*>(&field_#^#NAME#$#());\n"
            "}\n";

        util::GenReplacementMap repl = {
            {"FIELD_CLASS", gen.emscriptenClassName(f->emscriptenGenField())},
            {"NAME", comms::genAccessName(f->emscriptenGenField().genParseObj().parseName())},
        };

        fields.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>;\n"
        "public:\n"
        "    #^#CLASS_NAME#$#() = default;\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = default;\n"
        "    virtual ~#^#CLASS_NAME#$#() = default;\n\n"
        "    #^#FIELDS#$#\n"
        "};\n\n"
        "inline bool eq_#^#CLASS_NAME#$#(const #^#CLASS_NAME#$#& first, const #^#CLASS_NAME#$#& second)\n"
        "{\n"
        "    return first == second;\n"
        "}\n"
        ;

    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::GenReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
        {"COMMS_CLASS", comms::genScopeFor(*this, gen)},
        {"INTERFACE", gen.emscriptenClassName(*iFace)},
        {"FIELDS", util::genStrListToString(fields, "\n", "")}
    };

    if (EmscriptenProtocolOptions::emscriptenIsDefined(gen)) {
        repl["PROT_OPTS"] = ", " + EmscriptenProtocolOptions::emscriptenClassName(gen);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenMessage::emscriptenSourceFieldsInternal() const
{
    util::GenStringsList fields;
    for (auto* f : m_emscriptenFields) {
        fields.push_back(f->emscriptenSourceCode());
    }

    return util::genStrListToString(fields, "\n", "");
}

std::string EmscriptenMessage::emscriptenSourceCodeInternal() const
{
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());

    util::GenReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
    };

    util::GenStringsList fields;
    for (auto* f : m_emscriptenFields) {
        static const std::string Templ =
            ".function(\"field_#^#NAME#$#\", &#^#CLASS_NAME#$#::field_#^#NAME#$#_, emscripten::allow_raw_pointers())";

        repl["NAME"] = comms::genAccessName(f->emscriptenGenField().genParseObj().parseName());
        fields.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#, emscripten::base<#^#INTERFACE#$#> >(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$#&>()\n"
        "        #^#FIELDS#$#\n"
        "        ;\n"
        "    emscripten::function(\"eq_#^#CLASS_NAME#$#\", &eq_#^#CLASS_NAME#$#);\n"
        "}\n";

    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);
    repl["INTERFACE"] = gen.emscriptenClassName(*iFace);
    repl["FIELDS"] = util::genStrListToString(fields, "\n", "");
    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten
