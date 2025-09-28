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

#include "CInterface.h"

#include "CGenerator.h"
#include "CNamespace.h"
#include "CMsgId.h"

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

CInterface::CInterface(CGenerator& generator, ParseInterface parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

CInterface::~CInterface() = default;

std::string CInterface::cRelHeader() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cRelHeaderFor(*this);
}

std::string CInterface::cRelCommsDefHeader() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cRelCommsHeaderFor(*this);
}

void CInterface::cAddSourceFiles(GenStringsList& sources) const
{
    if (!cCodeGenerationAllowed()) {
        return;
    }

    auto& cGenerator = CGenerator::cCast(genGenerator());
    sources.push_back(cGenerator.cRelSourceFor(*this));
}

std::string CInterface::cCommsType() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return comms::genScopeFor(*this, cGenerator);
}

bool CInterface::cCodeGenerationAllowed() const
{
    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs);
    return parentNs->cIsSuitableInterface(*this);
}

std::string CInterface::cName() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cNameFor(*this);
}

std::string CInterface::cCommsTypeName() const
{
    return cName() + strings::genCommsNameSuffixStr();
}

const CMsgId* CInterface::cMsgId() const
{
    auto parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* msgId = parentNs->cMsgId();
    assert(msgId != nullptr);
    return msgId;
}

bool CInterface::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_cFields = CField::cTransformFieldsList(genFields());
    return true;
}

bool CInterface::genWriteImpl() const
{
    if (!cCodeGenerationAllowed()) {
        return true;
    }

    return
        cWriteHeaderInternal() &&
        cWriteSourceInternal() &&
        cWriteCommsHeaderInternal();
}

bool CInterface::cWriteHeaderInternal() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto filePath = cGenerator.cAbsHeaderFor(*this);
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

bool CInterface::cWriteSourceInternal() const
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

bool CInterface::cWriteCommsHeaderInternal() const
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
        "using #^#COMMS_NAME#$# =\n"
        "    ::#^#SCOPE#$#<\n"
        "        comms::option::app::IdInfoInterface,\n"
        "        comms::option::app::ReadIterator<const uint8_t*>,\n"
        "        comms::option::app::WriteIterator<uint8_t*>,\n"
        "        comms::option::app::ValidCheckInterface,\n"
        "        comms::option::app::LengthInfoInterface,\n"
        "        comms::option::app::RefreshInterface,\n"
        "        comms::option::app::NameInterface\n"
        // TODO: handling
        "    >;\n\n"
        "struct alignas(alignof(#^#COMMS_NAME#$#)) #^#NAME#$#_ {};\n\n"
        "inline const #^#COMMS_NAME#$#* fromInterfaceHandle(const #^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline #^#COMMS_NAME#$#* fromInterfaceHandle(#^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline const #^#NAME#$#* toInterfaceHandle(const #^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#NAME#$#*>(from);\n"
        "}\n\n"
        "inline #^#NAME#$#* toInterfaceHandle(#^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#NAME#$#*>(from);\n"
        "}\n"
    ;

    GenStringsList includes = {
        "<stdint.h>",
        "comms/options.h",
        comms::genRelHeaderPathFor(*this, cGenerator),
        cRelHeader(),
    };

    GenStringsList fieldsCode;
    for (auto* f : m_cFields) {
        f->cAddCommsHeaderIncludes(includes);
        fieldsCode.push_back(f->cCommsHeaderCode());
    }

    comms::genPrepareIncludeStatement(includes);

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"HEADER", cGenerator.cRelHeaderFor(*this)},
        {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
        {"FIELDS", util::genStrListToString(fieldsCode, "\n", "\n")},
        {"SCOPE", comms::genScopeFor(*this, cGenerator)},
        {"COMMS_NAME", cCommsTypeName()},
        {"NAME", cName()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string CInterface::cHeaderIncludesInternal() const
{
    util::GenStringsList includes;

    for (auto* f : m_cFields) {
        f->cAddHeaderIncludes(includes);
    }

    auto* msgId = cMsgId();
    assert(msgId != nullptr);
    includes.push_back(msgId->cRelHeader());

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CInterface::cHeaderFieldsInternal() const
{
    util::GenStringsList fields;
    for (auto* f : m_cFields) {
        fields.push_back(f->cHeaderCode());
    }

    return util::genStrListToString(fields, "\n", "\n");
}

std::string CInterface::cHeaderCodeInternal() const
{
    util::GenStringsList fieldsAcc;
    for (auto* f : m_cFields) {
        static const std::string AccTempl =
            "/// @brief Access to inner @ref #^#HANDLE#$# field.\n"
            "#^#HANDLE#$#* #^#NAME#$#_transportField_#^#FIELD_ACC#$#(#^#NAME#$#* msg);\n"
            ;

        util::GenReplacementMap accRepl = {
            {"HANDLE", f->cName()},
            {"NAME", cName()},
            {"FIELD_ACC", comms::genAccessName(f->cGenField().genName())}
        };

        fieldsAcc.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    static const std::string Templ =
        "/// @brief Definition of common interface handle for all the messages.\n"
        "typedef struct #^#NAME#$#_ #^#NAME#$#;\n\n"
        "#^#FIELDS_ACC#$#\n"
        "/// @brief Get message ID\n"
        "#^#MSG_ID#$# #^#NAME#$#_getId(const #^#NAME#$#* msg);\n"
        // TODO: extra code
        ;

    auto* msgId = cMsgId();
    assert(msgId != nullptr);

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"FIELDS_ACC", util::genStrListToString(fieldsAcc, "", "\n")},
        {"MSG_ID", msgId->cName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CInterface::cSourceIncludesInternal() const
{
    // auto& cGenerator = CGenerator::cCast(genGenerator());

    util::GenStringsList includes = {
        cRelCommsDefHeader(),
    };

    for (auto* f : m_cFields) {
        f->cAddSourceIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CInterface::cSourceFieldsInternal() const
{
    util::GenStringsList fields;
    for (auto* f : m_cFields) {
        fields.push_back(f->cSourceCode());
    }

    return util::genStrListToString(fields, "\n", "");
}

std::string CInterface::cSourceCodeInternal() const
{
    util::GenStringsList fieldsAcc;
    for (auto* f : m_cFields) {
        static const std::string AccTempl =
            "#^#HANDLE#$#* #^#NAME#$#_field_#^#FIELD_ACC#$#(#^#NAME#$#* msg)\n"
            "{\n"
            "    return to#^#CONV_SUFFIX#$#(&(fromInterfaceHandle(msg)->transportField_#^#FIELD_ACC#$#()));\n"
            "}\n"
            ;

        util::GenReplacementMap accRepl = {
            {"HANDLE", f->cName()},
            {"NAME", cName()},
            {"FIELD_ACC", comms::genAccessName(f->cGenField().genName())},
            {"CONV_SUFFIX", f->cConversionSuffix()},
        };

        fieldsAcc.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    static const std::string Templ =
        "#^#FIELDS_ACC#$#\n"
        "#^#MSG_ID#$# #^#NAME#$#_getId(const #^#NAME#$#* msg)\n"
        "{\n"
        "    return static_cast<#^#MSG_ID#$#>(fromInterfaceHandle(msg)->getId());\n"
        "}\n"
        // TODO: extra code
        ;

    auto* msgId = cMsgId();
    assert(msgId != nullptr);

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"FIELDS_ACC", util::genStrListToString(fieldsAcc, "", "\n")},
        {"MSG_ID", msgId->cName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
