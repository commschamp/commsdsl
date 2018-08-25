#include "AllMessages.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool AllMessages::write(Generator& generator)
{
    AllMessages obj(generator);
    return obj.writeProtocolDefinition() && obj.writePluginDefinition();
}

bool AllMessages::writeProtocolDefinition() const
{
    auto startInfo = m_generator.startGenericProtocolWrite(common::allMessagesStr());
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

    auto allMessages = m_generator.getAllDslMessages();
    common::StringsList messages;
    common::StringsList includes;
    messages.reserve(allMessages.size());
    includes.reserve(allMessages.size());

    for (auto m : allMessages) {
        assert(m.valid());

        if (!m_generator.doesElementExist(m.sinceVersion(), m.deprecatedSince(), m.isDeprecatedRemoved())) {
            continue;
        }

        auto extRef = m.externalRef();
        assert(!extRef.empty());
        messages.push_back(m_generator.scopeForMessage(extRef, true, true) + "<TBase, TOpt>");
        common::mergeInclude(m_generator.headerfileForMessage(extRef, false), includes);
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
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));

    const std::string Template(
        "/// @file\n"
        "/// @brief Contains definition of all messages bundle.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#BEG_NAMESPACE#$#\n"
        "/// @brief All messages of the protocol in ascending order.\n"
        "/// @tparam TBase Base class of all the messages.\n"
        "/// @tparam TOpt Protocol definition options.\n"
        "template <typename TBase, typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions>\n"
        "using #^#CLASS_NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#MESSAGES#$#\n"
        "    >;\n\n"
        "#^#END_NAMESPACE#$#\n"
    );

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

bool AllMessages::writePluginDefinition() const
{
    auto startInfo = m_generator.startGenericPluginHeaderWrite(common::allMessagesStr());
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

    auto allMessages = m_generator.getAllDslMessages();
    common::StringsList messages;
    common::StringsList includes;
    messages.reserve(allMessages.size());
    includes.reserve(allMessages.size());

    std::string interfaceStr;
    auto* interface = m_generator.getDefaultInterface();
    if (interface == nullptr) {
        interfaceStr = "<TInterface>";
    }

    for (auto m : allMessages) {
        assert(m.valid());

        if (!m_generator.doesElementExist(m.sinceVersion(), m.deprecatedSince(), m.isDeprecatedRemoved())) {
            continue;
        }

        auto extRef = m.externalRef();
        assert(!extRef.empty());
        messages.push_back(m_generator.scopeForMessageInPlugin(extRef) + interfaceStr);
        common::mergeInclude(m_generator.headerfileForMessageInPlugin(extRef, false), includes);
    }

    common::mergeInclude("<tuple>", includes);

    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForPlugin();
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(includes)));
    replacements.insert(std::make_pair("MESSAGES", common::listToString(messages, ",\n", common::emptyString())));

    if (interface == nullptr) {
        replacements.insert(std::make_pair("TEMPLATE_PARAM", "template <typename TInterface>"));
    }

    const std::string Template(
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#BEG_NAMESPACE#$#\n"
        "#^#TEMPLATE_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#MESSAGES#$#\n"
        "    >;\n\n"
        "#^#END_NAMESPACE#$#\n"
    );

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
