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

#include "DefaultOptions.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2old
{

namespace 
{

const std::string BaseTemplateParam("TBase");

} // namespace 
    

bool DefaultOptions::write(Generator& generator)
{
    DefaultOptions obj(generator);
    return 
        obj.writeDefinition() &&
        obj.writeClientServer(true) &&
        obj.writeClientServer(false) &&
        obj.writeBareMetal() && 
        obj.writeDataView();
}

bool DefaultOptions::writeDefinition() const
{
    auto info = m_generator.startOptionsProtocolWrite(common::defaultOptionsStr());
    auto& fileName = info.first;
    auto& className = info.second;

    if (fileName.empty()) {
        return true;
    }

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForOptions();
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
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
        "#^#GEN_COMMENT#$#\n"
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

bool DefaultOptions::writeClientServer(bool client) const
{
    std::string type;
    std::string body;

    if (client) {
        type = "Client";
        body = m_generator.getClientDefaultOptionsBody(BaseTemplateParam);
    }
    else {
        type = "Server";
        body = m_generator.getServerDefaultOptionsBody(BaseTemplateParam);
    }

    // if (body.empty()) {
    //     return true;
    // }

    auto info = m_generator.startOptionsProtocolWrite(type + common::defaultOptionsStr());
    auto& fileName = info.first;
    auto& className = info.second;

    if (fileName.empty()) {
        return true;
    }

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForOptions();
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("BODY", std::move(body)));
    replacements.insert(std::make_pair("TYPE", common::toLowerCopy(type)));
    replacements.insert(std::make_pair("DEFAULT_OPT", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    replacements.insert(std::make_pair("BASE", BaseTemplateParam));

    std::ofstream stream(fileName);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + fileName + "\" for writing.");
        return false;
    }

    static const std::string Template(
        "#^#GEN_COMMENT#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol default options for a #^#TYPE#$#.\n\n"
        "#pragma once\n\n"
        "#include \"DefaultOptions.h\"\n\n"
        "#^#BEG_NAMESPACE#$#\n"
        "/// @brief Default options of the protocol for a #^#TYPE#$#.\n"
        "/// @tparam #^#BASE#$# Options to use as a basis.\n"
        "template <typename #^#BASE#$# = #^#DEFAULT_OPT#$#>\n"        
        "struct #^#CLASS_NAME#$#T : public #^#BASE#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "/// @brief Alias to @ref #^#CLASS_NAME#$#T with default template parameter.\n"
        "using #^#CLASS_NAME#$# = #^#CLASS_NAME#$#T<>;\n\n"        
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

bool DefaultOptions::writeBareMetal() const
{
    std::string body = m_generator.getBareMetalDefaultOptionsBody(BaseTemplateParam);

    auto info = m_generator.startOptionsProtocolWrite(common::bareMetalStr() + common::defaultOptionsStr());
    auto& fileName = info.first;
    auto& className = info.second;

    if (fileName.empty()) {
        return true;
    }

    static const common::StringsList Includes = {
        m_generator.headerfileForOptions(common::defaultOptionsStr(), false)
    };

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForOptions();
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("BODY", std::move(body)));
    replacements.insert(std::make_pair("SEQ_DEFAULT_SIZE", common::seqDefaultSizeStr()));
    replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(Includes)));
    replacements.insert(std::make_pair("DEFAULT_OPT", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    replacements.insert(std::make_pair("BASE", BaseTemplateParam));


    std::ofstream stream(fileName);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + fileName + "\" for writing.");
        return false;
    }

    static const std::string Template(
        "#^#GEN_COMMENT#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol default options for bare-metal application\n"
        "///    where usage of dynamic memory allocation is disabled.\n\n"
        "#pragma once\n\n"
        "#ifndef #^#SEQ_DEFAULT_SIZE#$#\n"
        "/// @brief Define default fixed size for various sequence fields\n"
        "/// @details May be defined during compile time to change the default value.\n"
        "#define #^#SEQ_DEFAULT_SIZE#$# 32\n"
        "#endif\n\n"
        "#^#INCLUDES#$#\n"
        "#^#BEG_NAMESPACE#$#\n"
        "/// @brief Default options for bare-metal application where usage of dynamic\n"
        "///    memory allocation is diabled.\n"
        "/// @tparam #^#BASE#$# Options to use as a basis.\n"
        "template <typename #^#BASE#$# = #^#DEFAULT_OPT#$#>\n"
        "struct #^#CLASS_NAME#$#T : public #^#BASE#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "/// @brief Alias to @ref #^#CLASS_NAME#$#T with default template parameter.\n"
        "using #^#CLASS_NAME#$# = #^#CLASS_NAME#$#T<>;\n\n"
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

bool DefaultOptions::writeDataView() const
{
    std::string body = m_generator.getDataViewDefaultOptionsBody(BaseTemplateParam);

    auto info = m_generator.startOptionsProtocolWrite(common::dataViewStr() + common::defaultOptionsStr());
    auto& fileName = info.first;
    auto& className = info.second;

    if (fileName.empty()) {
        return true;
    }

    static const common::StringsList Includes = {
        m_generator.headerfileForOptions(common::defaultOptionsStr(), false)
    };

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForOptions();
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("BODY", std::move(body)));
    replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(Includes)));
    replacements.insert(std::make_pair("DEFAULT_OPT", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    replacements.insert(std::make_pair("BASE", BaseTemplateParam));


    std::ofstream stream(fileName);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + fileName + "\" for writing.");
        return false;
    }

    static const std::string Template(
        "#^#GEN_COMMENT#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol default options that apply\n"
        "///     @b comms::option::app::OrigDataView to applicable fields.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#BEG_NAMESPACE#$#\n"
        "/// @brief Default options for data view on contiguous buffers to avoid\n"
        "///    unnecessary copy of data.\n"
        "/// @tparam #^#BASE#$# Options to use as a basis.\n"
        "template <typename #^#BASE#$# = #^#DEFAULT_OPT#$#>\n"
        "struct #^#CLASS_NAME#$#T : public #^#BASE#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "/// @brief Alias to @ref #^#CLASS_NAME#$#T with default template parameter.\n"
        "using #^#CLASS_NAME#$# = #^#CLASS_NAME#$#T<>;\n\n"
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


} // namespace commsdsl2old