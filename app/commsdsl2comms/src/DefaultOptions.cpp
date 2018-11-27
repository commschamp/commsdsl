//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "DefaultOptions.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool DefaultOptions::write(Generator& generator)
{
    DefaultOptions obj(generator);
    return 
        obj.writeDefinition() &&
        obj.writeClientServerWrite(true) &&
        obj.writeClientServerWrite(false);
}

bool DefaultOptions::writeDefinition() const
{
    auto info = m_generator.startGenericProtocolWrite(common::defaultOptionsStr());
    auto& fileName = info.first;
    auto& className = info.second;

    if (fileName.empty()) {
        return true;
    }

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("BODY", m_generator.getDefaultOptionsBody()));

    std::ofstream stream(fileName);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + fileName + "\" for writing.");
        return false;
    }

    static const std::string Template(
        "/// @file\n"
        "/// @brief Contains definition of protocol default options.\n\n"
        "#pragma once\n\n"
        "#include \"comms/options.h\"\n\n"
        "#^#BEG_NAMESPACE#$#\n"
        "/// @brief Default (empty) options of the protocol.\n"
        "struct #^#CLASS_NAME#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "#^#END_NAMESPACE#$#\n"
    );

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + fileName + "\".");
        return false;
    }

    return true;
}

bool DefaultOptions::writeClientServerWrite(bool client) const
{
    std::string type;
    std::string body;

    if (client) {
        type = "Client";
        body = m_generator.getClientDefaultOptionsBody();
    }
    else {
        type = "Server";
        body = m_generator.getServerDefaultOptionsBody();
    }

    if (body.empty()) {
        return true;
    }

    auto info = m_generator.startGenericProtocolWrite(type + common::defaultOptionsStr());
    auto& fileName = info.first;
    auto& className = info.second;

    if (fileName.empty()) {
        return true;
    }

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("BODY", std::move(body)));
    replacements.insert(std::make_pair("TYPE", common::toLowerCopy(type)));

    std::ofstream stream(fileName);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + fileName + "\" for writing.");
        return false;
    }

    static const std::string Template(
        "/// @file\n"
        "/// @brief Contains definition of protocol default options for #^#TYPE#$#.\n\n"
        "#pragma once\n\n"
        "#include \"DefaultOptions.h\"\n\n"
        "#^#BEG_NAMESPACE#$#\n"
        "/// @brief Default options of the protocol specific for a #^#TYPE#$#.\n"
        "struct #^#CLASS_NAME#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "#^#END_NAMESPACE#$#\n"
    );

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + fileName + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2comms
