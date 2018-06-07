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
    "#pragma once\n\n"
    "#include \"comms/Field.h\"\n"
    "#include \"comms/options.h\"\n\n"
    "#^#BEG_NAMESPACE#$#\n"
    "/// @brief Common base class for all the fields.\n"
    "using FieldBase =\n"
    "    comms::Field<\n"
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
    auto dir = m_generator.protocolDefRootDir();
    if (dir.empty()) {
        return false;
    }

    bf::path filePath(dir);
    filePath /= common::fieldBaseStr() + common::headerSuffix();

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
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

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePathStr + "\".");
        return false;
    }
    return true;
}

} // namespace commsdsl2comms
