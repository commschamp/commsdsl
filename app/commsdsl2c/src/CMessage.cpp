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
#include "CInterface.h"
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

std::string CMessage::cRelCommsDefHeader() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cRelCommsHeaderFor(*this);
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

std::string CMessage::cCommsTypeName() const
{
    return cStructName() + strings::genCommsNameSuffixStr();
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
    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    if (interface == nullptr) {
        return true;
    }
    
    assert(genIsReferenced());
    return 
        cWriteHeaderInternal() &&
        cWriteSourceInternal() &&
        cWriteCommsHeaderInternal();
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

bool CMessage::cWriteCommsHeaderInternal() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto filePath = cGenerator.cAbsCommsHeaderFor(*this);
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
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#FIELDS#$#\n"
        "#^#CODE#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"HEADER", cGenerator.cRelHeaderFor(*this)},
        {"INCLUDES", cCommsHeaderIncludesInternal()},
        {"FIELDS", cCommsHeaderFieldsInternal()},
        {"CODE", cCommsHeaderCodeInternal()},
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

std::string CMessage::cHeaderIncludesInternal() const
{
    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    assert (interface != nullptr);

    util::GenStringsList includes = {
        interface->cRelHeader()
    };

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
    util::GenStringsList fieldsAcc;
    for (auto* f : m_cFields) {
        static const std::string AccTempl = 
            "/// @brief Access to inner @ref #^#HANDLE#$# field.\n"
            "#^#HANDLE#$#* #^#NAME#$#_field_#^#FIELD_ACC#$#(#^#NAME#$#* msg);\n"
            ;

        util::GenReplacementMap accRepl = {
            {"HANDLE", f->cStructName()},
            {"NAME", cStructName()},
            {"FIELD_ACC", comms::genAccessName(f->cGenField().genName())}
        };

        fieldsAcc.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    static const std::string Templ = 
        "/// @brief Definition of <b>#^#DISP_NAME#$#</b> message handle.\n"
        "typedef struct #^#NAME#$#_ #^#NAME#$#;\n\n"
        "/// @brief Access to common interface handle.\n"
        "#^#INTERFACE#$#* #^#NAME#$#_interface(#^#NAME#$#* msg);\n"
        "\n"
        "#^#FIELDS_ACC#$#\n"
        // TODO: extra code
        ;

    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    assert (interface != nullptr);        

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"FIELDS_ACC", util::genStrListToString(fieldsAcc, "", "\n")},
        {"INTERFACE", interface->cStructName()}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CMessage::cSourceIncludesInternal() const
{
    util::GenStringsList includes = {
        cRelCommsDefHeader(),
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
    util::GenStringsList fieldsAcc;
    for (auto* f : m_cFields) {
        static const std::string AccTempl = 
            "#^#HANDLE#$#* #^#NAME#$#_field_#^#FIELD_ACC#$#(#^#NAME#$#* msg)\n"
            "{\n"
            "    return to#^#CONV_SUFFIX#$#(&(fromMessageHandle(msg)->field_#^#FIELD_ACC#$#()));\n"
            "}\n"
            ;

        util::GenReplacementMap accRepl = {
            {"HANDLE", f->cStructName()},
            {"NAME", cStructName()},
            {"FIELD_ACC", comms::genAccessName(f->cGenField().genName())},
            {"CONV_SUFFIX", f->cConversionSuffix()},
        };

        fieldsAcc.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    assert (interface != nullptr);       

    static const std::string Templ = 
        "#^#INTERFACE#$#* #^#NAME#$#_interface(#^#NAME#$#* msg)\n"
        "{\n"
        "    return toInterfaceHandle(fromMessageHandle(msg));\n"
        "}\n"
        "\n"
        "#^#FIELDS_ACC#$#\n"
        // TODO: extra code
        ;

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
        {"FIELDS_ACC", util::genStrListToString(fieldsAcc, "", "\n")},
        {"INTERFACE", interface->cStructName()},
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CMessage::cCommsHeaderIncludesInternal() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    assert (interface != nullptr);

    GenStringsList includes = {
        "<stdint.h>",
        "comms/options.h",
        comms::genRelHeaderPathFor(*this, cGenerator),
        cRelHeader(),
        interface->cRelCommsDefHeader(),
        CProtocolOptions::cRelHeaderPath(cGenerator),
    };

    for (auto* f : m_cFields) {
        f->cAddCommsHeaderIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CMessage::cCommsHeaderFieldsInternal() const
{
    GenStringsList fieldsCode;
    for (auto* f : m_cFields) {
        fieldsCode.push_back(f->cCommsHeaderCode());
    }

    return util::genStrListToString(fieldsCode, "\n", "\n");
}

std::string CMessage::cCommsHeaderCodeInternal() const
{
    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    assert (interface != nullptr);

    static const std::string Templ = 
        "using #^#COMMS_NAME#$# = ::#^#COMMS_TYPE#$#<#^#INTERFACE#$#, #^#OPTS#$#>;\n"
        "struct alignas(alignof(#^#COMMS_NAME#$#)) #^#NAME#$#_ {};\n\n"
        "inline const #^#COMMS_NAME#$#* fromMessageHandle(const #^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline #^#COMMS_NAME#$#* fromMessageHandle(#^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline const #^#NAME#$#* toMessageHandle(const #^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#NAME#$#*>(from);\n"
        "}\n\n"      
        "inline #^#NAME#$#* toMessageHandle(#^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#NAME#$#*>(from);\n"
        "}\n"      
       ;

    auto& cGenerator = CGenerator::cCast(genGenerator());
    util::GenReplacementMap repl = {
        {"COMMS_TYPE", comms::genScopeFor(*this, cGenerator)},
        {"COMMS_NAME", cCommsTypeName()},
        {"NAME", cStructName()},
        {"INTERFACE", interface->cCommsTypeName()},
        {"OPTS", CProtocolOptions::cClassName(cGenerator)},
    };   
    
    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
