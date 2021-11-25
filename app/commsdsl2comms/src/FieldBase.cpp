//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "FieldBase.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

namespace
{

const std::string Template(
    "#^#GEN_COMMENT#$#\n"
    "/// @file\n"
    "/// @brief Contains definition of base class of all the fields.\n\n"
    "#pragma once\n\n"
    "#include \"comms/Field.h\"\n"
    "#include \"comms/options.h\"\n"
    "#include \"#^#PROT_NAMESPACE#$#/Version.h\"\n\n"
    "#^#BEG_NAMESPACE#$#\n"
    "/// @brief Common base class for all the fields.\n"
    "/// @tparam TOpt Extra options.\n"
    "template <typename... TOpt>\n"
    "using #^#CLASS_NAME#$# =\n"
    "    comms::Field<\n"
    "        TOpt...,\n"
    "        #^#OPTIONS#$#\n"
    "    >;\n\n"
    "#^#END_NAMESPACE#$#\n"
);

} // namespace

bool FieldBase::write(Generator& generator)
{
    FieldBase obj(generator);
    return obj.writeDefinition();
}

bool FieldBase::writeDefinition() const
{
    auto startInfo = m_generator.startFieldProtocolWrite(common::fieldBaseStr());
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        return true;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    common::StringsList options;
    options.push_back(common::dslEndianToOpt(m_generator.schemaEndian()));
    // TODO: version type
    std::string optionsStr = common::listToString(options, ",\n", common::emptyString());

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForField(common::fieldBaseStr());
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("OPTIONS", std::move(optionsStr)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));

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
