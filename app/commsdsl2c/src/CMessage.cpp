//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CMessage.h"

#include "CGenerator.h"
// #include "CInterface.h"
#include "CNamespace.h"
#include "CProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{


CMessage::CMessage(CGenerator& generator, ParseMessage parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}   

CMessage::~CMessage() = default;

std::string CMessage::cRelHeader() const
{
    auto& gen = CGenerator::cCast(genGenerator());
    return gen.cRelHeaderFor(*this);
}

void CMessage::cAddSourceFiles(GenStringsList& sources) const
{
    if (!genIsReferenced()) {
        return;
    }
    
    auto& gen = CGenerator::cCast(genGenerator());
    sources.push_back(gen.cRelSourceFor(*this));
}

std::string CMessage::cCommsType(bool appendOptions) const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto str = comms::genScopeFor(*this, cGenerator);
    if (appendOptions) {
        str += '<' + CProtocolOptions::cClassName(cGenerator) + '>';
    }    
    return str;
}

bool CMessage::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    assert(genIsReferenced());

    m_cFields = CField::cTransformFieldsList(genFields());
    return true;
}

bool CMessage::genWriteImpl() const
{
    assert(genIsReferenced());
    return 
        cWriteHeaderInternal() &&
        cWriteSourceInternal();
}

bool CMessage::cWriteHeaderInternal() const
{
    auto& gen = CGenerator::cCast(genGenerator());
    auto filePath = gen.cAbsHeaderFor(*this);
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
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cHeaderIncludesInternal()},
        {"FIELDS", cHeaderFieldsInternal()},
        {"DEF", cHeaderCodeInternal()},
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

bool CMessage::cWriteSourceInternal() const
{
    auto& gen = CGenerator::cCast(genGenerator());
    auto filePath = gen.cAbsSourceFor(*this);
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
        "#^#INCLUDES#$#\n"
        "#^#FIELDS#$#\n"
        "#^#CODE#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"HEADER", gen.cRelHeaderFor(*this)},
        {"INCLUDES", cSourceIncludesInternal()},
        {"FIELDS", cSourceFieldsInternal()},
        {"CODE", cSourceCodeInternal()},
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

std::string CMessage::cHeaderIncludesInternal() const
{
    // auto& gen = CGenerator::cCast(genGenerator());
    // TODO: interface
    // auto* iFace = gen.cMainInterface();
    // assert(iFace != nullptr);
    // auto* parentNs = iFace->genParentNamespace();
    
    util::GenStringsList includes;

    for (auto* f : m_cFields) {
        f->cAddHeaderIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CMessage::cHeaderFieldsInternal() const
{
    util::GenStringsList fields;
    for (auto* f : m_cFields) {
        fields.push_back(f->cHeaderCode());
    }

    return util::genStrListToString(fields, "\n", "\n");
}

std::string CMessage::cHeaderCodeInternal() const
{
    // auto& gen = CGenerator::cCast(genGenerator());
    // util::GenStringsList fields;
    // for (auto* f : m_cFields) {
    //     static const std::string Templ = 
    //         "using Base::field_#^#NAME#$#;\n"
    //         "#^#FIELD_CLASS#$#* field_#^#NAME#$#_()\n"
    //         "{\n"
    //         "    return static_cast<#^#FIELD_CLASS#$#*>(&field_#^#NAME#$#());\n"
    //         "}\n";

    //     util::GenReplacementMap repl = {
    //         {"FIELD_CLASS", gen.cClassName(f->cGenField())},
    //         {"NAME", comms::genAccessName(f->cGenField().genParseObj().parseName())},
    //     };

    //     fields.push_back(util::genProcessTemplate(Templ, repl));
    // }

    // static const std::string Templ =
    //     "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>\n"
    //     "{\n"
    //     "    using Base = #^#COMMS_CLASS#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>;\n"
    //     "public:\n"
    //     "    #^#CLASS_NAME#$#() = default;\n"
    //     "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = default;\n"
    //     "    virtual ~#^#CLASS_NAME#$#() = default;\n\n"
    //     "    #^#FIELDS#$#\n"
    //     "};\n\n"
    //     "inline bool eq_#^#CLASS_NAME#$#(const #^#CLASS_NAME#$#& first, const #^#CLASS_NAME#$#& second)\n"
    //     "{\n"
    //     "    return first == second;\n"
    //     "}\n"
    //     ;

    // auto* iFace = gen.cMainInterface();
    // assert(iFace != nullptr);

    // util::GenReplacementMap repl = {
    //     {"CLASS_NAME", gen.cClassName(*this)},
    //     {"COMMS_CLASS", comms::genScopeFor(*this, gen)},
    //     {"INTERFACE", gen.cClassName(*iFace)},
    //     {"FIELDS", util::genStrListToString(fields, "\n", "")}
    // };

    // if (CProtocolOptions::cIsDefined(gen)) {
    //     repl["PROT_OPTS"] = ", " + CProtocolOptions::cClassName(gen);
    // }

    // return util::genProcessTemplate(Templ, repl);
    return std::string(); // TODO
}

std::string CMessage::cSourceIncludesInternal() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    // TODO: interface
    
    util::GenStringsList includes = {
        CProtocolOptions::cRelHeaderPath(cGenerator),
        comms::genRelHeaderPathFor(*this, cGenerator),
    };

    for (auto* f : m_cFields) {
        f->cAddSourceIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CMessage::cSourceFieldsInternal() const
{
    util::GenStringsList fields;
    for (auto* f : m_cFields) {
        fields.push_back(f->cSourceCode());
    }

    return util::genStrListToString(fields, "\n", "");
}

std::string CMessage::cSourceCodeInternal() const
{
    // auto& gen = CGenerator::cCast(genGenerator());

    // util::GenReplacementMap repl = {
    //     {"CLASS_NAME", gen.cClassName(*this)},
    // };

    // util::GenStringsList fields;
    // for (auto* f : m_cFields) {
    //     static const std::string Templ = 
    //         ".function(\"field_#^#NAME#$#\", &#^#CLASS_NAME#$#::field_#^#NAME#$#_, c::allow_raw_pointers())";

    //     repl["NAME"] = comms::genAccessName(f->cGenField().genParseObj().parseName());
    //     fields.push_back(util::genProcessTemplate(Templ, repl));
    // }

    // static const std::string Templ = 
    //     "C_BINDINGS(#^#CLASS_NAME#$#) {\n"
    //     "    c::class_<#^#CLASS_NAME#$#, c::base<#^#INTERFACE#$#> >(\"#^#CLASS_NAME#$#\")\n"
    //     "        .constructor<>()\n"
    //     "        .constructor<const #^#CLASS_NAME#$#&>()\n"
    //     "        #^#FIELDS#$#\n"
    //     "        ;\n"
    //     "    c::function(\"eq_#^#CLASS_NAME#$#\", &eq_#^#CLASS_NAME#$#);\n"
    //     "}\n";

    // auto* iFace = gen.cMainInterface();
    // assert(iFace != nullptr);
    // repl["INTERFACE"] = gen.cClassName(*iFace);
    // repl["FIELDS"] = util::genStrListToString(fields, "\n", "");
    // return util::genProcessTemplate(Templ, repl);

    return std::string(); // TODO
}

} // namespace commsdsl2c
