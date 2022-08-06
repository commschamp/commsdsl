//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include <fstream>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

enum VersionIdx
{
    VersionIdx_major,
    VersionIdx_minor,
    VersionIdx_patch,
    VersionIdx_numOfValues
};

} // namespace 
    

bool CommsVersion::write(CommsGenerator& generator)
{
    auto& thisSchema = static_cast<CommsSchema&>(generator.currentSchema());
    if ((!generator.isCurrentProtocolSchema()) && (!thisSchema.commsHasAnyGeneratedCode())) {
        return true;
    }

    CommsVersion obj(generator);
    return obj.writeInternal();
}

bool CommsVersion::writeInternal() const
{
    auto filePath = comms::headerPathRoot("Version", m_generator);

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
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


    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"PROT_NAMESPACE", m_generator.currentSchema().mainNamespace()},
        {"VERSION", util::numToString(m_generator.currentSchema().schemaVersion())},
        {"NS", util::strToUpper(m_generator.currentSchema().mainNamespace())},
        {"COMMS_MIN", util::strReplace(CommsGenerator::minCommsVersion(), ".", ", ")},
        {"PROT_VER_DEFINE", commsProtVersionDefineInternal()},
        {"PROT_VER_FUNC", commsProtVersionFuncsInternal()},
        {"APPEND", util::readFileContents(comms::inputCodePathForRoot("Version", m_generator))},
    };        
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string CommsVersion::commsProtVersionDefineInternal() const
{
    auto& protVersion = m_generator.getProtocolVersion();
    if (protVersion.empty()) {
        return strings::emptyString();
    }

    auto tokens = util::strSplitByAnyChar(protVersion, ".");
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

    util::ReplacementMap repl = {
        {"NS", util::strToUpper(m_generator.currentSchema().mainNamespace())},
        {"MAJOR_VERSION", tokens[VersionIdx_major]},
        {"MINOR_VERSION", tokens[VersionIdx_minor]},
        {"PATCH_VERSION", tokens[VersionIdx_patch]},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsVersion::commsProtVersionFuncsInternal() const
{
    auto& protVersion = m_generator.getProtocolVersion();
    if (protVersion.empty()) {
        return strings::emptyString();
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
        "/// @brief Full version of the protocol library as single number\n"
        "inline constexpr unsigned version()\n"
        "{\n"
        "    return #^#NS#$#_VERSION;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"NS", util::strToUpper(m_generator.currentSchema().mainNamespace())},
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2comms