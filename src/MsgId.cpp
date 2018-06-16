#include "MsgId.h"

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "EnumField.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string Template(
    "/// @file\n"
    "/// @brief Contains definition of message ids enumeration.\n\n"
    "#pragma once\n\n"
    "#include <cstdint>\n\n"
    "#^#BEG_NAMESPACE#$#\n"
    "/// @brief Message ids enumeration.\n"
    "enum MsgId #^#TYPE#$#\n"
    "{\n"
    "    #^#IDS#$#\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
);

} // namespace

bool MsgId::write(Generator& generator)
{
    MsgId obj(generator);
    return obj.writeDefinition();
}

bool MsgId::writeDefinition() const
{
    auto dir = m_generator.protocolDefRootDir();
    if (dir.empty()) {
        return false;
    }

    bf::path filePath(dir);
    filePath /= common::msgIdEnumNameStr() + common::headerSuffix();

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
        return false;
    }


    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    std::string idsStr;
    std::string typeStr;
    auto* msgIdField = m_generator.getMessageIdField();
    if (msgIdField != nullptr) {
        assert(msgIdField->kind() == commsdsl::Field::Kind::Enum);
        auto* castedMsgIdField = static_cast<const EnumField*>(msgIdField);
        auto values = castedMsgIdField->getValuesList(false);
        // TODO: generate doxygen description
        auto& prefix = common::msgIdPrefixStr();
        for (auto& v : values) {
            v.insert(v.begin(), prefix.begin(), prefix.end());
        }

        idsStr = common::listToString(values, ",\n", common::emptyString());
        typeStr = ": " + castedMsgIdField->underlyingType();
    }
    else {
        auto allMessages = m_generator.getAllMessageIds();
        common::StringsList ids;
        ids.reserve(allMessages.size());
        for (auto& m : allMessages) {
            ids.push_back(m.second + " = " + common::numToString(m.first));
        }

        idsStr = common::listToString(ids, ",\n", common::emptyString());
    }
    replacements.insert(std::make_pair("IDS", std::move(idsStr)));
    replacements.insert(std::make_pair("TYPE", std::move(typeStr)));

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
