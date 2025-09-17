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
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cRelHeaderFor(*this);
}

void CMessage::cAddSourceFiles(GenStringsList& sources) const
{
    if (!genIsReferenced()) {
        return;
    }
    
    auto& cGenerator = CGenerator::cCast(genGenerator());
    sources.push_back(cGenerator.cRelSourceFor(*this));
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

std::string CMessage::cStructName() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cStructNameFor(*this);
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
        "#^#CPP_GUARD_BEGIN#$#\n"
        "#^#FIELDS#$#\n"
        "#^#DEF#$#\n"
        "#^#CPP_GUARD_END#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cHeaderIncludesInternal()},
        {"FIELDS", cHeaderFieldsInternal()},
        {"DEF", cHeaderCodeInternal()},
        {"CPP_GUARD_BEGIN", CGenerator::cCppGuardBegin()},
        {"CPP_GUARD_END", CGenerator::cCppGuardEnd()},   
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

bool CMessage::cWriteSourceInternal() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto filePath = cGenerator.cAbsSourceFor(*this);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }       

    auto& logger = cGenerator.genLogger();
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
        {"HEADER", cGenerator.cRelHeaderFor(*this)},
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
    static const std::string Templ = 
        // TODO: fields
        "/// @brief Definition of <b>#^#DISP_NAME#$#</b> message handle.\n"
        "typedef struct #^#NAME#$#_ #^#NAME#$#;\n\n"
        // TODO: extra code
        ;

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
    };
    
    return util::genProcessTemplate(Templ, repl);
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
    return std::string(); // TODO
}

} // namespace commsdsl2c
