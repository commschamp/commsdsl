#include "Message.h"

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

const std::string Template(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#MESSAGE_NAME#$#\"<\\b> message and its fields.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
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
    "/// @headerfile #^#MESSAGE_HEADERFILE#$#\n"
    "template <typename TMsgBase, typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions>\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::MessageBase<\n"
    "        TMsgBase,\n"
    "        typename TOpt::#^#NAMESPACE_SCOPE#$#,\n"
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
    "            typename TOpt::#^#NAMESPACE_SCOPE#$#,\n"
    "            comms::option::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
    "            comms::option::FieldsImpl<typename #^#CLASS_NAME#$#Fields<TOpt>::All>,\n"
    "            comms::option::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
    "            comms::option::HasName\n"
    "        >;\n"
    "\n"
    "public:\n"
    "#^#MESSAGE_BODY#$#\n"
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

    // TODO
    return true;
}

bool Message::write()
{
    bool exists =
        m_generator.doesElementExist(
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
            m_dslObj.isDeprecatedRemoved());
    if (!exists) {
        return true;
    }

    // TODO: write plugin
    return writeProtocol();
}

std::string Message::getDefaultOptions() const
{
    std::string fieldsOpts;
    auto addFieldOptsFunc =
        [&fieldsOpts](const std::string& str)
        {
            if (str.empty()) {
                return;
            }

            if (!fieldsOpts.empty()) {
                fieldsOpts += '\n';
            }

            fieldsOpts += str;
        };

    for (auto& f : m_fields) {
        addFieldOptsFunc(f->getDefaultOptions());
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("MESSAGE_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("FIELDS_OPTS", std::move(fieldsOpts)));

    static const std::string Templ =
        "struct #^#MESSAGE_NAME#$#Fields\n"
        "{\n"
        "    #^#FIELDS_OPTS#$#\n"
        "};\n\n"
        "using #^#MESSAGE_NAME#$# = comms::option::EmptyOption;\n";

    static const std::string NoFieldsTempl =
        "using #^#MESSAGE_NAME#$# = comms::option::EmptyOption;\n";

    auto* templ = &Templ;
    if (m_fields.empty()) {
        templ = &NoFieldsTempl;
    }

    return common::processTemplate(*templ, replacements);
}

bool Message::writeProtocol()
{
    assert(!m_externalRef.empty());

    auto names =
        m_generator.startMessageProtocolWrite(
            m_externalRef,
            m_dslObj.platforms());
    auto& filePath = names.first;
    auto& className = names.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("MESSAGE_NAME", getDisplayName()));
    replacements.insert(std::make_pair("DOC_DETAILS", getDescription()));
    replacements.insert(std::make_pair("MESSAGE_ID", m_generator.getMessageIdStr(m_externalRef, m_dslObj.id())));

    auto namespaces = m_generator.namespacesForMessage(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    replacements.insert(std::make_pair("MESSAGE_HEADERFILE", m_generator.headerfileForMessage(m_externalRef)));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("FIELDS_LIST", getFieldsClassesList()));
    replacements.insert(std::make_pair("INCLUDES", getIncludes()));
    replacements.insert(std::make_pair("MESSAGE_BODY", getBody()));
    replacements.insert(std::make_pair("FIELDS_DEF", getFieldsDef()));
    replacements.insert(std::make_pair("NAMESPACE_SCOPE", getNamespaceScope()));
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

const std::string& Message::getDisplayName() const
{
    auto* displayName = &m_dslObj.displayName();
    if (displayName->empty()) {
        displayName = &m_dslObj.name();
    }
    return *displayName;
}

std::string Message::getDescription() const
{
    auto desc = common::makeMultiline(m_dslObj.description());
    if (!desc.empty()) {
        static const std::string DocPrefix("///     ");
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + DocPrefix);
        ba::replace_all(desc, "\n", DocNewLineRepl);
        desc += " @n";
    }
    return desc;
}

std::string Message::getFieldsClassesList() const
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

std::string Message::getIncludes() const
{
    common::StringsList includes;
    for (auto& f : m_fields) {
        f->updateIncludes(includes);
    }

    static const common::StringsList MessageIncludes = {
        "<tuple>",
        "comms/MessageBase.h",
        m_generator.mainNamespace() + '/' + common::msgIdEnuNameStr() + common::headerSuffix(),
        m_generator.mainNamespace() + '/' + common::defaultOptionsStr() + common::headerSuffix()
    };
    common::mergeIncludes(MessageIncludes, includes);

    return common::includesToStatements(includes);
}

std::string Message::getBody() const
{
    std::string result = getPublic();
    std::string prot = getProtected();
    if (!prot.empty()) {
        result += "\nprotected:\n";
        result += prot;
    }

    std::string priv = getPrivate();
    if (!priv.empty()) {
        result += "\nprivate:\n";
        result += priv;
    }

    return result;
}

std::string Message::getPublic() const
{
    std::string result;
    result += getFieldsAccess();
    result += getLengthCheck();
    result += getNameFunc();
    common::insertIndent(result);
    return result;
}

std::string Message::getProtected() const
{
    // TODO:
    return common::emptyString();
}

std::string Message::getPrivate() const
{
    // TODO:
    return common::emptyString();
}

std::string Message::getFieldsAccess() const
{
    if (m_fields.empty()) {
        return common::emptyString();
    }

    static const std::string DocPrefix =
        "/// @brief Allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_MSG_FIELDS_ACCESS macro\n"
        "///     related to @b comms::MessageBase class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated functions are:\n";

    std::string result = DocPrefix;
    for (auto& f : m_fields) {
        result += common::doxygenPrefixStr();
        result += common::indentStr();
        result += "@li @b field_";
        result += common::nameToAccessCopy(f->name());
        result += "() for @ref ";
        result += common::nameToClassCopy(name());
        result += "Fields::";
        result += common::nameToClassCopy(f->name());
        result += " field.\n";
    }

    result += "COMMS_MSG_FIELDS_ACCESS(\n";
    for (auto& f : m_fields) {
        result += common::indentStr();
        result += common::nameToAccessCopy(f->name());
        if (&f != &m_fields.back()) {
            result += ',';
        }
        result += '\n';
    }
    result += ");\n\n";

    return result;
}

std::string Message::getLengthCheck() const
{
    static const std::size_t MaxLen =
        std::numeric_limits<std::size_t>::max();

    auto minLength =
        std::accumulate(
            m_fields.begin(), m_fields.end(), std::size_t(0),
            [](std::size_t soFar, auto& f)
            {
                return soFar + f->minLength();
            });
    auto maxLength =
        std::accumulate(
            m_fields.begin(), m_fields.end(), std::size_t(0),
            [](std::size_t soFar, auto& f)
            {


                if (soFar == MaxLen) {
                    return MaxLen;
                }

                auto fLen = f->maxLength();
                if ((MaxLen - soFar) <= fLen) {
                    return MaxLen;
                }

                return soFar + fLen;
            });

    std::string result =
            "// Compile time check for serialisation length.\n"
            "static const std::size_t MsgMinLen = Base::doMinLength();\n";
    if (maxLength != MaxLen) {
        result += "static const std::size_t MsgMaxLen = Base::doMaxLength();\n";
    }
    result += "static_assert(MsgMinLen == ";
    result += common::numToString(minLength);
    result += ", \"Unexpected min serialisation length\");\n";

    if (maxLength != MaxLen) {
        result += "static_assert(MsgMaxLen == ";
        result += common::numToString(maxLength);
        result += ", \"Unexpected max serialisation length\");\n";
    }
    return result;
}

std::string Message::getFieldsDef() const
{
    std::string result;
    auto scope =
        "TOpt::" +
        getNamespaceScope() +
        common::fieldsSuffixStr() +
        "::";

    for (auto& f : m_fields) {
        result += f->getClassDefinition(scope);
        if (&f != &m_fields.back()) {
            result += '\n';
        }
    }
    return result;
}

std::string Message::getNamespaceScope() const
{
    return
        m_generator.scopeForMessage(m_externalRef) +
            common::nameToClassCopy(name());
}

std::string Message::getNameFunc() const
{
    return
        "\n"
        "/// @brief Name of the message.\n"
        "static const char* doName()\n"
        "{\n"
        "    return \"" + getDisplayName() + "\";\n"
        "}\n";
}

}
