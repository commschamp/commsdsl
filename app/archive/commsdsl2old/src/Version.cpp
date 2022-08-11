//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "Version.h"

#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "EnumField.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2old
{

namespace
{

enum VersionToken : unsigned
{
    VersionToken_Major,
    VersionToken_Minor,
    VersionToken_Patch,
    VersionToken_NumOfValues
};

bool splitProtVersion(const std::string& verString, Version::VersionNumbers& verOut)
{
    std::vector<std::string> tokens;
    ba::split(tokens, verString, ba::is_any_of("."));
    if (VersionToken_NumOfValues < tokens.size()) {
        return false;
    }

    try {

        verOut.clear();
        for (auto& t : tokens) {
            verOut.push_back(static_cast<unsigned>(std::stoul(t)));
        }

        verOut.resize(VersionToken_NumOfValues);
    }
    catch (...) {
        // Not an unsigned number
        return false;
    }
    return true;
}

} // namespace

bool Version::write(Generator& generator)
{
    Version obj(generator);
    return obj.writeDefinition();
}

bool Version::writeDefinition() const
{
    auto startInfo = m_generator.startGenericProtocolWrite(common::versionStr());
    auto& filePath = startInfo.first;

    if (filePath.empty()) {
        return true;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto versionHeaderFileName = 
        common::nameToClassCopy(common::versionStr()) + common::headerSuffix();
        

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("NS", common::toUpperCopy(m_generator.mainNamespace())));
    replacements.insert(std::make_pair("COMMS_MIN", m_generator.getMinCommsVersionStr()));
    replacements.insert(std::make_pair("VERSION", common::numToString(m_generator.schemaVersion())));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForProtocolDefFile(versionHeaderFileName)));

    std::string protVersion = m_generator.getProtocolVersion();
    do {
        if (protVersion.empty()) {
            break;
        }

        VersionNumbers versions;
        if (!splitProtVersion(protVersion, versions)) {
            m_generator.logger().error("Invalid protocol version format: \"" + protVersion + "\" .");
            return false;
        }

        replacements.insert(std::make_pair("PROT_VER_DEFINE", protVersionDefine(versions)));
        replacements.insert(std::make_pair("PROT_VER_FUNC", protVersionFunc()));
    } while (false);


    static const std::string Template(
        "#^#GEN_COMMENT#$#\n"
        "/// @file\n"
        "/// @brief Contains protocol version definition.\n\n"
        "#pragma once\n\n"
        "#include \"comms/version.h\"\n\n"
        "/// @brief Version of the protocol specification.\n"
        "#define #^#NS#$#_SPEC_VERSION (#^#VERSION#$#)\n\n"
        "#^#PROT_VER_DEFINE#$#\n"
        "#^#BEG_NAMESPACE#$#\n"
        "/// @brief Version of the protocol specification.\n"
        "inline constexpr unsigned specVersion()\n"
        "{\n"
        "    return #^#NS#$#_SPEC_VERSION;\n"
        "}\n\n"
        "#^#PROT_VER_FUNC#$#\n"
        "#^#END_NAMESPACE#$#\n"
        "// Generated compile time check for minimal supported version of the COMMS library\n"
        "static_assert(COMMS_MAKE_VERSION(#^#COMMS_MIN#$#) <= comms::version(),\n"
        "    \"The version of COMMS library is too old\");\n\n"
        "#^#APPEND#$#\n"
    );
    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}


std::string Version::protVersionDefine(const VersionNumbers& version) const
{
    assert(VersionToken_NumOfValues <= version.size());
    common::ReplacementMap repl;
    repl.insert(std::make_pair("NS", common::toUpperCopy(m_generator.mainNamespace())));
    repl.insert(std::make_pair("MAJOR_VERSION", common::numToString(version[VersionToken_Major])));
    repl.insert(std::make_pair("MINOR_VERSION", common::numToString(version[VersionToken_Minor])));
    repl.insert(std::make_pair("PATCH_VERSION", common::numToString(version[VersionToken_Patch])));

    static const std::string Templ = 
        "/// @brief Major version of the protocol library.\n"
        "#define #^#NS#$#_MAJOR_VERSION (#^#MAJOR_VERSION#$#)\n\n"
        "/// @brief Minor version of the protocol library.\n"
        "#define #^#NS#$#_MINOR_VERSION (#^#MINOR_VERSION#$#)\n\n"        
        "/// @brief Patch version of the protocol library.\n"
        "#define #^#NS#$#_PATCH_VERSION (#^#PATCH_VERSION#$#)\n\n"        
        "/// @brief Full version of the protocol library as single number.\n"
        "#define #^#NS#$#_VERSION (COMMS_MAKE_VERSION(#^#NS#$#_MAJOR_VERSION, #^#NS#$#_MINOR_VERSION, #^#NS#$#_PATCH_VERSION))\n\n";

    return common::processTemplate(Templ, repl);            
}

std::string Version::protVersionFunc() const
{
    common::ReplacementMap repl;
    repl.insert(std::make_pair("NS", common::toUpperCopy(m_generator.mainNamespace())));

    static const std::string Templ = 
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
        "}\n\n" ;

    return common::processTemplate(Templ, repl);       
}

} // namespace commsdsl2old