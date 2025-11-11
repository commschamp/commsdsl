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

#include "CErrorStatus.h"
#include "CGenerator.h"
#include "CInterface.h"
#include "CMsgId.h"
#include "CNamespace.h"
#include "CProtocolOptions.h"
#include "CVersion.h"

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
    if (!genIsReferenced()) {
        return strings::genEmptyString();
    }

    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cRelHeaderFor(*this);
}

std::string CMessage::cRelCommsHeader() const
{
    if (!genIsReferenced()) {
        return strings::genEmptyString();
    }

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
        str += '<' + CProtocolOptions::cName(cGenerator) + '>';
    }
    return str;
}

std::string CMessage::cName() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cNameFor(*this);
}

std::string CMessage::cCommsTypeName() const
{
    return cName() + strings::genCommsNameSuffixStr();
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
    auto* msgId = interface->cMsgId();
    assert(msgId != nullptr);

    util::GenStringsList includes = {
        "<stddef.h>",
        "<stdint.h>",
        CErrorStatus::cRelHeader(CGenerator::cCast(genGenerator())),
        interface->cRelHeader(),
        msgId->cRelHeader(),
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
            {"HANDLE", f->cName()},
            {"NAME", cName()},
            {"FIELD_ACC", comms::genAccessName(f->cGenField().genName())}
        };

        fieldsAcc.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    static const std::string Templ =
        "/// @brief Definition of <b>#^#DISP_NAME#$#</b> message handle.\n"
        "typedef struct #^#NAME#$#_ #^#NAME#$#;\n\n"
        "/// @brief Access to common interface handle.\n"
        "/// @details Internally performs upcasting to interface class.\n"
        "#^#INTERFACE#$#* #^#NAME#$#_toInterface(#^#NAME#$#* msg);\n"
        "\n"
        "/// @brief Access the message object.\n"
        "/// @details Internally performs downcasting to message class.\n"
        "#^#NAME#$#* #^#NAME#$#_fromInterface(#^#INTERFACE#$#* msg);\n"
        "\n"
        "#^#FIELDS_ACC#$#\n"
        "/// @brief Get message ID.\n"
        "#^#MSG_ID#$# #^#NAME#$#_doGetId(const #^#NAME#$#* msg);\n"
        "\n"
        "/// @brief Read the message contents from input buffer.\n"
        "/// @param[in, out] msg Handle of the @ref #^#NAME#$# message.\n"
        "/// @param[in, out] iter Pointer to bufer iterator.\n"
        "/// @param[in] bufLen Remaining bytes in the input buffer.\n"
        "/// @post The iterator is advanced by amount of consumed bytes.\n"
        "/// @return Status of the read operation.\n"
        "#^#ERROR_STATUS#$# #^#NAME#$#_doRead(#^#NAME#$#* msg, const uint8_t** iter, size_t bufLen);\n"
        "\n"
        "/// @brief Write the message contents into output buffer.\n"
        "/// @param[in] msg Handle of the @ref #^#NAME#$# message.\n"
        "/// @param[in, out] iter Pointer to bufer iterator.\n"
        "/// @param[in] bufLen Available bytes in the output buffer.\n"
        "/// @post The iterator is advanced by amount of written bytes.\n"
        "/// @return Status of the write operation.\n"
        "#^#ERROR_STATUS#$# #^#NAME#$#_doWrite(const #^#NAME#$#* msg, uint8_t** iter, size_t bufLen);\n"
        "\n"
        "/// @brief Retrieve serialization length of the @ref #^#NAME#$# message.\n"
        "size_t #^#NAME#$#_doLength(const #^#NAME#$#* msg);\n"
        "\n"
        "/// @brief Retrieve name of the @ref #^#NAME#$# message.\n"
        "const char* #^#NAME#$#_doName(const #^#NAME#$#* msg);\n"
        "\n"
        "/// @brief Check if message @ref #^#NAME#$# contents are valid.\n"
        "bool #^#NAME#$#_doValid(const #^#NAME#$#* msg);\n"
        "\n"
        "/// @brief Dynaically allocate @ref #^#NAME#$# message object.\n"
        "/// @details Use @ref #^#NAME#$#_free() to release message object when it's no longer needed.\n"
        "#^#NAME#$#* #^#NAME#$#_alloc(void);\n"
        "\n"
        "/// @brief Release @ref #^#NAME#$# message object when it's no longer needed.\n"
        "/// @param[in] msg Message object handle returned by the @ref #^#NAME#$#_alloc().\n"
        "void #^#NAME#$#_free(#^#NAME#$#* msg);\n"
        ;

    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    assert (interface != nullptr);
    auto* msgId = interface->cMsgId();
    assert(msgId != nullptr);

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"FIELDS_ACC", util::genStrListToString(fieldsAcc, "\n", "\n")},
        {"INTERFACE", interface->cName()},
        {"MSG_ID", msgId->cName()},
        {"ERROR_STATUS", CErrorStatus::cName(CGenerator::cCast(genGenerator()))},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CMessage::cSourceIncludesInternal() const
{
    util::GenStringsList includes = {
        cRelCommsHeader(),
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
            {"HANDLE", f->cName()},
            {"NAME", cName()},
            {"FIELD_ACC", comms::genAccessName(f->cGenField().genName())},
            {"CONV_SUFFIX", f->cConversionSuffix()},
        };

        fieldsAcc.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    auto* parentNs = CNamespace::cCast(genParentNamespace());
    assert(parentNs != nullptr);
    auto* interface = parentNs->cInterface();
    assert (interface != nullptr);
    auto* msgId = interface->cMsgId();
    assert(msgId != nullptr);

    static const std::string Templ =
        "#^#INTERFACE#$#* #^#NAME#$#_toInterface(#^#NAME#$#* msg)\n"
        "{\n"
        "    return toInterfaceHandle(fromMessageHandle(msg));\n"
        "}\n"
        "\n"
        "#^#NAME#$#* #^#NAME#$#_fromInterface(#^#INTERFACE#$#* msg)\n"
        "{\n"
        "    return toMessageHandle(static_cast<#^#COMMS_NAME#$#*>(fromInterfaceHandle(msg)));\n"
        "}\n"
        "\n"
        "#^#FIELDS_ACC#$#\n"
        "#^#MSG_ID#$# #^#NAME#$#_doGetId(const #^#NAME#$#* msg)\n"
        "{\n"
        "    return static_cast<#^#MSG_ID#$#>(fromMessageHandle(msg)->doGetId());\n"
        "}\n"
        "\n"
        "#^#ERROR_STATUS#$# #^#NAME#$#_doRead(#^#NAME#$#* msg, const uint8_t** iter, size_t bufLen)\n"
        "{\n"
        "    return static_cast<#^#ERROR_STATUS#$#>(fromMessageHandle(msg)->doRead(*iter, bufLen));\n"
        "}\n"
        "\n"
        "#^#ERROR_STATUS#$# #^#NAME#$#_doWrite(const #^#NAME#$#* msg, uint8_t** iter, size_t bufLen)\n"
        "{\n"
        "    return static_cast<#^#ERROR_STATUS#$#>(fromMessageHandle(msg)->doWrite(*iter, bufLen));\n"
        "}\n"
        "\n"
        "size_t #^#NAME#$#_doLength(const #^#NAME#$#* msg)\n"
        "{\n"
        "    return fromMessageHandle(msg)->doLength();\n"
        "}\n"
        "\n"
        "const char* #^#NAME#$#_doName(const #^#NAME#$#* msg)\n"
        "{\n"
        "    return fromMessageHandle(msg)->doName();\n"
        "}\n"
        "\n"
        "bool #^#NAME#$#_doValid(const #^#NAME#$#* msg)\n"
        "{\n"
        "    return fromMessageHandle(msg)->doValid();\n"
        "}\n"
        "\n"
        "#^#NAME#$#* #^#NAME#$#_alloc(void)\n"
        "{\n"
        "    return toMessageHandle(new #^#COMMS_NAME#$#);\n"
        "}\n"
        "\n"
        "void #^#NAME#$#_free(#^#NAME#$#* msg)\n"
        "{\n"
        "    delete fromMessageHandle(msg);\n"
        "}\n"
        ;

    auto parseObj = genParseObj();
    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"FIELDS_ACC", util::genStrListToString(fieldsAcc, "\n", "\n")},
        {"INTERFACE", interface->cName()},
        {"MSG_ID", msgId->cName()},
        {"ERROR_STATUS", CErrorStatus::cName(CGenerator::cCast(genGenerator()))},
        {"COMMS_NAME", cCommsTypeName()},
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
    auto* msgHandler = interface->cMsgHandler();
    assert(msgHandler != nullptr);

    GenStringsList includes = {
        "<stdint.h>",
        "comms/options.h",
        comms::genRelHeaderPathFor(*this, cGenerator),
        cRelHeader(),
        interface->cRelCommsHeader(),
        msgHandler->cRelCommsHeader(),
        CProtocolOptions::cRelHeader(cGenerator),
        CVersion::cRelCommsHeader(cGenerator),
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
        //"class #^#COMMS_NAME#$# : public ::#^#COMMS_TYPE#$#<#^#INTERFACE#$#, #^#OPTS#$#> {};\n"
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
        {"NAME", cName()},
        {"INTERFACE", interface->cCommsTypeName()},
        {"OPTS", CProtocolOptions::cName(cGenerator)},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
