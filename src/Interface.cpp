#include "Interface.h"

#include <cassert>
#include <fstream>
#include <map>
#include <algorithm>
#include <iterator>
#include <numeric>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string AliasTemplate(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$#\"<\\b> interface class.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"<\\b> common interface class.\n"
    "#^#DOC_DETAILS#$#\n"
    "/// @tparam TOpt Interface definition options\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "template <typename... TOpt>\n"
    "using #^#CLASS_NAME#$# =\n"
    "    comms::Message<\n"
    "        TOpt...\n,"
    "        #^#ENDIAN#$#,\n"
    "        comms::option::MsgIdType<#^#PROT_NAMESPACE#$#::MsgId>\n"
    "    >;\n\n"
    "#^#END_NAMESPACE#$#\n"
);

} // namespace

bool Interface::prepare()
{
    if (!m_dslObj.valid()) {
        return true;
    }


    m_externalRef = m_dslObj.externalRef();
    if (m_externalRef.empty()) {
        m_generator.logger().log(commsdsl::ErrorLevel_Error, "Unknown external reference for message: " + m_dslObj.name());
        return false;
    }

    auto dslFields = m_dslObj.fields();
    m_fields.reserve(dslFields.size());
    for (auto& f : dslFields) {
        auto ptr = Field::create(m_generator, f);
        assert(ptr);
        if (!ptr->doesExist()) {
            continue;
        }

        if (!ptr->prepare()) {
            return false;
        }
        m_fields.push_back(std::move(ptr));
    }

    return true;
}

bool Interface::write()
{
    // TODO: write plugin
    return writeProtocol();
}

bool Interface::writeProtocol()
{
    auto names =
        m_generator.startInterfaceProtocolWrite(m_externalRef);
    auto& filePath = names.first;
    auto& className = names.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("DOC_DETAILS", getDescription()));
    replacements.insert(std::make_pair("ENDIAN", common::dslEndianToOpt(m_generator.schemaEndian())));
    replacements.insert(std::make_pair("INCLUDES", getIncludes()));
    replacements.insert(std::make_pair("HEADERFILE", m_generator.headerfileForInterface(m_externalRef)));

    auto namespaces = m_generator.namespacesForInterface(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    // TODO: all values

    // TODO: class template
    auto* templ = &AliasTemplate;
    auto str = common::processTemplate(*templ, replacements);

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

std::string Interface::getDescription() const
{
    if (!m_dslObj.valid()) {
        return common::emptyString();
    }

    auto desc = common::makeMultiline(m_dslObj.description());
    if (!desc.empty()) {
        static const std::string DocPrefix("/// @details ");
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + common::doxygenPrefixStr());
        ba::replace_all(desc, "\n", DocNewLineRepl);
        desc += " @n";
    }
    return desc;
}

std::string Interface::getFieldsClassesList() const
{
    std::string result;
    for (auto& f : m_fields) {
        if (!result.empty()) {
            result += ",\n";
        }
        result += common::nameToClassCopy(f->name());
    }
    return result;
}

std::string Interface::getIncludes() const
{
    common::StringsList includes;
    for (auto& f : m_fields) {
        f->updateIncludes(includes);
    }

    static const common::StringsList InterfaceIncludes = {
        "comms/Message.h",
        "comms/options.h",
        m_generator.mainNamespace() + '/' + common::msgIdEnuNameStr() + common::headerSuffix()
    };

    common::mergeIncludes(InterfaceIncludes, includes);
    return common::includesToStatements(includes);
}

std::string Interface::getFieldsAccess() const
{
    if (m_fields.empty()) {
        return common::emptyString();
    }

//    static const std::string DocPrefix =
//        "/// @brief Allow access to internal fields.\n"
//        "/// @details See definition of @b COMMS_MSG_FIELDS_ACCESS macro\n"
//        "///     related to @b comms::InterfaceBase class from COMMS library\n"
//        "///     for details.\n"
//        "///\n"
//        "///     The generated functions are:\n";

//    std::string result = DocPrefix;
//    for (auto& f : m_fields) {
//        result += common::doxygenPrefixStr();
//        result += common::indentStr();
//        result += "@li @b field_";
//        result += common::nameToAccessCopy(f->name());
//        result += "() for @ref ";
//        result += common::nameToClassCopy(name());
//        result += "Fields::";
//        result += common::nameToClassCopy(f->name());
//        result += " field.\n";
//    }

//    result += "COMMS_MSG_FIELDS_ACCESS(\n";
//    for (auto& f : m_fields) {
//        result += common::indentStr();
//        result += common::nameToAccessCopy(f->name());
//        if (&f != &m_fields.back()) {
//            result += ',';
//        }
//        result += '\n';
//    }
//    result += ");\n\n";

//    return result;
    // TODO
    return common::emptyString();
}

std::string Interface::getFieldsDef() const
{
    // TODO:
    return common::emptyString();
//    std::string result;
//    auto scope =
//        getNamespaceScope() +
//        common::fieldsSuffixStr() +
//        "::";

//    for (auto& f : m_fields) {
//        result += f->getClassDefinition(scope);
//        if (&f != &m_fields.back()) {
//            result += '\n';
//        }
//    }
//    return result;
}

}
