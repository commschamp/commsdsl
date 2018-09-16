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
    "/// @brief Contains definition of <b>\"#^#MESSAGE_NAME#$#\"</b> message and its fields.\n"
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
    "struct #^#ORIG_CLASS_NAME#$#Fields\n"
    "{\n"
    "    #^#FIELDS_DEF#$#\n"
    "    /// @brief All the fields bundled in std::tuple.\n"
    "    using All = std::tuple<\n"
    "        #^#FIELDS_LIST#$#\n"
    "    >;\n"
    "};\n"
    "\n"
    "/// @brief Definition of <b>\"#^#MESSAGE_NAME#$#\"</b> message class.\n"
    "/// @details\n"
    "#^#DOC_DETAILS#$#\n"
    "///     See @ref #^#ORIG_CLASS_NAME#$#Fields for definition of the fields this message contains.\n"
    "/// @tparam TMsgBase Base (interface) class.\n"
    "/// @tparam TOpt Extra options\n"
    "/// @headerfile #^#MESSAGE_HEADERFILE#$#\n"
    "template <typename TMsgBase, typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions>\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::MessageBase<\n"
    "        TMsgBase,\n"
    "        #^#CUSTOMIZATION_OPT#$#\n"
    "        comms::option::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
    "        comms::option::FieldsImpl<typename #^#ORIG_CLASS_NAME#$#Fields<TOpt>::All>,\n"
    "        comms::option::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
    "        comms::option::HasName#^#COMMA#$#\n"
    "        #^#EXTRA_OPTIONS#$#\n"
    "    >\n"
    "{\n"
    "    // Redefinition of the base class type\n"
    "    using Base =\n"
    "        comms::MessageBase<\n"
    "            TMsgBase,\n"
    "            #^#CUSTOMIZATION_OPT#$#\n"
    "            comms::option::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
    "            comms::option::FieldsImpl<typename #^#ORIG_CLASS_NAME#$#Fields<TOpt>::All>,\n"
    "            comms::option::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
    "            comms::option::HasName#^#COMMA#$#\n"
    "            #^#EXTRA_OPTIONS#$#\n"
    "        >;\n"
    "\n"
    "public:\n"
    "#^#MESSAGE_BODY#$#\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
);

static const std::string PluginSingleInterfaceHeaderTemplate(
    "#pragma once\n\n"
    "#include <QtCore/QVariantList>\n"
    "#include \"comms_champion/ProtocolMessageBase.h\"\n"
    "#include #^#MESSAGE_INC#$#\n"
    "#include #^#INTERFACE_INC#$#\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms_champion::ProtocolMessageBase<\n"
    "        #^#PROT_MESSAGE#$#<#^#INTERFACE#$#>,\n"
    "        #^#CLASS_NAME#$#\n"
    "    >\n"
    "{\n"
    "protected:\n"
    "    virtual const QVariantList& fieldsPropertiesImpl() const override;\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n\n"
    "extern template class #^#PROT_MESSAGE#$#<#^#INTERFACE#$#>;\n"
    "extern template class comms_champion::ProtocolMessageBase<\n"
    "    #^#PROT_MESSAGE#$#<#^#INTERFACE#$#>,\n"
    "    #^#CLASS_PLUGIN_SCOPE#$##^#CLASS_NAME#$#\n"
    ">;\n\n"
);

static const std::string PluginMultiInterfaceHeaderTemplate(
    "#pragma once\n\n"
    "#include <QtCore/QVariantList>\n"
    "#include \"comms_champion/ProtocolMessageBase.h\"\n"
    "#include #^#MESSAGE_INC#$#\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "struct #^#CLASS_NAME#$#Fields\n"
    "{\n"
    "    static const QVariantList& props();\n"
    "};\n\n"
    "template <typename TIterface>\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms_champion::ProtocolMessageBase<\n"
    "        #^#PROT_MESSAGE#$#<TIterface>,\n"
    "        #^#CLASS_NAME#$#<TIterface>\n"
    "    >\n"
    "{\n"
    "protected:\n"
    "    virtual const QVariantList& fieldsPropertiesImpl() const override\n"
    "    {\n"
    "        return #^#CLASS_NAME#$#Fields::props();\n"
    "    }\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
);

static const std::string PluginSingleInterfaceSrcTemplate(
    "#include \"#^#CLASS_NAME#$#.h\"\n\n"
    "#include \"comms_champion/property/field.h\"\n"
    "#^#INCLUDES#$#\n"
    "namespace cc = comms_champion;\n\n"
    "template class #^#PROT_MESSAGE#$#<#^#INTERFACE#$#>;\n"
    "template class cc::ProtocolMessageBase<\n"
    "    #^#PROT_MESSAGE#$#<#^#INTERFACE#$#>,\n"
    "    #^#CLASS_PLUGIN_SCOPE#$##^#CLASS_NAME#$#\n"
    ">;\n\n"    
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
    "} // namespace\n\n"
    "const QVariantList& #^#CLASS_NAME#$#::fieldsPropertiesImpl() const\n"
    "{\n"
    "    static const QVariantList Props = createProps();\n"
    "    return Props;\n"
    "}\n\n"
    "#^#END_NAMESPACE#$#\n"
);

static const std::string PluginMultiInterfaceSrcTemplate(
    "#include \"#^#CLASS_NAME#$#.h\"\n\n"
    "#include \"comms_champion/property/field.h\"\n\n"
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
    "} // namespace\n\n"
    "const QVariantList& #^#CLASS_NAME#$#Fields::props()\n"
    "{\n"
    "    static const QVariantList Props = createProps();\n"
    "    return Props;\n"
    "}\n\n"
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

        if (!ptr->prepare(m_dslObj.sinceVersion())) {
            return false;
        }
        m_fields.push_back(std::move(ptr));
    }

    m_customRefresh = m_generator.getCustomRefreshForMessage(m_externalRef);
    return true;
}

bool Message::doesExist() const
{
    bool exists =
        m_generator.doesElementExist(
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
            m_dslObj.isDeprecatedRemoved());
    return exists;
}

bool Message::write()
{
    if (!doesExist()) {
        return true;
    }

    return
        writeProtocol() &&
        writePluginHeader() &&
        writePluginSrc();
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

    auto scope = m_generator.scopeForMessage(m_externalRef, true, true) + common::fieldsSuffixStr() + "::";
    for (auto& f : m_fields) {
        addFieldOptsFunc(f->getDefaultOptions(scope));
    }

    static const std::string Templ =
        "/// @brief Extra options for fields of @ref #^#MESSAGE_SCOPE#$# message.\n"
        "struct #^#MESSAGE_NAME#$#Fields\n"
        "{\n"
        "    #^#FIELDS_OPTS#$#\n"
        "}; // struct #^#MESSAGE_NAME#$#Fields\n\n"
        "#^#MESSAGE_OPT#$#\n";

    static const std::string NoFieldsTempl =
        "#^#MESSAGE_OPT#$#\n";

    auto* templ = &Templ;
    if (m_fields.empty() || fieldsOpts.empty()) {
        templ = &NoFieldsTempl;
    }

    bool customizable = isCustomizable();
    if ((!customizable) && (templ == &NoFieldsTempl)) {
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("MESSAGE_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("FIELDS_OPTS", std::move(fieldsOpts)));
    replacements.insert(std::make_pair("MESSAGE_SCOPE", m_generator.scopeForMessage(m_externalRef, true, true)));

    if (customizable) {
        static const std::string OptTempl = 
            "/// @brief Extra options for @ref #^#MESSAGE_SCOPE#$# message.\n"
            "using #^#MESSAGE_NAME#$# = comms::option::EmptyOption;";
        replacements.insert(std::make_pair("MESSAGE_OPT", common::processTemplate(OptTempl, replacements)));
    }

    return common::processTemplate(*templ, replacements);
}

std::string Message::getClientOptions() const
{
    if ((m_dslObj.sender() == Sender::Both) || (!isCustomizable())) {
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("MESSAGE_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("MESSAGE_SCOPE", m_generator.scopeForMessage(m_externalRef, true, true)));

    if (m_dslObj.sender() == Sender::Client) {
        static const std::string Templ = 
            "/// @brief Extra options for @ref #^#MESSAGE_SCOPE#$# message.\n"
            "using #^#MESSAGE_NAME#$# = comms::option::NoReadImpl;\n";

        return common::processTemplate(Templ, replacements);
    }

    assert(m_dslObj.sender() == Sender::Server);
    static const std::string Templ = 
        "/// @brief Extra options for @ref #^#MESSAGE_SCOPE#$# message.\n"
        "using #^#MESSAGE_NAME#$# =\n"
        "    std::tuple<\n"
        "        comms::option::NoWriteImpl,\n"
        "        comms::option::NoRefreshImpl\n"
        "    >;\n";

    return common::processTemplate(Templ, replacements);
}

std::string Message::getServerOptions() const
{
    if ((m_dslObj.sender() == Sender::Both) || (!isCustomizable())) {
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("MESSAGE_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("MESSAGE_SCOPE", m_generator.scopeForMessage(m_externalRef, true, true)));

    if (m_dslObj.sender() == Sender::Client) {
        static const std::string Templ = 
            "/// @brief Extra options for @ref #^#MESSAGE_SCOPE#$# message.\n"
            "using #^#MESSAGE_NAME#$# =\n"
            "    std::tuple<\n"
            "        comms::option::NoWriteImpl,\n"
            "        comms::option::NoRefreshImpl\n"
            "    >;\n";        

        return common::processTemplate(Templ, replacements);
    }

    assert(m_dslObj.sender() == Sender::Server);
    static const std::string Templ = 
        "/// @brief Extra options for @ref #^#MESSAGE_SCOPE#$# message.\n"
        "using #^#MESSAGE_NAME#$# = comms::option::NoReadImpl;\n";

    return common::processTemplate(Templ, replacements);
}

bool Message::writeProtocol()
{
    assert(!m_externalRef.empty());

    auto names = m_generator.startMessageProtocolWrite(m_externalRef);
    auto& filePath = names.first;
    auto& className = names.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("ORIG_CLASS_NAME", common::nameToClassCopy(name())));
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
    replacements.insert(std::make_pair("EXTRA_OPTIONS", getExtraOptions()));
    if (!replacements["EXTRA_OPTIONS"].empty()) {
        replacements.insert(std::make_pair("COMMA", ","));
    }

    if (isCustomizable()) {
        auto opt = "typename TOpt::" + getNamespaceScope() + ",";
        replacements.insert(std::make_pair("CUSTOMIZATION_OPT", std::move(opt)));
    }

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

bool Message::writePluginHeader()
{
    assert(!m_externalRef.empty());

    auto startInfo = m_generator.startMessagePluginHeaderWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("MESSAGE_INC", m_generator.headerfileForMessage(m_externalRef, true)));
    replacements.insert(std::make_pair("PROT_MESSAGE", m_generator.scopeForMessage(m_externalRef, true, true)));
    replacements.insert(std::make_pair("CLASS_PLUGIN_SCOPE", m_generator.scopeForMessageInPlugin(m_externalRef, true, false)));

    auto namespaces = m_generator.namespacesForMessageInPlugin(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto* templ = &PluginMultiInterfaceHeaderTemplate;
    auto* defaultInterface = m_generator.getDefaultInterface();
    if (defaultInterface != nullptr) {
        replacements.insert(std::make_pair("INTERFACE_INC", m_generator.headerfileForInterfaceInPlugin(defaultInterface->externalRef())));
        replacements.insert(std::make_pair("INTERFACE", m_generator.scopeForInterfaceInPlugin(defaultInterface->externalRef())));
        templ = &PluginSingleInterfaceHeaderTemplate;
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

bool Message::writePluginSrc()
{
    assert(!m_externalRef.empty());

    auto startInfo = m_generator.startMessagePluginSrcWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::StringsList fieldsProps;
    common::StringsList appends;
    common::StringsList includes;
    fieldsProps.reserve(m_fields.size());
    appends.reserve(m_fields.size());
    auto scope = m_generator.scopeForMessage(m_externalRef, true, true) + common::fieldsSuffixStr() + "<>::";
    for (auto& f : m_fields) {
        fieldsProps.push_back(f->getPluginCreatePropsFunc(scope, false, false));
        appends.push_back("props.append(createProps_" + common::nameToAccessCopy(f->name()) + "());");
        f->updatePluginIncludes(includes);
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("FIELDS_PROPS", common::listToString(fieldsProps, "\n", common::emptyString())));
    replacements.insert(std::make_pair("PROPS_APPENDS", common::listToString(appends, "\n", common::emptyString())));
    replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(includes)));
    replacements.insert(std::make_pair("CLASS_PLUGIN_SCOPE", m_generator.scopeForMessageInPlugin(m_externalRef, true, false)));

    auto namespaces = m_generator.namespacesForMessageInPlugin(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto* templ = &PluginMultiInterfaceSrcTemplate;
    auto* defaultInterface = m_generator.getDefaultInterface();
    if (defaultInterface != nullptr) {
        replacements.insert(std::make_pair("PROT_MESSAGE", m_generator.scopeForMessage(m_externalRef, true, true)));
        replacements.insert(std::make_pair("INTERFACE", m_generator.scopeForInterfaceInPlugin(defaultInterface->externalRef())));
        templ = &PluginSingleInterfaceSrcTemplate;
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

const std::string& Message::getDisplayName() const
{
    return common::displayName(m_dslObj.displayName(), m_dslObj.name());
}

std::string Message::getDescription() const
{
    auto desc = common::makeMultilineCopy(m_dslObj.description());
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
        m_generator.mainNamespace() + '/' + common::msgIdEnumNameStr() + common::headerSuffix(),
        m_generator.mainNamespace() + '/' + common::defaultOptionsStr() + common::headerSuffix()
    };
    common::mergeIncludes(MessageIncludes, includes);

    return common::includesToStatements(includes);
}

std::string Message::getBody() const
{
    std::string result = getPublic();
    common::insertIndent(result);
    std::string prot = getProtected();
    if (!prot.empty()) {
        common::insertIndent(prot);
        result += "\nprotected:\n";
        result += prot;
    }

    std::string priv = getPrivate();
    if (!priv.empty()) {
        common::insertIndent(priv);
        result += "\nprivate:\n";
        result += priv;
    }

    return result;
}

std::string Message::getPublic() const
{
    static const std::string Templ =
        "#^#ACCESS#$#\n"
        "#^#LENGTH_CHECK#$#\n"
        "#^#EXTRA#$#\n"
        "#^#NAME#$#\n"
        "#^#READ#$#\n"
        "#^#WRITE#$#\n"
        "#^#LENGTH#$#\n"
        "#^#VALID#$#\n"
        "#^#REFRESH#$#\n";
    
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ACCESS", getFieldsAccess()));
    replacements.insert(std::make_pair("LENGTH_CHECK", getLengthCheck()));
    replacements.insert(std::make_pair("EXTRA", getExtraPublic()));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getReadFunc()));
    replacements.insert(std::make_pair("WRITE", m_generator.getCustomWriteForMessage(m_externalRef)));
    replacements.insert(std::make_pair("LENGTH", m_generator.getCustomLengthForMessage(m_externalRef)));
    replacements.insert(std::make_pair("VALID", m_generator.getCustomValidForMessage(m_externalRef)));
    replacements.insert(std::make_pair("REFRESH", getRefreshFunc()));

    return common::processTemplate(Templ, replacements);
}

std::string Message::getProtected() const
{
    return m_generator.getExtraProtectedForMessage(m_externalRef);
}

std::string Message::getPrivate() const
{
    auto extra = m_generator.getExtraPrivateForMessage(m_externalRef);
    auto privateRefresh = Field::getPrivateRefreshForFields(m_fields);
    if ((!extra.empty()) && (!privateRefresh.empty())) {
        extra += '\n';
    }

    return extra + privateRefresh;
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
    result += ");\n";

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
    auto str = m_generator.getCustomNameForMessage(m_externalRef);
    if (!str.empty()) {
        return str;
    }

    return
        "/// @brief Name of the message.\n"
        "static const char* doName()\n"
        "{\n"
        "    return \"" + getDisplayName() + "\";\n"
        "}\n";
}

std::string Message::getReadFunc() const
{
    auto str = m_generator.getCustomReadForMessage(m_externalRef);
    if (!str.empty()) {
        return str;
    }

    return Field::getReadForFields(m_fields, true, m_generator.versionDependentCode());
}

std::string Message::getRefreshFunc() const
{
    if (!m_customRefresh.empty()) {
        return m_customRefresh;
    }

    return Field::getPublicRefreshForFields(m_fields, true);
}

std::string Message::getExtraOptions() const
{
    if ((!m_customRefresh.empty()) || (mustImplementReadRefresh())) {
        return "comms::option::HasCustomRefresh";
    }

    return common::emptyString();
}

std::string Message::getExtraPublic() const
{
    auto str = m_generator.getExtraPublicForMessage(m_externalRef);
    if (str.empty()) {
        return str;
    }

    return "\n" + str;
}

bool Message::mustImplementReadRefresh() const
{
    for (auto& f : m_fields) {
        if (f->hasCustomReadRefresh()) {
            return true;
        }
    }
    return false;
}

bool Message::isCustomizable() const
{
    if (m_generator.customizationLevel() == CustomizationLevel::Full) {
        return true;
    }

    if (m_dslObj.isCustomizable()) {
        return true;
    }

    if (m_generator.customizationLevel() == CustomizationLevel::None) {
        return false;
    }

    return m_dslObj.sender() != commsdsl::Message::Sender::Both;
}

}
