#include "DefaultOptions.h"

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

} // namespace

bool DefaultOptions::write(Generator& generator)
{
    DefaultOptions obj(generator);
    return obj.writeDefinition();
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
