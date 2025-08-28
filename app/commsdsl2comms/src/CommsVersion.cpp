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

#include "CommsVersion.h"

#include "CommsGenerator.h"
#include "CommsSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

enum CommsVersionIdx
{
    VersionIdx_major,
    VersionIdx_minor,
    VersionIdx_patch,
    VersionIdx_numOfValues
};

} // namespace 
    

bool CommsVersion::commsWrite(CommsGenerator& generator)
{
    auto& thisSchema = static_cast<CommsSchema&>(generator.genCurrentSchema());
    if ((!generator.genIsCurrentProtocolSchema()) && (!thisSchema.commsHasAnyGeneratedCode())) {
        return true;
    }

    CommsVersion obj(generator);
    return obj.commsWriteInternal();
}

bool CommsVersion::commsWriteInternal() const
{
    auto filePath = comms::genHeaderPathRoot(strings::genVersionFileNameStr(), m_commsGenerator);

    m_commsGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_commsGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains protocol version definition.\n\n"
        "#pragma once\n\n"
        "#include \"comms/version.h\"\n\n"
        "/// @brief Version of the protocol specification.\n"
        "#define #^#NS#$#_SPEC_VERSION (#^#VERSION#$#)\n\n"
        "#^#PROT_VER_DEFINE#$#\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "/// @brief Version of the protocol specification.\n"
        "inline constexpr unsigned specVersion()\n"
        "{\n"
        "    return #^#NS#$#_SPEC_VERSION;\n"
        "}\n\n"
        "#^#PROT_VER_FUNC#$#\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "// Generated compile time check for minimal supported version of the COMMS library\n"
        "static_assert(COMMS_MAKE_VERSION(#^#COMMS_MIN#$#) <= comms::version(),\n"
        "    \"The version of COMMS library is too old\");\n\n"
        "#^#APPEND#$#\n";


    util::GenReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"PROT_NAMESPACE", m_commsGenerator.genCurrentSchema().genMainNamespace()},
        {"VERSION", util::genNumToString(m_commsGenerator.genCurrentSchema().genSchemaVersion())},
        {"NS", util::genStrToUpper(m_commsGenerator.genCurrentSchema().genMainNamespace())},
        {"COMMS_MIN", util::genStrReplace(CommsGenerator::commsMinCommsVersion(), ".", ", ")},
        {"PROT_VER_DEFINE", commsProtVersionDefineInternal()},
        {"PROT_VER_FUNC", commsProtVersionFuncsInternal()},
        {"APPEND", util::genReadFileContents(comms::genInputCodePathForRoot(strings::genVersionFileNameStr(), m_commsGenerator))},
    };        
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_commsGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string CommsVersion::commsProtVersionDefineInternal() const
{
    auto& protVersion = m_commsGenerator.commsGetProtocolVersion();
    if (protVersion.empty()) {
        return strings::genEmptyString();
    }

    auto tokens = util::genStrSplitByAnyChar(protVersion, ".");
    while (tokens.size() < VersionIdx_numOfValues) {
        tokens.push_back("0");
    }

    const std::string Templ = 
        "/// @brief Major version of the protocol library.\n"
        "#define #^#NS#$#_MAJOR_VERSION (#^#MAJOR_VERSION#$#)\n\n"
        "/// @brief Minor version of the protocol library.\n"
        "#define #^#NS#$#_MINOR_VERSION (#^#MINOR_VERSION#$#)\n\n"        
        "/// @brief Patch version of the protocol library.\n"
        "#define #^#NS#$#_PATCH_VERSION (#^#PATCH_VERSION#$#)\n\n"        
        "/// @brief Full version of the protocol library as single number.\n"
        "#define #^#NS#$#_VERSION (COMMS_MAKE_VERSION(#^#NS#$#_MAJOR_VERSION, #^#NS#$#_MINOR_VERSION, #^#NS#$#_PATCH_VERSION))\n";

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_commsGenerator.genCurrentSchema().genMainNamespace())},
        {"MAJOR_VERSION", tokens[VersionIdx_major]},
        {"MINOR_VERSION", tokens[VersionIdx_minor]},
        {"PATCH_VERSION", tokens[VersionIdx_patch]},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsVersion::commsProtVersionFuncsInternal() const
{
    auto& protVersion = m_commsGenerator.commsGetProtocolVersion();
    if (protVersion.empty()) {
        return strings::genEmptyString();
    }

    const std::string Templ = 
        "/// @brief Major version of the protocol library\n"
        "inline constexpr unsigned versionMajor()\n"
        "{\n"
        "    return #^#NS#$#_MAJOR_VERSION;\n"
        "}\n\n"
        "/// @brief Minor version of the protocol library\n"
        "inline constexpr unsigned versionMinor()\n"
        "{\n"
        "    return #^#NS#$#_MINOR_VERSION;\n"
        "}\n\n"     
        "/// @brief Patch version of the protocol library\n"
        "inline constexpr unsigned versionPatch()\n"
        "{\n"
        "    return #^#NS#$#_PATCH_VERSION;\n"
        "}\n\n"  
        "/// @brief Full version of the protocol library as a single number\n"
        "inline constexpr unsigned version()\n"
        "{\n"
        "    return #^#NS#$#_VERSION;\n"
        "}\n";

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_commsGenerator.genCurrentSchema().genMainNamespace())},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2comms