//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
#include "EnumField.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string AliasTemplate(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$#\"</b> interface class.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"</b> common interface class.\n"
    "#^#DOC_DETAILS#$#\n"
    "/// @tparam TOpt Interface definition options\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "template <typename... TOpt>\n"
    "using #^#CLASS_NAME#$# =\n"
    "    comms::Message<\n"
    "        TOpt...,\n"
    "        #^#ENDIAN#$#,\n"
    "        comms::option::def::MsgIdType<#^#PROT_NAMESPACE#$#::MsgId>\n"
    "    >;\n\n"
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n"
);

const std::string ClassTemplate(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$#\"</b> interface class.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "/// @brief Extra transport fields of @ref #^#CLASS_NAME#$# interface class.\n"
    "/// @see @ref #^#CLASS_NAME#$#\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "struct #^#CLASS_NAME#$#Fields\n"
    "{\n"
    "    #^#FIELDS_DEF#$#\n"
    "    /// @brief All the fields bundled in std::tuple.\n"
    "    using All = std::tuple<\n"
    "        #^#FIELDS_LIST#$#\n"
    "    >;\n"
    "};\n\n"
    "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"</b> common interface class.\n"
    "#^#DOC_DETAILS#$#\n"
    "/// @tparam TOpt Interface definition options\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "template <typename... TOpt>\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::Message<\n"
    "        TOpt...,\n"
    "        #^#ENDIAN#$#,\n"
    "        comms::option::def::MsgIdType<#^#PROT_NAMESPACE#$#::MsgId>,\n"
    "        #^#FIELDS_OPTIONS#$#\n"
    "    >\n"
    "{\n"
    "    using Base =\n"
    "        comms::Message<\n"
    "            TOpt...,\n"
    "            #^#ENDIAN#$#,\n"
    "            comms::option::def::MsgIdType<#^#PROT_NAMESPACE#$#::MsgId>,\n"
    "            #^#FIELDS_OPTIONS#$#\n"
    "        >;\n"
    "public:\n"
    "    /// @brief Allow access to extra transport fields.\n"
    "    /// @details See definition of @b COMMS_MSG_TRANSPORT_FIELDS_NAMES macro\n"
    "    ///     related to @b comms::Message class from COMMS library\n"
    "    ///     for details.\n"
    "    ///\n"
    "    ///     The generated functions are:\n"
    "    #^#ACCESS_FUNCS_DOC#$#\n"
    "    COMMS_MSG_TRANSPORT_FIELDS_NAMES(\n"
    "        #^#FIELDS_ACCESS_LIST#$#\n"
    "    );\n"
    "    #^#ALIASES#$#\n"
    "    #^#PUBLIC#$#\n"
    "#^#PROTECTED#$#\n"
    "#^#PRIVATE#$#\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n"
);

std::string PluginHeaderTemplate = 
    "#pragma once\n\n"
    "#include \"comms_champion/MessageBase.h\"\n"
    "#include #^#INTERFACE_INCLUDE#$#\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms_champion::MessageBase<\n"
    "        #^#INTERFACE#$#\n"
    "    >\n"
    "{\n"
    "protected:\n"
    "    #^#ID_FUNC#$#\n"
    "    virtual const QVariantList& extraTransportFieldsPropertiesImpl() const override;\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n\n"
    "#^#APPEND#$#\n";

std::string PluginHeaderAliasTemplate = 
    "#pragma once\n\n"
    "#include \"comms_champion/MessageBase.h\"\n"
    "#include #^#INTERFACE_INCLUDE#$#\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "using #^#CLASS_NAME#$# =\n"
    "    comms_champion::MessageBase<\n"
    "        #^#INTERFACE#$#\n"
    "    >;\n\n"
    "#^#END_NAMESPACE#$#\n\n"
    "#^#APPEND#$#\n";

std::string PluginSrcTemplate = 
    "#include \"#^#CLASS_NAME#$#.h\"\n\n"
    "#include \"comms_champion/property/field.h\"\n"
    "#^#INCLUDES#$#\n"
    "namespace cc = comms_champion;\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "namespace\n"
    "{\n\n"
    "#^#FIELDS_PROPS#$#\n"
    "QVariantList createProps()\n"
    "{\n"
    "    QVariantList props;\n"
    "    #^#PROPS_APPENDS#$#\n"
    "    return props;\n"
    "}\n\n"
    "} // namespace \n\n"
    "#^#ID_FUNC#$#\n"
    "const QVariantList& #^#CLASS_NAME#$#::extraTransportFieldsPropertiesImpl() const\n"
    "{\n"
    "    static const QVariantList Props = createProps();\n"
    "    return Props;\n"
    "}\n\n"
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n";
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

        ptr->setMemberChild();
        ptr->setCommonPreDefDisabled();
        if (!ptr->prepare(0U)) {
            return false;
        }

        ptr->setForcedNoOptionsConfig();
        m_fields.push_back(std::move(ptr));
    }

    return true;
}

bool Interface::write()
{
    return
        writeProtocol() &&
        writePluginHeader() &&
        writePluginSrc();
}

const std::string& Interface::name() const
{
    if (!m_dslObj.valid()) {
        return common::messageClassStr();
    }

    return m_dslObj.name();
}

bool Interface::hasVersion() const
{
    return
        std::any_of(
            m_fields.begin(), m_fields.end(),
            [](auto& f)
            {
                return f->semanticType() == commsdsl::Field::SemanticType::Version;
            });
}

bool Interface::hasFields() const
{
    return !m_fields.empty();
}

std::vector<std::string> Interface::getVersionFields() const
{
    std::vector<std::string> result;
    for (auto& f : m_fields) {
        if (f->semanticType() == commsdsl::Field::SemanticType::Version) {
            result.push_back(f->name());
        }
    }
    return result;
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
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForInterface(m_externalRef)));

    auto namespaces = m_generator.namespacesForInterface(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto* templ = &AliasTemplate;
    bool useClass = false;
    if (!m_fields.empty()) {
        useClass = true;
        replacements.insert(std::make_pair("FIELDS_LIST", getFieldsClassesList()));
        replacements.insert(std::make_pair("FIELDS_ACCESS_LIST", getFieldsAccessList()));
        replacements.insert(std::make_pair("FIELDS_OPTIONS", getFieldsOpts()));
        replacements.insert(std::make_pair("ACCESS_FUNCS_DOC", getFieldsAccessDoc()));
        replacements.insert(std::make_pair("FIELDS_DEF", getFieldsDef()));
        replacements.insert(std::make_pair("ALIASES", getAliases()));
    }

    auto extraPublic = m_generator.getExtraPublicForInterface(externalRef());
    if (!extraPublic.empty()) {
        useClass = true;
        replacements.insert(std::make_pair("PUBLIC", "\n" + std::move(extraPublic)));
    }

    auto extraProtected = m_generator.getExtraProtectedForInterface(externalRef());
    if (!extraProtected.empty()) {
        useClass = true;
        common::insertIndent(extraProtected);
        replacements.insert(std::make_pair("PROTECTED", "\nprotected:\n" + std::move(extraProtected)));
    }

    auto extraPrivate = m_generator.getExtraPrivateForInterface(externalRef());
    if (!extraPrivate.empty()) {
        useClass = true;
        common::insertIndent(extraPrivate);
        replacements.insert(std::make_pair("PUBLIC", "\nprivate:\n" + std::move(extraPrivate)));
    }

    if (useClass) {
        templ = &ClassTemplate;
    }
    
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

bool Interface::writePluginHeader()
{
    auto startInfo = m_generator.startInterfacePluginHeaderWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("INTERFACE_INCLUDE", m_generator.headerfileForInterface(externalRef())));
    replacements.insert(std::make_pair("INTERFACE", m_generator.scopeForInterface(externalRef(), true, true)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForInterfaceHeaderInPlugin(m_externalRef)));

    auto namespaces = m_generator.namespacesForInterfaceInPlugin(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto hexWidth = getHexMsgIdWidth();
    auto* templ = &PluginHeaderAliasTemplate;
    if ((!m_fields.empty()) || (0U < hexWidth)) {
        templ = &PluginHeaderTemplate;
    }

    if (0U < hexWidth) {
        auto str = "virtual QString idAsStringImpl() const override;";
        replacements.insert(std::make_pair("ID_FUNC", std::move(str)));
    }

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

bool Interface::writePluginSrc()
{
    auto startInfo = m_generator.startInterfacePluginSrcWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    std::string str;
    do {
        auto hexWidth = getHexMsgIdWidth();
        if ((m_fields.empty()) && (hexWidth == 0U)) {
            break;
        }

        auto scope = m_generator.scopeForInterface(m_externalRef, true, false);
        scope += common::nameToClassCopy(name()) + common::fieldsSuffixStr() + "::";
        common::StringsList fieldsProps;
        common::StringsList appends;
        common::StringsList includes;
        fieldsProps.reserve(m_fields.size());
        fieldsProps.reserve(appends.size());
        for (auto& f : m_fields) {
            fieldsProps.push_back(f->getPluginCreatePropsFunc(scope, true, false));
            appends.push_back("props.append(createProps_" + common::nameToAccessCopy(f->name()) + "());");
            f->updatePluginIncludes(includes);
        }

        auto namespaces = m_generator.namespacesForInterfaceInPlugin(m_externalRef);

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("CLASS_NAME", className));
        replacements.insert(std::make_pair("INTERFACE", m_generator.scopeForInterface(externalRef(), true, true)));
        replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
        replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
        replacements.insert(std::make_pair("FIELDS_PROPS", common::listToString(fieldsProps, "\n", "\n")));
        replacements.insert(std::make_pair("PROPS_APPENDS", common::listToString(appends, "\n", common::emptyString())));
        replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForInterfaceSrcInPlugin(m_externalRef)));
        replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(includes)));

        if (0U < hexWidth) {
            auto func =
                "QString " + className + "::idAsStringImpl() const\n"
                "{\n"
                "    return \"0x\" + QString(\"%1\").arg(static_cast<unsigned long long>(getId()), " +
                std::to_string(hexWidth) + ", 16, QChar('0')).toUpper();\n"
                "}\n";
            replacements.insert(std::make_pair("ID_FUNC", std::move(func)));
        }

        str = common::processTemplate(PluginSrcTemplate, replacements);
    } while (false);

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

    auto desc = common::makeMultilineCopy(m_dslObj.description());
    if (!desc.empty()) {
        static const std::string DocPrefix("/// @details ");
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + common::doxygenPrefixStr() + "    ");
        ba::replace_all(desc, "\n", DocNewLineRepl);
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

std::string Interface::getFieldsAccessList() const
{
    std::string result;
    for (auto& f : m_fields) {
        if (!result.empty()) {
            result += ",\n";
        }
        result += common::nameToAccessCopy(f->name());
    }
    return result;
}

std::string Interface::getAliases() const
{
    auto aliases = m_dslObj.aliases();
    if (aliases.empty()) {
        return common::emptyString();
    }

    common::StringsList result;
    for (auto& a : aliases) {
        auto& fieldName = a.fieldName();
        assert(!fieldName.empty());

        auto dotPos = fieldName.find('.');
        std::string firstFieldName(fieldName, 0, dotPos);
        auto iter =
            std::find_if(
                m_fields.begin(), m_fields.end(),
                [&firstFieldName](auto& f)
                {
                    return firstFieldName == f->name();
                });

        if (iter == m_fields.end()) {
            continue;
        }

        std::string restFieldName;
        if (dotPos != std::string::npos) {
            restFieldName.assign(fieldName, dotPos + 1, fieldName.size());
        }

        if (!restFieldName.empty() && (!(*iter)->verifyAlias(restFieldName))) {
            continue;
        }

        static const std::string Templ =
            "/// @brief Alias to an extra transport member field.\n"
            "/// @details\n"
            "#^#ALIAS_DESC#$#\n"
            "///     Generates field access alias function(s):\n"
            "///     @b transportField_#^#ALIAS_NAME#$#() -> <b>transportField_#^#ALIASED_FIELD_DOC#$#</b>\n"
            "COMMS_MSG_TRANSPORT_FIELD_ALIAS(#^#ALIAS_NAME#$#, #^#ALIASED_FIELD#$#);\n";

        std::vector<std::string> aliasedFields;
        ba::split(aliasedFields, fieldName, ba::is_any_of("."));
        std::string aliasedFieldDocStr;
        std::string aliasedFieldStr;
        for (auto& f : aliasedFields) {
            common::nameToAccess(f);

            if (!aliasedFieldDocStr.empty()) {
                aliasedFieldDocStr += ".field_";
            }
            aliasedFieldDocStr += f + "()";

            if (!aliasedFieldStr.empty()) {
                aliasedFieldStr += ", ";
            }

            aliasedFieldStr += f;
        }

        auto desc = common::makeDoxygenMultilineCopy(a.description());
        if (!desc.empty()) {
            desc = common::doxygenPrefixStr() + common::indentStr() + desc + " @n";
        }

        common::ReplacementMap repl;
        repl.insert(std::make_pair("ALIAS_NAME", common::nameToAccessCopy(a.name())));
        repl.insert(std::make_pair("ALIASED_FIELD_DOC", std::move(aliasedFieldDocStr)));
        repl.insert(std::make_pair("ALIASED_FIELD", std::move(aliasedFieldStr)));
        repl.insert(std::make_pair("ALIAS_DESC", std::move(desc)));
        result.push_back(common::processTemplate(Templ, repl));
    }

    if (result.empty()) {
        return common::emptyString();
    }

    return '\n' + common::listToString(result, "\n", common::emptyString());
}

std::string Interface::getIncludes() const
{
    common::StringsList includes;
    for (auto& f : m_fields) {
        f->updateIncludes(includes);
    }

    if (!m_fields.empty()) {
        common::mergeInclude("<tuple>", includes);
    }

    static const common::StringsList InterfaceIncludes = {
        "comms/Message.h",
        "comms/options.h",
    };

    common::mergeIncludes(InterfaceIncludes, includes);
    common::mergeInclude(m_generator.mainNamespace() + '/' + common::msgIdEnumNameStr() + common::headerSuffix(), includes);
    return common::includesToStatements(includes) + m_generator.getExtraIncludeForInterface(externalRef());
}

std::string Interface::getFieldsAccessDoc() const
{
    if (m_fields.empty()) {
        return common::emptyString();
    }

    std::string result;
    for (auto& f : m_fields) {
        if (!result.empty()) {
            result += '\n';
        }
        result += common::doxygenPrefixStr();
        result += common::indentStr();
        result += "@li @b TransportField_";
        result += common::nameToAccessCopy(f->name());
        result +=  " type and @b transportField_";
        result += common::nameToAccessCopy(f->name());
        result += "() function for\n";
        result += common::doxygenPrefixStr();
        result += common::indentStr();
        result += common::indentStr();
        result += "@ref ";
        result += common::nameToClassCopy(m_dslObj.name());
        result += "Fields::";
        result += common::nameToClassCopy(f->name());
        result += " field.";
    }

    return result;
}

std::string Interface::getFieldsDef() const
{
    std::string result;

    for (auto& f : m_fields) {
        result += f->getClassDefinition(common::emptyString());
        if (&f != &m_fields.back()) {
            result += '\n';
        }
    }
    return result;
}

std::string Interface::getFieldsOpts() const
{
    std::string result =
        "comms::option::def::ExtraTransportFields<" +
        common::nameToClassCopy(m_dslObj.name()) + 
        common::fieldsSuffixStr() + 
        "::All>";

    auto iter =
        std::find_if(
            m_fields.begin(), m_fields.end(),
            [](auto& f)
            {
                return f->semanticType() == commsdsl::Field::SemanticType::Version;
            });

    if (iter != m_fields.end()) {
        result += ",\n";
        result += "comms::option::def::VersionInExtraTransportFields<";
        result += common::numToString(static_cast<std::size_t>(std::distance(m_fields.begin(), iter)));
        result += ">";
    }
    return result;
}

unsigned Interface::getHexMsgIdWidth() const
{
    auto* msgIdField = m_generator.getMessageIdField();
    if (msgIdField == nullptr) {
        return 0U;
    }

    if (msgIdField->kind() != commsdsl::Field::Kind::Enum) {
        return 0U;
    }

    auto* enumMsgIdField = static_cast<const EnumField*>(msgIdField);
    return enumMsgIdField->hexWidth();
}

}
