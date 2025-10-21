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

#include "CFrame.h"

#include "CGenerator.h"
#include "CMsgId.h"
#include "CNamespace.h"
#include "CProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

CFrame::CFrame(CGenerator& generator, ParseFrame parseObj, commsdsl::gen::GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

CFrame::~CFrame() = default;

std::string CFrame::cRelHeader() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cRelHeaderFor(*this);
}

std::string CFrame::cRelCommsHeader() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cRelCommsHeaderFor(*this);
}

void CFrame::cAddSourceFiles(GenStringsList& sources) const
{
    if (!m_validFrame) {
        return;
    }

    auto& cGenerator = CGenerator::cCast(genGenerator());
    sources.push_back(cGenerator.cRelSourceFor(*this));
}

std::string CFrame::cCommsType(bool appendOptions) const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto str = comms::genScopeFor(*this, cGenerator);
    if (appendOptions) {
        auto* iFace = cInterfaceInternal();
        assert(iFace != nullptr);
        auto* ns = CNamespace::cCast(genParentNamespace());
        assert(ns != nullptr);
        auto* input = ns->cInputMessages();
        assert(input != nullptr);
        str += '<' + iFace->cCommsTypeName() + ", " + input->cName() + ", " + CProtocolOptions::cName(cGenerator) + '>';
    }
    return str;
}

std::string CFrame::cName() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cNameFor(*this);
}

std::string CFrame::cCommsTypeName() const
{
    return cName() + strings::genCommsNameSuffixStr();
}

bool CFrame::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    bool success = true;
    auto reorderedLayers = getCommsOrderOfLayers(success);
    if (!success) {
        return false;
    }

    for (auto* l : reorderedLayers) {
        auto* cLayer = CLayer::cCast(l);
        assert(cLayer != nullptr);
        m_cLayers.push_back(const_cast<CLayer*>(cLayer));
    }

    auto* iFace = cInterfaceInternal();
    do {
        if (iFace == nullptr) {
            break;
        }

        m_validFrame =
            std::all_of(
                m_cLayers.begin(), m_cLayers.end(),
                [iFace](auto* l)
                {
                    return l->cIsInterfaceSupported(*iFace);
                });
    } while (false);
    return true;
}

bool CFrame::genWriteImpl() const
{
    if (!m_validFrame) {
        return true;
    }

    return
        cWriteHeaderInternal() &&
        cWriteSourceInternal() &&
        cWriteCommsHeaderInternal();
}

bool CFrame::cWriteHeaderInternal() const
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
        "#^#INCLUDES#$#\n"
        "#^#CPP_GUARD_BEGIN#$#\n"
        "#^#LAYERS#$#\n"
        "#^#FRAME#$#\n"
        "#^#CPP_GUARD_END#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cHeaderIncludesInternal()},
        {"LAYERS", cHeaderLayersCodeInternal()},
        {"CPP_GUARD_BEGIN", CGenerator::cCppGuardBegin()},
        {"CPP_GUARD_END", CGenerator::cCppGuardEnd()}, 
        {"FRAME", cHeaderFrameCodeInternal()},       
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool CFrame::cWriteSourceInternal() const
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
        "#^#GENERATED#$#\n\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#^#INCLUDES#$#\n"
        "#^#LAYERS#$#\n"
        "#^#FRAME#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"HEADER", cRelHeader()},
        {"INCLUDES", cSourceIncludesInternal()},
        {"LAYERS", cSourceLayersCodeInternal()},
        {"FRAME", cSourceFrameCodeInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool CFrame::cWriteCommsHeaderInternal() const
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
        "#^#GENERATED#$#\n\n"
        "#^#INCLUDES#$#\n"
        "#^#LAYERS#$#\n"
        "#^#FRAME#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cCommsHeaderIncludesInternal()},
        {"LAYERS", cCommsHeaderLayersCodeInternal()},
        {"FRAME", cCommsHeaderFrameCodeInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string CFrame::cHeaderIncludesInternal() const
{
    auto* iFace = cInterfaceInternal();
    assert(iFace != nullptr);

    GenStringsList includes = {
        "<stddef.h>",
        "<stdint.h>",
        iFace->cRelHeader(),
    };

    for (auto* l : m_cLayers) {
        l->cAddHeaderIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CFrame::cHeaderLayersCodeInternal() const
{
    GenStringsList code;
    for (auto* l : m_cLayers) {
        code.push_back(l->cHeaderCode());
    }

    return util::genStrListToString(code, "\n", "\n");
}

std::string CFrame::cHeaderFrameCodeInternal() const
{
    static const std::string Templ = 
        "/// @brief Definition of <b>#^#DISP_NAME#$#</b> frame.\n"
        "typedef struct #^#NAME#$#_ #^#NAME#$#;\n"
        "\n"
        "/// @brief Dynamically allocate @ref #^#NAME#$# object.\n"
        "/// @details Use @ref #^#NAME#$#_free() to de-allocate it.\n"
        "#^#NAME#$#* #^#NAME#$#_alloc(void);\n"
        "\n"
        "/// @brief Free frame object allocated with @ref #^#NAME#$#_alloc().\n"
        "/// @details Use @ref #^#NAME#$#_free() to de-allocate it.\n"
        "void #^#NAME#$#_free(#^#NAME#$#* frame);\n"        
        "\n"
        "/// @brief Create message object with factory used to create message objects when processing input data.\n"
        "/// @param[in] frame Frame object handle.\n"
        "/// @param[in] msgId Numeric message ID.\n"
        "/// @param[in] idx Index (offset) of the message type among the ones sharing the same numeric ID.\n"
        "///     Use @b 0 when there is only one type of message per ID.\n"
        "/// @return Allocated message object, use @ref #^#NAME#$#_deleteMsg() to delete it and prevent memory leaks.\n"
        "#^#INTERFACE#$#* #^#NAME#$#_createMsg(#^#NAME#$#* frame, #^#MSG_ID#$# msgId, unsigned idx);\n"
        "\n"
        "/// @brief Delete message object allocated using @ref #^#NAME#$#_createMsg().\n"
        "void #^#NAME#$#_deleteMsg(#^#INTERFACE#$#* msg);\n"
        ;  
    
    auto parseObj = genParseObj();
    auto* iFace = cInterfaceInternal();
    assert(iFace != nullptr);
    
    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"INTERFACE", iFace->cName()},
        {"MSG_ID", cMsgIdTypeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CFrame::cSourceIncludesInternal() const
{
    GenStringsList includes = {
        cRelCommsHeader(),
    };

    for (auto* l : m_cLayers) {
        l->cAddSourceIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CFrame::cSourceLayersCodeInternal() const
{
    GenStringsList code;
    for (auto* l : m_cLayers) {
        code.push_back(l->cSourceCode());
    }

    return util::genStrListToString(code, "\n", "\n");
}

std::string CFrame::cSourceFrameCodeInternal() const
{
    static const std::string Templ = 
        "#^#NAME#$#* #^#NAME#$#_alloc(void)\n"
        "{\n"
        "    return toFrameHandle(new #^#COMMS_NAME#$#);\n"
        "}\n"
        "\n"
        "void #^#NAME#$#_free(#^#NAME#$#* frame)\n"
        "{\n"
        "    delete fromFrameHandle(frame);\n"
        "}\n"            
        "\n"
        "#^#INTERFACE#$#* #^#NAME#$#_createMsg(#^#NAME#$#* frame, #^#MSG_ID#$# msgId, unsigned idx)\n"
        "{\n"
        "    return toInterfaceHandle(fromFrameHandle(frame)->createMsg(static_cast<#^#INTERFACE_COMMS#$#::MsgIdType>(msgId), idx).release());\n"
        "}\n" 
        "\n"
        "void #^#NAME#$#_deleteMsg(#^#INTERFACE#$#* msg)\n"
        "{\n"
        "    #^#COMMS_NAME#$#::MsgPtr ptr(fromInterfaceHandle(msg)); // delete on destruct\n"
        "}\n"            
        ;  
    
    auto parseObj = genParseObj();
    auto* iFace = cInterfaceInternal();
    assert(iFace != nullptr);
    
    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"DISP_NAME", util::genDisplayName(parseObj.parseDisplayName(), parseObj.parseName())},
        {"INTERFACE", iFace->cName()},
        {"MSG_ID", cMsgIdTypeInternal()},
        {"COMMS_NAME", cCommsTypeName()},
        {"INTERFACE_COMMS", iFace->cCommsTypeName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CFrame::cCommsHeaderIncludesInternal() const
{
    auto* iFace = cInterfaceInternal();
    assert(iFace != nullptr);

    auto* ns = CNamespace::cCast(genParentNamespace());
    assert(ns != nullptr);
    auto* input = ns->cInputMessages();
    assert(input != nullptr);
    auto* msgHandler = ns->cMsgHandler();
    assert(msgHandler != nullptr);

    GenStringsList includes {
        cRelHeader(),
        comms::genRelHeaderPathFor(*this, genGenerator()),
        iFace->cRelCommsHeader(),
        msgHandler->cRelCommsHeader(),
        input->cRelHeader(),
    };

    for (auto* l : m_cLayers) {
        l->cAddCommsHeaderIncludes(includes);
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CFrame::cCommsHeaderLayersCodeInternal() const
{
    auto* iFace = cInterfaceInternal();
    assert(iFace != nullptr);

    bool hasInputMessages = false; // gets updated in the loop below
    GenStringsList code;
    for (auto iter = m_cLayers.rbegin(); iter != m_cLayers.rend(); ++iter) {
        auto* l = *iter;
        code.push_back(l->cCommsHeaderCode(*iFace, hasInputMessages));
    }

    return util::genStrListToString(code, "\n", "\n");
}

std::string CFrame::cCommsHeaderFrameCodeInternal() const
{
    static const std::string Templ =
        "using #^#COMMS_NAME#$# = ::#^#COMMS_TYPE#$#;\n"
        "struct alignas(alignof(#^#COMMS_NAME#$#)) #^#NAME#$#_ {};\n"
        "\n"
        "inline const #^#COMMS_NAME#$#* fromFrameHandle(const #^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline #^#COMMS_NAME#$#* fromFrameHandle(#^#NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#COMMS_NAME#$#*>(from);\n"
        "}\n\n"
        "inline const #^#NAME#$#* toFrameHandle(const #^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<const #^#NAME#$#*>(from);\n"
        "}\n\n"
        "inline #^#NAME#$#* toFrameHandle(#^#COMMS_NAME#$#* from)\n"
        "{\n"
        "    return reinterpret_cast<#^#NAME#$#*>(from);\n"
        "}\n" 
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"COMMS_NAME", cCommsTypeName()},
        {"COMMS_TYPE", cCommsType()},
    };

    return util::genProcessTemplate(Templ, repl);        
}

const CInterface* CFrame::cInterfaceInternal() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto* parentNs = CNamespace::cCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    return parentNs->cInterface();
}

std::string CFrame::cMsgIdTypeInternal() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto* parentNs = CNamespace::cCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    auto* msgId = parentNs->cMsgId();
    if (msgId != nullptr) {
        return msgId->cName();
    }

    return "unsigned";
}

} // namespace commsdsl2c
