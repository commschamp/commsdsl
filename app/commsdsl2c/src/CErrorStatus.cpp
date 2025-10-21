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

#include "CErrorStatus.h"

#include "CGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <cassert>
#include <fstream>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2c
{

std::string CErrorStatus::cName(const CGenerator& generator)
{
    return generator.genProtocolSchema().genMainNamespace() + "_ErrorStatus";
}

std::string CErrorStatus::cRelHeader(const CGenerator& generator)
{
    return generator.cRelRootHeaderFor(cName(generator));
}

std::string CErrorStatus::cRelSourcePath(const CGenerator& generator)
{
    return generator.cRelRootSourceFor(cName(generator));
}

void CErrorStatus::cAddSourceFiles(const CGenerator& generator, GenStringsList& sources)
{
    sources.push_back(cRelSourcePath(generator));
}

bool CErrorStatus::cWrite(CGenerator& generator)
{
    CErrorStatus obj(generator);
    return
        obj.cWriteHeaderInternal() &&
        obj.cWriteSourceInternal();
}

CErrorStatus::CErrorStatus(CGenerator& generator) :
    m_cGenerator(generator)
{
}

bool CErrorStatus::cWriteHeaderInternal()
{
    auto filePath = m_cGenerator.cAbsRootHeaderFor(cName(m_cGenerator));
    m_cGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n"
        "\n"
        "#^#CPP_GUARD_BEGIN#$#\n"
        "\n"
        "/// @brief Status report equivalent to the @b comms::ErrorStatus.\n"
        "typedef enum\n"
        "{\n"
        "    #^#NAME#$#_Success, ///< Used to indicate successful outcome of the operation.\n"
        "    #^#NAME#$#_UpdateRequired, ///< Used to indicate that write operation wasn't complete.\n"
        "    #^#NAME#$#_NotEnoughData, ///< Used to indicate that stream buffer didn't contain enough data to complete read operation.\n"
        "    #^#NAME#$#_ProtocolError, ///< Used to indicate that any of the used protocol layers encountered an error while processing the data.\n"
        "    #^#NAME#$#_BufferOverflow, ///< Used to indicate that stream buffer was overflowed when attempting to write data.\n"
        "    #^#NAME#$#_InvalidMsgId, ///< Used to indicate that received message has unknown id.\n"
        "    #^#NAME#$#_InvalidMsgData, ///< Used to indicate that a message has invalid data.\n"
        "    #^#NAME#$#_MsgAllocFailure, ///< Used to indicate that message allocation has failed.\n"
        "    #^#NAME#$#_NotSupported, ///< The operation is not supported.\n"
        "    #^#NAME#$#_NumOfErrorStatuses, ///< Number of supported error statuses, must be last.\n"
        "    #^#NAME#$#_ValuesLimit = 255 ///< Allow unknown values beyond @b #^#NAME#$#_NumOfErrorStatuses.\n"
        "} #^#NAME#$#;\n"
        "\n"
        "#^#CPP_GUARD_END#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"NAME", cName(m_cGenerator)},
        {"CPP_GUARD_BEGIN", CGenerator::cCppGuardBegin(false)},
        {"CPP_GUARD_END", CGenerator::cCppGuardEnd()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool CErrorStatus::cWriteSourceInternal()
{
    auto filePath = m_cGenerator.cAbsRootSourceFor(cName(m_cGenerator));
    m_cGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#include \"comms/ErrorStatus.h\"\n\n"
        "namespace\n"
        "{\n"
        "\n"
        "template <typename T>"
        "constexpr int asInt(T val)\n"
        "{\n"
        "    return static_cast<int>(val);\n"
        "}\n"
        "\n"
        "}\n\n"
        "static_assert(asInt(#^#NAME#$#_Success) == asInt(comms::ErrorStatus::Success), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_UpdateRequired) == asInt(comms::ErrorStatus::UpdateRequired), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_NotEnoughData) == asInt(comms::ErrorStatus::NotEnoughData), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_ProtocolError) == asInt(comms::ErrorStatus::ProtocolError), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_BufferOverflow) == asInt(comms::ErrorStatus::BufferOverflow), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_InvalidMsgId) == asInt(comms::ErrorStatus::InvalidMsgId), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_InvalidMsgData) == asInt(comms::ErrorStatus::InvalidMsgData), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_MsgAllocFailure) == asInt(comms::ErrorStatus::MsgAllocFailure), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_NotSupported) == asInt(comms::ErrorStatus::NotSupported), \"Invalid map\");\n"
        "static_assert(asInt(#^#NAME#$#_NumOfErrorStatuses) <= asInt(comms::ErrorStatus::NumOfErrorStatuses), \"Invalid map\");\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"NAME", cName(m_cGenerator)},
        {"HEADER", cRelHeader(m_cGenerator)},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2c