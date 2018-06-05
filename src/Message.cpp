#include "Message.h"

#include <cassert>
#include <fstream>
#include <map>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string Template(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#MESSAGE_NAME#$#\"<\\b> message and its fields.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#include \"comms/MessageBase.h\"\n"
    "#include \"#^#PROT_NAMESPACE#$#/DefaultOptions.h\"\n"
    "#^#INCLUDES#$#\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "/// @brief Fields of @ref #^#CLASS_NAME#$#.\n"
    "/// @tparam TOpt Extra options\n"
    "/// @see @ref #^#CLASS_NAME#$#\n"
    "/// @headerfile #^#MESSAGE_HEADERFILE#$#\n"
    "template <typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions>\n"
    "struct #^#CLASS_NAME#$#Fields\n"
    "{\n"
    "    #^#FIELDS_DEF#$#\n"
    "    /// @brief All the fields bundled in std::tuple.\n"
    "    using All = std::tuple<\n"
    "        #^#FIELDS_LIST#$#\n"
    "    >;\n"
    "};\n"
    "\n"
    "/// @brief Definition of <b>\"#^#MESSAGE_NAME#$#\"<\\b> message class.\n"
    "/// @details\n"
    "#^#DOC_DETAILS#$#\n"
    "///     See @ref #^#CLASS_NAME#$#Fields for definition of the fields this message contains.\n"
    "/// @tparam TMsgBase Base (interface) class.\n"
    "/// @tparam TOpt Extra options\n"
    "template <typename TMsgBase, typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions>\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::MessageBase<\n"
    "        TMsgBase,\n"
    "        typename TOpt::#^#NAMESPACE_SCOPE#$#::#^#CLASS_NAME#$#,\n"
    "        comms::option::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
    "        comms::option::FieldsImpl<typename #^#CLASS_NAME#$#Fields<TOpt>::All>,\n"
    "        comms::option::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
    "        comms::option::HasName\n"
    "    >\n"
    "{\n"
    "    // Redefinition of the base class type\n"
    "    using Base =\n"
    "        comms::MessageBase<\n"
    "            TMsgBase,\n"
    "            typename TOpt::#^#NAMESPACE_SCOPE#$#::#^#CLASS_NAME#$#,\n"
    "            comms::option::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
    "            comms::option::FieldsImpl<typename #^#CLASS_NAME#$#Fields<TOpt>::All>,\n"
    "            comms::option::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
    "            comms::option::HasName\n"
    "        >;\n"
    "\n"
    "public:\n"
    "    #^#MESSAGE_BODY#$#\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
);

} // namespace

bool Message::prepare()
{
    m_externalRef = m_dslObj.externalRef();
    if (m_externalRef.empty()) {
        m_generator.logger().log(commsdsl::ErrorLevel_Error, "Unknown external reference for message: " + m_dslObj.name());
        return false;
    }


    // TODO
    return true;
}

bool Message::write()
{
    // TODO: write plugin
    return writeProtocol();
}

bool Message::writeProtocol()
{
    assert(!m_externalRef.empty());

    auto names =
        m_generator.startMessageProtocolWrite(
            m_externalRef,
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
            m_dslObj.isDeprecatedRemoved(),
            m_dslObj.platforms());
    auto& filePath = names.first;
    auto& className = names.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    auto* displayName = &m_dslObj.displayName();
    if (displayName->empty()) {
        displayName = &m_dslObj.name();
    }
    replacements.insert(std::make_pair("MESSAGE_NAME", *displayName));

    auto desc = common::makeMultiline(m_dslObj.description());
    if (!desc.empty()) {
        static const std::string DocPrefix("///     ");
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + DocPrefix);
        ba::replace_all(desc, "\n", DocNewLineRepl);
        desc += " @n";
        replacements.insert(std::make_pair("DOC_DETAILS", std::move(desc)));
    }

    replacements.insert(std::make_pair("MESSAGE_ID", common::numToString(m_dslObj.id())));

    auto namespaces = m_generator.namespacesForMessage(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    replacements.insert(std::make_pair("MESSAGE_HEADERFILE", m_generator.headerfileForMessage(m_externalRef)));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    // TODO: all values

    auto str = common::processTemplate(Template, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

}
