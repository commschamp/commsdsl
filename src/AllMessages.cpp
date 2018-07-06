#include "AllMessages.h"

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
    "/// @brief Contains definition of @ref #^#PROT_NAMESPACE#$#::AllMessages definition.\n\n"
    "#pragma once\n\n"
    "#^#INCLUDES#$#\n"
    "#^#BEG_NAMESPACE#$#\n"
    "/// @brief All messages of the protocol in ascending order.\n"
    "/// @tparam TBase Base class of all the messages.\n"
    "/// @tparam TOpt Protocol definition options.\n"
    "template <typename TBase, typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions>\n"
    "using AllMessages =\n"
    "    std::tuple<\n"
    "        #^#MESSAGES#$#\n"
    "    >;\n\n"
    "#^#END_NAMESPACE#$#\n"
);

} // namespace

bool AllMessages::write(Generator& generator)
{
    AllMessages obj(generator);
    return obj.writeDefinition();
}

bool AllMessages::writeDefinition() const
{
    auto dir = m_generator.protocolDefRootDir();
    if (dir.empty()) {
        return false;
    }

    bf::path filePath(dir);
    filePath /= common::allMessagesStr() + common::headerSuffix();

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
        return false;
    }

    auto allMessages = m_generator.getAllDslMessages();
    common::StringsList messages;
    common::StringsList includes;
    messages.reserve(allMessages.size());
    includes.reserve(allMessages.size());

    for (auto m : allMessages) {
        assert(m.valid());
        if (!m_generator.isAnyPlatformSupported(m.platforms())) {
            continue;
        }

        if (!m_generator.doesElementExist(m.sinceVersion(), m.deprecatedSince(), m.isDeprecatedRemoved())) {
            continue;
        }

        auto extRef = m.externalRef();
        assert(!extRef.empty());
        messages.push_back(m_generator.scopeForMessage(extRef, true, true) + "<TBase, TOpt>");
        includes.push_back(m_generator.headerfileForMessage(extRef, false));
    }

    common::mergeInclude("<tuple>", includes);
    common::mergeInclude(m_generator.mainNamespace() + '/' + common::defaultOptionsStr() + common::headerSuffix(), includes);

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(includes)));
    replacements.insert(std::make_pair("MESSAGES", common::listToString(messages, ",\n", common::emptyString())));

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
