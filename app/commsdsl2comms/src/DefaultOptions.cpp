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
    auto info = m_generator.startDefaultOptionsWrite();
    auto& filename = info.first;

    auto dir = m_generator.protocolDefRootDir();
    if (dir.empty()) {
        return false;
    }

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(info.second)));
    replacements.insert(std::make_pair("BODY", m_generator.getDefaultOptionsBody()));

    std::ofstream stream(filename);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filename + "\" for writing.");
        return false;
    }

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filename + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2comms
