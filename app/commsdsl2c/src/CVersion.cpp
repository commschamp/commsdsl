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

#include "CVersion.h"

#include "CGenerator.h"
#include "CSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

bool CVersion::cWrite(CGenerator& generator)
{
    CVersion obj(generator);
    return
        obj.cWriteHeaderInternal() &&
        obj.cWriteCommsHeaderInternal();
}

std::string CVersion::cRelCommsHeader(const CGenerator& generator)
{
    return generator.cRelRootCommsHeaderFor(strings::genVersionFileNameStr());
}

bool CVersion::cWriteHeaderInternal() const
{
    auto filePath = m_cGenerator.cAbsRootHeaderFor(strings::genVersionFileNameStr());

    m_cGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "/// @brief Version of the protocol specification.\n"
        "#define CC_C_#^#PREFIX#$#_SPEC_VERSION (#^#VERSION#$#)\n\n"
        "#^#CODE_VER_DEFINE#$#\n"
        "#^#APPEND#$#\n";

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"VERSION", util::genNumToString(m_cGenerator.genProtocolSchema().genSchemaVersion())},
        {"PREFIX", util::genStrToUpper(m_cGenerator.cNamesPrefix())},
        {"CODE_VER_DEFINE", cCodeVersionDefineInternal()},
        {"APPEND", util::genReadFileContents(m_cGenerator.cInputAbsRootHeaderFor(strings::genVersionFileNameStr()))},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool CVersion::cWriteCommsHeaderInternal() const
{
    auto filePath = m_cGenerator.cAbsRootCommsHeaderFor(strings::genVersionFileNameStr());

    m_cGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "static_assert(CC_C_#^#PREFIX#$#_SPEC_VERSION == #^#NS#$#_SPEC_VERSION, \"Specification versions mismatch\");\n"
        "#^#CODE_VER_DEFINE#$#\n"
        ;

    util::GenStringsList includes = {
        comms::genRelHeaderForRoot(strings::genVersionFileNameStr(), m_cGenerator),
        m_cGenerator.cRelRootHeaderFor(strings::genVersionFileNameStr()),
    };

    comms::genPrepareIncludeStatement(includes);

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
        {"PREFIX", util::genStrToUpper(m_cGenerator.cNamesPrefix())},
        {"CODE_VER_DEFINE", cCodeVersionCommsHeaderInternal()},
        {"NS", util::genStrToUpper(m_cGenerator.genProtocolSchema().genMainNamespace())},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string CVersion::cCodeVersionDefineInternal() const
{
    auto tokens = m_cGenerator.genGetCodeVersionTokens();
    if (tokens.empty()) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "/// @brief Major version of the protocol library.\n"
        "#define CC_C_#^#PREFIX#$#_MAJOR_VERSION (#^#MAJOR_VERSION#$#)\n\n"
        "/// @brief Minor version of the protocol library.\n"
        "#define CC_C_#^#PREFIX#$#_MINOR_VERSION (#^#MINOR_VERSION#$#)\n\n"
        "/// @brief Patch version of the protocol library.\n"
        "#define CC_C_#^#PREFIX#$#_PATCH_VERSION (#^#PATCH_VERSION#$#)\n\n"
        "/// @brief Make version as a single number.\n"
        "#define CC_C_#^#PREFIX#$#_MAKE_VERSION(major_, minor_, patch_) \\\n"
        "    ((static_cast<unsigned>(major_) << 24) | \\\n"
        "     (static_cast<unsigned>(minor_) << 8) | \\\n"
        "     (static_cast<unsigned>(patch_)))\n\n"
        "/// @brief Full version of the protocol library as single number.\n"
        "#define CC_C_#^#PREFIX#$#_VERSION CC_C_#^#PREFIX#$#_MAKE_VERSION(CC_C_#^#PREFIX#$#_MAJOR_VERSION, CC_C_#^#PREFIX#$#_MINOR_VERSION, CC_C_#^#PREFIX#$#_PATCH_VERSION)\n"
        ;

    util::GenReplacementMap repl = {
        {"PREFIX", util::genStrToUpper(m_cGenerator.cNamesPrefix())},
        {"MAJOR_VERSION", tokens[CGenerator::GenVersionIdx_Major]},
        {"MINOR_VERSION", tokens[CGenerator::GenVersionIdx_Minor]},
        {"PATCH_VERSION", tokens[CGenerator::GenVersionIdx_Patch]},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CVersion::cCodeVersionCommsHeaderInternal() const
{
    auto& codeVersion = m_cGenerator.genGetCodeVersion();
    if (codeVersion.empty()) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "static_assert(CC_C_#^#PREFIX#$#_VERSION == #^#NS#$#_VERSION, \"Versions mismatch\");\n"
        ;

    util::GenReplacementMap repl = {
        {"PREFIX", util::genStrToUpper(m_cGenerator.cNamesPrefix())},
        {"NS", util::genStrToUpper(m_cGenerator.genProtocolSchema().genMainNamespace())},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c