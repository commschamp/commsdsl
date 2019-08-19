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
    "#^#COMMON_PRE_DEF#$#\n"
    "/// @brief Fields of @ref #^#CLASS_NAME#$#.\n"
    "/// @tparam TOpt Extra options\n"
    "/// @see @ref #^#CLASS_NAME#$#\n"
    "/// @headerfile #^#MESSAGE_HEADERFILE#$#\n"
    "template <typename TOpt = #^#OPTIONS#$#>\n"
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
    "template <typename TMsgBase, typename TOpt = #^#OPTIONS#$#>\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::MessageBase<\n"
    "        TMsgBase,\n"
    "        #^#CUSTOMIZATION_OPT#$#\n"
    "        comms::option::def::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
    "        comms::option::def::FieldsImpl<typename #^#ORIG_CLASS_NAME#$#Fields<TOpt>::All>,\n"
    "        comms::option::def::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
    "        comms::option::def::HasName#^#COMMA#$#\n"
    "        #^#EXTRA_OPTIONS#$#\n"
    "    >\n"
    "{\n"
    "    // Redefinition of the base class type\n"
    "    using Base =\n"
    "        comms::MessageBase<\n"
    "            TMsgBase,\n"
    "            #^#CUSTOMIZATION_OPT#$#\n"
    "            comms::option::def::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
    "            comms::option::def::FieldsImpl<typename #^#ORIG_CLASS_NAME#$#Fields<TOpt>::All>,\n"
    "            comms::option::def::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
    "            comms::option::def::HasName#^#COMMA#$#\n"
    "            #^#EXTRA_OPTIONS#$#\n"
    "        >;\n"
    "\n"
    "public:\n"
    "#^#MESSAGE_BODY#$#\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n"
);

static const std::string PluginSingleInterfacePimplHeaderTemplate(
    "#pragma once\n\n"
    "#include <memory>\n"
    "#include <QtCore/QVariantList>\n"
    "#include #^#INTERFACE_INC#$#\n\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "class #^#CLASS_NAME#$#Impl;\n"
    "class #^#CLASS_NAME#$# : public #^#INTERFACE#$#\n"
    "{\n"
    "public:\n"
    "    #^#CLASS_NAME#$#();\n"
    "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = delete;\n"
    "    #^#CLASS_NAME#$#(#^#CLASS_NAME#$#&&) = delete;\n"
    "    virtual ~#^#CLASS_NAME#$#();\n"
    "    #^#CLASS_NAME#$#& operator=(const #^#CLASS_NAME#$#& other);\n"
    "    #^#CLASS_NAME#$#& operator=(#^#CLASS_NAME#$#&&);\n\n"
    "protected:\n"
    "    virtual const char* nameImpl() const override;\n"
    "    virtual const QVariantList& fieldsPropertiesImpl() const override;\n"
    "    virtual void dispatchImpl(comms_champion::MessageHandler& handler) override;\n"
    "    virtual void resetImpl() override;\n"
    "    virtual bool assignImpl(const comms_champion::Message& other) override;\n"
    "    virtual MsgIdParamType getIdImpl() const override;\n"
    "    virtual comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t len) override;\n"
    "    virtual comms::ErrorStatus writeImpl(WriteIterator& iter, std::size_t len) const override;\n"
    "    virtual bool validImpl() const override;\n"
    "    virtual std::size_t lengthImpl() const override;\n"
    "    virtual bool refreshImpl() override;\n\n"
    "private:\n"
    "    std::unique_ptr<#^#CLASS_NAME#$#Impl> m_pImpl;\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n\n"
    "#^#APPEND#$#\n"
);

static const std::string PluginSingleInterfaceHeaderTemplate(
    "#pragma once\n\n"
    "#include <memory>\n"
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
    "public:\n"
    "    #^#CLASS_NAME#$#();\n"
    "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&) = delete;\n"
    "    #^#CLASS_NAME#$#(#^#CLASS_NAME#$#&&) = delete;\n"
    "    virtual ~#^#CLASS_NAME#$#();\n"
    "    #^#CLASS_NAME#$#& operator=(const #^#CLASS_NAME#$#&);\n"
    "    #^#CLASS_NAME#$#& operator=(#^#CLASS_NAME#$#&&);\n\n"
    "protected:\n"
    "    virtual const QVariantList& fieldsPropertiesImpl() const override;\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n\n"
    "#^#APPEND#$#\n"
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
    "#^#APPEND#$#\n"
);

static const std::string PluginSingleInterfacePimplSrcTemplate(
    "#include \"#^#CLASS_NAME#$#.h\"\n\n"
    "#include \"comms_champion/property/field.h\"\n"
    "#include \"comms_champion/ProtocolMessageBase.h\"\n"
    "#include #^#MESSAGE_INC#$#\n"
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
    "class #^#CLASS_NAME#$#Impl : public\n"
    "    comms_champion::ProtocolMessageBase<\n"
    "        #^#PROT_MESSAGE#$#<#^#INTERFACE#$#>,\n"
    "        #^#CLASS_NAME#$#Impl\n"
    "    >\n"
    "{\n"
    "public:\n"
    "    #^#CLASS_NAME#$#Impl() = default;\n"
    "    #^#CLASS_NAME#$#Impl(const #^#CLASS_NAME#$#Impl&) = delete;\n"
    "    #^#CLASS_NAME#$#Impl(#^#CLASS_NAME#$#Impl&&) = delete;\n"
    "    virtual ~#^#CLASS_NAME#$#Impl() = default;\n"
    "    #^#CLASS_NAME#$#Impl& operator=(const #^#CLASS_NAME#$#Impl&) = default;\n"
    "    #^#CLASS_NAME#$#Impl& operator=(#^#CLASS_NAME#$#Impl&&) = default;\n\n"
    "protected:\n"
    "    virtual const QVariantList& fieldsPropertiesImpl() const override\n"
    "    {\n"
    "        static const QVariantList Props = createProps();\n"
    "        return Props;\n"
    "    }\n"
    "};\n\n"
    "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() : m_pImpl(new #^#CLASS_NAME#$#Impl) {}\n"
    "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n\n"
    "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(const #^#CLASS_NAME#$#& other)\n"
    "{\n"
    "    *m_pImpl = *other.m_pImpl;\n"
    "    return *this;\n"
    "}\n\n"
    "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(#^#CLASS_NAME#$#&& other)\n"
    "{\n"
    "    *m_pImpl = std::move(*other.m_pImpl);\n"
    "    return *this;\n"
    "}\n\n"
    "const char* #^#CLASS_NAME#$#::nameImpl() const\n"
    "{\n"
    "    return static_cast<const cc::Message*>(m_pImpl.get())->name();\n"
    "}\n\n"
    "const QVariantList& #^#CLASS_NAME#$#::fieldsPropertiesImpl() const\n"
    "{\n"
    "    return m_pImpl->fieldsProperties();\n"
    "}\n\n"
    "void #^#CLASS_NAME#$#::dispatchImpl(cc::MessageHandler& handler)\n"
    "{\n"
    "    static_cast<cc::Message*>(m_pImpl.get())->dispatch(handler);\n"
    "}\n\n"
    "void #^#CLASS_NAME#$#::resetImpl()\n"
    "{\n"
    "    m_pImpl->reset();\n"
    "}\n\n"
    "bool #^#CLASS_NAME#$#::assignImpl(const cc::Message& other)\n"
    "{\n"
    "    auto* castedOther = dynamic_cast<const #^#CLASS_NAME#$#*>(&other);\n"
    "    if (castedOther == nullptr) {\n"
    "        return false;\n"
    "    }\n"
    "    return m_pImpl->assign(*castedOther->m_pImpl);\n"
    "}\n\n"
    "#^#CLASS_NAME#$#::MsgIdParamType #^#CLASS_NAME#$#::getIdImpl() const\n"
    "{\n"
    "    return m_pImpl->getId();\n"
    "}\n\n"
    "comms::ErrorStatus #^#CLASS_NAME#$#::readImpl(ReadIterator& iter, std::size_t len)\n"
    "{\n"
    "    return m_pImpl->read(iter, len);\n"
    "}\n\n"
    "comms::ErrorStatus #^#CLASS_NAME#$#::writeImpl(WriteIterator& iter, std::size_t len) const\n"
    "{\n"
    "    return m_pImpl->write(iter, len);\n"
    "}\n\n"
    "bool #^#CLASS_NAME#$#::validImpl() const\n"
    "{\n"
    "    return m_pImpl->valid();\n"
    "}\n\n"
    "std::size_t #^#CLASS_NAME#$#::lengthImpl() const\n"
    "{\n"
    "    return m_pImpl->length();\n"
    "}\n\n"  
    "bool #^#CLASS_NAME#$#::refreshImpl()\n"
    "{\n"
    "    return m_pImpl->refresh();\n"
    "}\n\n"  
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n"
);

static const std::string PluginSingleInterfaceSrcTemplate(
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
    "} // namespace\n\n"
    "#^#CLASS_NAME#$#::#^#CLASS_NAME#$#() = default;\n"
    "#^#CLASS_NAME#$#::~#^#CLASS_NAME#$#() = default;\n"
    "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(const #^#CLASS_NAME#$#&) = default;\n"
    "#^#CLASS_NAME#$#& #^#CLASS_NAME#$#::operator=(#^#CLASS_NAME#$#&&) = default;\n\n"
    "const QVariantList& #^#CLASS_NAME#$#::fieldsPropertiesImpl() const\n"
    "{\n"
    "    static const QVariantList Props = createProps();\n"
    "    return Props;\n"
    "}\n\n"
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n"
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
    "#^#APPEND#$#\n"
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

        ptr->setMemberChild();
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
    return getOptions(&Field::getDefaultOptions);
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
            "/// @brief Extra options for\n"
            "///     @ref #^#MESSAGE_SCOPE#$# message.\n"
            "using #^#MESSAGE_NAME#$# =\n"
            "    std::tuple<\n"
            "        comms::option::app::NoReadImpl,\n"
            "        comms::option::app::NoDispatchImpl\n"
            "    >;\n";

        return common::processTemplate(Templ, replacements);
    }

    assert(m_dslObj.sender() == Sender::Server);
    static const std::string Templ = 
        "/// @brief Extra options for\n"
        "///     @ref #^#MESSAGE_SCOPE#$# message.\n"
        "using #^#MESSAGE_NAME#$# =\n"
        "    std::tuple<\n"
        "        comms::option::app::NoWriteImpl,\n"
        "        comms::option::app::NoRefreshImpl\n"
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
            "/// @brief Extra options for\n"
            "///     @ref #^#MESSAGE_SCOPE#$# message.\n"
            "using #^#MESSAGE_NAME#$# =\n"
            "    std::tuple<\n"
            "        comms::option::app::NoWriteImpl,\n"
            "        comms::option::app::NoRefreshImpl\n"
            "    >;\n";        

        return common::processTemplate(Templ, replacements);
    }

    assert(m_dslObj.sender() == Sender::Server);
    static const std::string Templ = 
        "/// @brief Extra options for\n"
        "///     @ref #^#MESSAGE_SCOPE#$# message.\n"
        "using #^#MESSAGE_NAME#$# =\n"
        "    std::tuple<\n"
        "        comms::option::app::NoReadImpl,\n"
        "        comms::option::app::NoDispatchImpl\n"
        "    >;\n";

    return common::processTemplate(Templ, replacements);
}

std::string Message::getBareMetalDefaultOptions() const
{
    return getOptions(&Field::getBareMetalDefaultOptions);
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
    replacements.insert(std::make_pair("COMMON_PRE_DEF", getCommonPreDef()));
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
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForMessage(m_externalRef)));
    replacements.insert(std::make_pair("OPTIONS", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
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
    replacements.insert(std::make_pair("CLASS_PLUGIN_SCOPE", m_generator.scopeForMessageInPlugin(m_externalRef, true, false)));
    replacements.insert(std::make_pair("PROT_MESSAGE", m_generator.scopeForMessage(m_externalRef, true, true)));
    replacements.insert(std::make_pair("MESSAGE_INC", m_generator.headerfileForMessage(m_externalRef, true)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForMessageHeaderInPlugin(m_externalRef)));

    auto namespaces = m_generator.namespacesForMessageInPlugin(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto* templ = &PluginMultiInterfaceHeaderTemplate;
    auto* defaultInterface = m_generator.getDefaultInterface();
    if (defaultInterface != nullptr) {
        replacements.insert(std::make_pair("INTERFACE_INC", m_generator.headerfileForInterfaceInPlugin(defaultInterface->externalRef())));
        replacements.insert(std::make_pair("INTERFACE", m_generator.scopeForInterfaceInPlugin(defaultInterface->externalRef())));
        templ = &PluginSingleInterfacePimplHeaderTemplate;

        if (defaultInterface->hasFields()) {
            templ = &PluginSingleInterfaceHeaderTemplate;
        }
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
    replacements.insert(std::make_pair("PROT_MESSAGE", m_generator.scopeForMessage(m_externalRef, true, true)));
    replacements.insert(std::make_pair("FIELDS_PROPS", common::listToString(fieldsProps, "\n", common::emptyString())));
    replacements.insert(std::make_pair("PROPS_APPENDS", common::listToString(appends, "\n", common::emptyString())));
    replacements.insert(std::make_pair("MESSAGE_INC", m_generator.headerfileForMessage(m_externalRef, true)));
    replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(includes)));
    replacements.insert(std::make_pair("CLASS_PLUGIN_SCOPE", m_generator.scopeForMessageInPlugin(m_externalRef, true, false)));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForMessageSrcInPlugin(m_externalRef)));

    auto namespaces = m_generator.namespacesForMessageInPlugin(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto* templ = &PluginMultiInterfaceSrcTemplate;
    auto* defaultInterface = m_generator.getDefaultInterface();
    if (defaultInterface != nullptr) {
        replacements.insert(std::make_pair("PROT_MESSAGE", m_generator.scopeForMessage(m_externalRef, true, true)));
        replacements.insert(std::make_pair("INTERFACE", m_generator.scopeForInterfaceInPlugin(defaultInterface->externalRef())));
        templ = &PluginSingleInterfacePimplSrcTemplate;

        if (defaultInterface->hasFields()) {
            templ = &PluginSingleInterfaceSrcTemplate;
        }
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
        m_generator.headerfileForOptions(common::defaultOptionsStr(), false)
    };
    common::mergeIncludes(MessageIncludes, includes);

    return common::includesToStatements(includes) + m_generator.getExtraIncludeForMessage(m_externalRef);
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
        return "comms::option::def::HasCustomRefresh";
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

std::string Message::getCommonPreDef() const
{
    common::StringsList defs;
    for (auto& f : m_fields) {
        auto str = f->getCommonPreDefinition();
        if (!str.empty()) {
            defs.emplace_back(std::move(str));
        }
    }

    if (defs.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Common fields definitions from @ref #^#CLASS_NAME#$#Fields.\n"
        "/// @see @ref #^#CLASS_NAME#$#Fields\n"
        "/// @headerfile #^#MESSAGE_HEADERFILE#$#\n"
        "struct #^#ORIG_CLASS_NAME#$#FieldsCommon\n"
        "{\n"
        "    #^#FIELDS_DEF#$#\n"
        "};\n";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    repl.insert(std::make_pair("MESSAGE_HEADERFILE", m_generator.headerfileForMessage(m_externalRef)));
    repl.insert(std::make_pair("FIELDS_DEF", common::listToString(defs, "\n", common::emptyString())));
    return common::processTemplate(Templ, repl);
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

std::string Message::getOptions(GetFieldOptionsFunc func) const
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
        addFieldOptsFunc((f.get()->*func)(scope));
    }

    static const std::string Templ =
        "/// @brief Extra options for fields of\n"
        "///     @ref #^#MESSAGE_SCOPE#$# message.\n"
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
            "/// @brief Extra options for\n"
            "///     @ref #^#MESSAGE_SCOPE#$# message.\n"
            "using #^#MESSAGE_NAME#$# = comms::option::app::EmptyOption;";
        replacements.insert(std::make_pair("MESSAGE_OPT", common::processTemplate(OptTempl, replacements)));
    }

    return common::processTemplate(*templ, replacements);
}

}
