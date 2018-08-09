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
    "/// @file\n"
    "/// @brief Contains definition of base class of all the fields.\n\n"
    "#pragma once\n\n"
    "#include \"comms/Field.h\"\n"
    "#include \"comms/options.h\"\n\n"
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
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("OPTIONS", std::move(optionsStr)));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));

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
