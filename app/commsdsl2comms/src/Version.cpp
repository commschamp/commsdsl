//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"
#include "EnumField.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

namespace
{

const std::string Template(
    "/// @file\n"
    "/// @brief Contains protocol version definition.\n\n"
    "#pragma once\n\n"
    "#include \"comms/version.h\"\n\n"
    "/// @brief Version of the protocol library as single numeric value\n"
    "#define #^#NS#$#_VERSION (#^#VERSION#$#)\n\n"
    "#^#BEG_NAMESPACE#$#\n"
    "/// @brief Version of the protocol library as single numeric value\n"
    "inline constexpr unsigned version()\n"
    "{\n"
    "    return #^#NS#$#_VERSION;\n"
    "}\n\n"
    "#^#END_NAMESPACE#$#\n"
    "// Generated compile time check for minimal supported version of the COMMS library\n"
    "static_assert(COMMS_MAKE_VERSION(#^#COMMS_MIN#$#) <= comms::version(),\n"
    "    \"The version of COMMS library is too old\");\n\n"
    "#^#APPEND#$#\n"
);

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
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("NS", common::toUpperCopy(m_generator.mainNamespace())));
    replacements.insert(std::make_pair("COMMS_MIN", m_generator.getMinCommsVersionStr()));
    replacements.insert(std::make_pair("VERSION", common::numToString(m_generator.schemaVersion())));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForProtocolDefFile(versionHeaderFileName)));
    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

} // namespace commsdsl2comms
