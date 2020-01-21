//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "Frame.h"

#include <cassert>
#include <fstream>
#include <map>
#include <algorithm>
#include <iterator>
#include <numeric>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "ChecksumLayer.h"
#include "CustomLayer.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string Template(
    "#^#GEN_COMMENT#$#\n"
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$#\"</b> frame class.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "/// @brief Layers definition of @ref #^#CLASS_NAME#$# frame class.\n"
    "/// @tparam TOpt Protocol options.\n"
    "/// @see @ref #^#CLASS_NAME#$#\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "template <typename TOpt = #^#OPTIONS#$#>\n"
    "struct #^#ORIG_CLASS_NAME#$#Layers\n"
    "{\n"
    "    #^#LAYERS_DEF#$#\n"
    "};\n\n"
    "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"</b> frame class.\n"
    "#^#DOC_DETAILS#$#\n"
    "/// @tparam TMessage Common interface class of all the messages\n"
    "#^#INPUT_MESSAGES_DOC#$#\n"
    "/// @tparam TOpt Frame definition options\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "template <\n"
    "   typename TMessage,\n"
    "   #^#INPUT_MESSAGES#$#\n"
    "   typename TOpt = #^#OPTIONS#$#\n"
    ">\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    #^#FRAME_DEF#$#\n"
    "{\n"
    "    using Base =\n"
    "        typename #^#FRAME_DEF#$#;\n"
    "public:\n"
    "    /// @brief Allow access to frame definition layers.\n"
    "    /// @details See definition of @b COMMS_PROTOCOL_LAYERS_ACCESS macro\n"
    "    ///     from COMMS library for details.\n"
    "    ///\n"
    "    ///     The generated functions are:\n"
    "    #^#ACCESS_FUNCS_DOC#$#\n"
    "    COMMS_PROTOCOL_LAYERS_ACCESS(\n"
    "        #^#LAYERS_ACCESS_LIST#$#\n"
    "    );\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
    "#^#APPEND#$#\n"
);

} // namespace

bool Frame::prepare()
{
    if (!m_dslObj.valid()) {
        assert(!"NYI");
        return true;
    }


    m_externalRef = m_dslObj.externalRef();
    if (m_externalRef.empty()) {
        m_generator.logger().log(commsdsl::ErrorLevel_Error, "Unknown external reference for frame: " + m_dslObj.name());
        return false;
    }

    auto dslLayers = m_dslObj.layers();
    m_layers.reserve(dslLayers.size());
    for (auto& l : dslLayers) {
        auto ptr = Layer::create(m_generator, l);
        assert(ptr);
        if (!ptr->prepare()) {
            return false;
        }
        m_layers.push_back(std::move(ptr));
    }

    while (true) {
        bool rearanged = false;
        for (auto& l : m_layers) {
            bool success = false;
            rearanged = l->rearange(m_layers, success);

            if (!success) {
                return false;
            }

            if (rearanged) {
                break;
            }
        }

        if (!rearanged) {
            break;
        }
    }

    return true;
}

bool Frame::write()
{
    return
        writeProtocolDefinitionCommonFile() &&
        writeProtocol() &&
        writePluginTransportMessageHeader() &&
        writePluginTransportMessageSrc() &&
        writePluginHeader();
}

std::string Frame::getDefaultOptions() const
{
    return getOptions(&Layer::getDefaultOptions);
}

std::string Frame::getBareMetalDefaultOptions() const
{
    return getOptions(&Layer::getBareMetalDefaultOptions);
}

std::vector<std::string> Frame::getPseudoVersionLayers(
    const std::vector<std::string>& interfaceVersionFields) const
{
    std::vector<std::string> result;
    for (auto& l : m_layers) {
        if (l->isPseudoVersionLayer(interfaceVersionFields)) {
            result.push_back(l->name());
        }
    }
    return result;
}

bool Frame::writeProtocolDefinitionCommonFile()
{
    common::StringsList commonElems;
    common::StringsList includes;
    auto frameScope =
        m_generator.scopeForFrame(m_externalRef, true, true);

    auto layerScope = frameScope + common::layersSuffixStr() + "::";
    for (auto& l : m_layers) {
        auto commonDef = l->getCommonDefinition(layerScope);
        if (!commonDef.empty()) {
            commonElems.push_back(commonDef);
            l->updateIncludesCommon(includes);
        }
    }

    if (commonElems.empty()) {
        return true;
    }


    auto adjName = m_externalRef + common::commonSuffixStr();
    auto names = m_generator.startFrameProtocolWrite(adjName);
    auto& filePath = names.first;
    //auto& className = names.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "/// @file\n"
        "/// @brief Contains common template parameters independent functionality of\n"
        "///    fields used in definition of @ref #^#SCOPE#$# frame.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "/// @brief Common types and functions of fields using in definition of\n"
        "///     @ref #^#SCOPE#$# frame.\n"
        "/// @see #^#SCOPE#$#Layers\n"
        "struct #^#NAME#$#LayersCommon\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n"
        "#^#END_NAMESPACE#$#\n";

    auto namespaces = m_generator.namespacesForFrame(m_externalRef);
    common::ReplacementMap repl;
    repl.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    repl.insert(std::make_pair("SCOPE", frameScope));
    repl.insert(std::make_pair("NAME", common::nameToClassCopy(name())));
    repl.insert(std::make_pair("BODY", common::listToString(commonElems, "\n", common::emptyString())));
    repl.insert(std::make_pair("INCLUDES", common::includesToStatements(includes)));
    repl.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    repl.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto str = common::processTemplate(Templ, repl);

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

bool Frame::writeProtocol()
{
    auto names =
        m_generator.startFrameProtocolWrite(m_externalRef);
    auto& filePath = names.first;
    auto& className = names.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("ORIG_CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("DOC_DETAILS", getDescription()));
    replacements.insert(std::make_pair("INCLUDES", getIncludes()));
    replacements.insert(std::make_pair("HEADERFILE", m_generator.headerfileForFrame(m_externalRef)));
    replacements.insert(std::make_pair("LAYERS_DEF", getLayersDef()));
    replacements.insert(std::make_pair("FRAME_DEF", getFrameDef()));
    replacements.insert(std::make_pair("LAYERS_ACCESS_LIST", getLayersAccess()));
    replacements.insert(std::make_pair("ACCESS_FUNCS_DOC", getLayersAccessDoc()));
    replacements.insert(std::make_pair("INPUT_MESSAGES", getInputMessages()));
    replacements.insert(std::make_pair("INPUT_MESSAGES_DOC", getInputMessagesDoc()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFrame(m_externalRef)));
    replacements.insert(std::make_pair("OPTIONS", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));

    auto namespaces = m_generator.namespacesForFrame(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

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

bool Frame::writePluginTransportMessageHeader()
{
    auto startInfo = m_generator.startFrameTransportMessageProtocolHeaderWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "#pragma once\n\n"
        "#include <tuple>\n"
        "#include <QtCore/QVariantList>\n"
        "#include \"comms_champion/TransportMessageBase.h\"\n"
        "#include #^#FRAME_INCLUDE#$#\n"
        "#^#INTERFACE_INCLUDE#$#\n"
        "\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "struct #^#ORIG_CLASS_NAME#$#Fields\n"
        "{\n"
        "    using All =\n"
        "        std::tuple<\n"
        "            #^#FIELDS#$#\n"
        "        >;\n"
        "    #^#PROPS_FUNC#$#\n"
        "    #^#READ_FUNC#$#\n"
        "};\n\n"
        "#^#INTERFACE_TEMPL_PARAM#$#\n"
        "class #^#CLASS_NAME#$# : public\n"
        "    comms_champion::TransportMessageBase<\n"
        "        #^#INTERFACE#$#,\n"
        "        #^#ORIG_CLASS_NAME#$#Fields::All\n"
        "    >\n"
        "{\n"
        "    #^#BASE_DEF#$#\n"
        "protected:\n"
        "    virtual const QVariantList& fieldsPropertiesImpl() const override#^#SEMICOLON#$#\n"
        "    #^#PROPS_BODY#$#\n"
        "    #^#READ_FUNC_DECL#$#\n"
        "};\n\n"
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n";

    common::StringsList fields;
    fields.reserve(m_layers.size());
    auto scope = m_generator.scopeForFrame(m_externalRef, true, false);
    scope += common::nameToClassCopy(name()) + common::layersSuffixStr() + "<>::";
    auto layers = m_dslObj.layers();
    for (auto& l : layers) {
        auto iter =
            std::find_if(
                m_layers.begin(), m_layers.end(),
                [&l](auto& ptr)
                {
                    return ptr->name() == l.name();
                });
        assert(iter != m_layers.end());
        fields.push_back((*iter)->getFieldScopeForPlugin(scope));
    }

    auto offset = calcBackPayloadOffset();

    auto namespaces = m_generator.namespacesForFrameInPlugin(m_externalRef);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("ORIG_CLASS_NAME", common::nameToClassCopy(name()) + common::transportMessageSuffixStr()));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("FRAME_INCLUDE", m_generator.headerfileForFrame(m_externalRef, true)));
    replacements.insert(std::make_pair("FIELDS", common::listToString(fields, ",\n", common::emptyString())));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFrameTransportMessageHeaderInPlugin(m_externalRef)));

    std::string interfaceStr = "TInterface";
    auto* interface = m_generator.getDefaultInterface();
    if (interface != nullptr) {
        replacements.insert(std::make_pair("INTERFACE_INCLUDE", "#include " + m_generator.headerfileForInterfaceInPlugin(interface->externalRef(), true)));
        replacements.insert(std::make_pair("SEMICOLON", ";"));
        interfaceStr = m_generator.scopeForInterfaceInPlugin(interface->externalRef());
        if (offset != 0U) {
            replacements.insert(std::make_pair("READ_FUNC_DECL", "virtual comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t len) override;"));
        }

    }
    else {
        std::string propsBody =
            "{\n"
            "    return " + common::nameToClassCopy(name()) + common::transportMessageSuffixStr() + common::fieldsSuffixStr() + "::props();\n"
            "}\n";

        std::string baseDef = 
            "using Base =\n"
            "    comms_champion::TransportMessageBase<\n"
            "        " + interfaceStr + ",\n"
            "        " + common::nameToClassCopy(name()) + common::transportMessageSuffixStr() + common::fieldsSuffixStr() + "::All\n"
            "    >;";
            
        replacements.insert(std::make_pair("INTERFACE_TEMPL_PARAM", "template <typename TInterface>"));
        replacements.insert(std::make_pair("BASE_DEF", std::move(baseDef)));
        replacements.insert(std::make_pair("PROPS_FUNC", "static const QVariantList& props();"));
        replacements.insert(std::make_pair("PROPS_BODY", std::move(propsBody)));

        if (offset != 0U) {
            std::string readMemFunc =
                "using typename Base::ReadIterator;\n"
                "virtual comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t len) override\n"
                "{\n"
                "    return " + common::nameToClassCopy(name()) + common::transportMessageSuffixStr() + common::fieldsSuffixStr() + "::read(Base::fields(), iter, len);\n"
                "}\n";
            replacements.insert(std::make_pair("READ_FUNC", "static comms::ErrorStatus read(All& fields, const std::uint8_t*& iter, std::size_t len);"));
            replacements.insert(std::make_pair("READ_FUNC_DECL", std::move(readMemFunc)));
        }
    }

    replacements.insert(std::make_pair("INTERFACE", std::move(interfaceStr)));

    std::string str = common::processTemplate(Templ, replacements);

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

bool Frame::writePluginTransportMessageSrc()
{
    auto startInfo = m_generator.startFrameTransportMessageProtocolSrcWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ = 
        "#^#GEN_COMMENT#$#\n"
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
        "     QVariantList props;\n"
        "     #^#APPENDS#$#\n"
        "     return props;\n"
        "}\n\n"
        "} // namespace\n\n"
        "const QVariantList& #^#CLASS_NAME#$##^#PROPS_FUNC_DECL#$#\n"
        "{\n"
        "    static const QVariantList Props = createProps();\n"
        "    return Props;\n"
        "}\n\n"
        "#^#READ_FUNC#$#\n"
        "#^#END_NAMESPACE#$#\n"
        "#^#FILE_APPEND#$#\n"
    ;

    common::StringsList includes;
    common::StringsList fieldsProps;
    common::StringsList appends;
    includes.reserve(m_layers.size());
    fieldsProps.reserve(m_layers.size());
    appends.reserve(m_layers.size());

    auto scope = 
        m_generator.scopeForFrame(m_externalRef, true, false) + 
            common::nameToClassCopy(name()) + common::layersSuffixStr() + "<>::";
    auto layers = m_dslObj.layers();
    for (auto& l : layers) {
        auto iter =
            std::find_if(
                m_layers.begin(), m_layers.end(),
                [&l](auto& ptr)
                {
                    return ptr->name() == l.name();
                });
        assert(iter != m_layers.end());
        (*iter)->updatePluginIncludes(includes);
        fieldsProps.push_back((*iter)->getPluginCreatePropsFunc(scope));

        auto propsStr =
            common::nameToClassCopy(l.name()) + "Layer::" +
            "createProps_" + (*iter)->getFieldAccNameForPlugin() + "()";
        appends.push_back("props.append(" + propsStr + ");");
    }

    auto namespaces = m_generator.namespacesForFrameInPlugin(m_externalRef);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("FIELDS_PROPS", common::listToString(fieldsProps, "\n", "\n")));
    replacements.insert(std::make_pair("APPENDS", common::listToString(appends, "\n", common::emptyString())));
    replacements.insert(std::make_pair("FILE_APPEND", m_generator.getExtraAppendForFrameTransportMessageSrcInPlugin(m_externalRef)));

    if (!includes.empty()) {
        replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(includes)));
    }

    auto offset = calcBackPayloadOffset();
    auto idxCalcFunc = 
        [&layers]()
        {
            auto payloadIter =
                std::find_if(
                    layers.begin(), layers.end(),
                    [](auto& l)
                    {
                        return l.kind() == commsdsl::Layer::Kind::Payload;
                    });
            assert(payloadIter != layers.end());
            return static_cast<unsigned>(std::distance(layers.begin(), payloadIter)) + 1U;
        };        

    auto* interface = m_generator.getDefaultInterface();
    if (interface != nullptr) {
        static const std::string PropsDecl = "::fieldsPropertiesImpl() const";
        replacements.insert(std::make_pair("PROPS_FUNC_DECL", PropsDecl));

        if (offset != 0U) {
            auto readUntilIdx = idxCalcFunc();

            std::string readFunc = 
                "comms::ErrorStatus " + common::nameToClassCopy(name()) + common::transportMessageSuffixStr() + "::readImpl(ReadIterator& iter, std::size_t len)\n"
                "{\n"
                "    len -= " + common::numToString(offset) + ";\n"
                "    auto es = doReadUntilAndUpdateLen<" + common::numToString(readUntilIdx) + ">(iter, len);\n"
                "    if (es == comms::ErrorStatus::Success) {\n"
                "        len += " + common::numToString(offset) + ";\n"
                "        es = doReadFrom<" + common::numToString(readUntilIdx) + ">(iter, len);\n"
                "    }\n\n"
                "    return es;\n"
                "}\n";
            replacements.insert(std::make_pair("READ_FUNC", std::move(readFunc)));                
        }
    }
    else {
        static const std::string PropsDecl = "Fields::props()";
        replacements.insert(std::make_pair("PROPS_FUNC_DECL", PropsDecl));
        replacements["CLASS_NAME"] = common::nameToClassCopy(name()) + common::transportMessageSuffixStr();

        if (offset != 0U) {
            auto readUntilIdx = idxCalcFunc();

            std::string readFunc = 
                "comms::ErrorStatus " + common::nameToClassCopy(name()) + common::transportMessageSuffixStr() + "Fields::read(All& fields, const std::uint8_t*& iter, std::size_t len)\n"
                "{\n"
                "    len -= " + common::numToString(offset) + ";\n"
                "    auto es = comms::ErrorStatus::NumOfErrorStatuses;\n";
            auto addToReadFunc = 
                [&readFunc](unsigned idx)
                {
                    auto idxStr = std::to_string(idx);
                    readFunc += 
                        "    auto& field" + idxStr + " = std::get<" + idxStr + ">(fields);\n"
                        "    es = field" + idxStr + ".read(iter, len);\n"
                        "    if (es != comms::ErrorStatus::Success) {\n"
                        "        return es;\n"
                        "    }\n"
                        "    len -= field" + idxStr + ".length();\n\n";

                };
            for (auto idx = 0U; idx < readUntilIdx; ++idx) {
                addToReadFunc(idx);
            }
            readFunc += "    len += " + common::numToString(offset) + ";\n";
            for (auto idx = readUntilIdx; idx < layers.size(); ++idx) {
                addToReadFunc(idx);
            }

            readFunc += "    return comms::ErrorStatus::Success;\n}\n";
            replacements.insert(std::make_pair("READ_FUNC", std::move(readFunc)));                
        }
    }


    std::string str = common::processTemplate(Templ, replacements);

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

bool Frame::writePluginHeader()
{
    auto startInfo = m_generator.startFrameProtocolHeaderWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    static const std::string Templ =
        "#^#GEN_COMMENT#$#\n"
        "#pragma once\n\n"
        "#include #^#FRAME_INCLUDE#$#\n"
        "#^#INTERFACE_INCLUDE#$#\n"
        "#^#ALL_MESSAGES_INCLUDE#$#\n"
        "\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "#^#INTERFACE_TEMPL_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    #^#FRAME_SCOPE#$#<\n"
        "        #^#INTERFACE#$#,\n"
        "        #^#ALL_MESSAGES#$#\n"
        "    >;\n\n"
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n";

    common::StringsList fields;
    fields.reserve(m_layers.size());
    auto scope = m_generator.scopeForFrame(m_externalRef, true, true);

    auto namespaces = m_generator.namespacesForFrameInPlugin(m_externalRef);

    auto allMessagesInclude = "#include " + m_generator.headerfileForInputInPlugin(common::allMessagesStr());
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("FRAME_SCOPE", std::move(scope)));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("FRAME_INCLUDE", m_generator.headerfileForFrame(m_externalRef, true)));
    replacements.insert(std::make_pair("ALL_MESSAGES_INCLUDE", std::move(allMessagesInclude)));
    replacements.insert(std::make_pair("ALL_MESSAGES", m_generator.scopeForInputInPlugin(common::allMessagesStr())));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFrameHeaderInPlugin(m_externalRef)));

    std::string interfaceStr = "TInterface";
    auto* interface = m_generator.getDefaultInterface();
    if (interface != nullptr) {
        replacements.insert(std::make_pair("INTERFACE_INCLUDE", "#include " + m_generator.headerfileForInterfaceInPlugin(interface->externalRef(), true)));
        interfaceStr = m_generator.scopeForInterfaceInPlugin(interface->externalRef());
    }
    else {
        replacements.insert(std::make_pair("INTERFACE_TEMPL_PARAM", "template <typename TInterface>"));
        replacements["ALL_MESSAGES"] += "<" + interfaceStr + ">";
    }

    replacements.insert(std::make_pair("INTERFACE", std::move(interfaceStr)));

    std::string str = common::processTemplate(Templ, replacements);

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

std::string Frame::getDescription() const
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

std::string Frame::getIncludes() const
{
    common::StringsList includes;
    for (auto& l : m_layers) {
        l->updateIncludes(includes);
    }

    if (hasCommonDefinition()) {
        auto refStr = m_externalRef + common::commonSuffixStr();
        common::mergeInclude(m_generator.headerfileForFrame(refStr, false), includes);
    }

//    if (!m_layers.empty()) {
//        common::mergeInclude("<tuple>", includes);
//    }

    common::mergeInclude(m_generator.headerfileForOptions(common::defaultOptionsStr(), false), includes);
    common::mergeInclude(m_generator.headerfileForInput(common::allMessagesStr(), false), includes);
    return common::includesToStatements(includes);
}

std::string Frame::getLayersDef() const
{
    common::StringsList defs;
    defs.reserve(m_layers.size() + 1);

    auto scope =
        "TOpt::" +
        m_generator.scopeForFrame(externalRef(), false, true) +
        common::layersSuffixStr() +
        "::";

    std::string prevLayer;
    bool hasInputMessages = false;
    for (auto iter = m_layers.rbegin(); iter != m_layers.rend(); ++iter) {
        auto& f = *iter;
        defs.push_back(f->getClassDefinition(scope, prevLayer, hasInputMessages));
    }

    static const std::string StackDefTempl =
        "/// @brief Final protocol stack definition.\n"
        "#^#STACK_PARAMS#$#\n"
        "using Stack = #^#LAST_LAYER#$##^#LAST_LAYER_PARAMS#$#;\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("LAST_LAYER", prevLayer));
    if (hasInputMessages) {
        std::string stackParams = 
            "template<typename TMessage, typename TAllMessages>";

        std::string lastLayerParams = "<TMessage, TAllMessages>";
        replacements.insert(std::make_pair("STACK_PARAMS", stackParams));
        replacements.insert(std::make_pair("LAST_LAYER_PARAMS", lastLayerParams));
    }
    defs.push_back(common::processTemplate(StackDefTempl, replacements));

    return common::listToString(defs, "\n", common::emptyString());

}

std::string Frame::getFrameDef() const
{
    auto className = common::nameToClassCopy(name());
    auto str = className + common::layersSuffixStr() + "<TOpt>::";
    if (hasIdLayer()) {
        str += "template Stack<TMessage, TAllMessages>";
    }
    else {
        str += "Stack";
    }
    return str;
}

std::string Frame::getLayersAccess() const
{
    common::StringsList names;
    names.reserve(m_layers.size());
    std::transform(
        m_layers.rbegin(), m_layers.rend(), std::back_inserter(names),
        [](auto& l)
        {
            return common::nameToAccessCopy(l->name());
        });
    return common::listToString(names, ",\n", common::emptyString());
}

std::string Frame::getLayersAccessDoc() const
{
    common::StringsList lines;
    auto className = common::nameToClassCopy(name());
    lines.reserve(m_layers.size());
    std::transform(
        m_layers.rbegin(), m_layers.rend(), std::back_inserter(lines),
        [&className](auto& l)
        {
            return
                "///     @li layer_" + common::nameToAccessCopy(l->name()) +
                "() for @ref " + className +
                common::layersSuffixStr() + "::" + common::nameToClassCopy(l->name()) + " layer.";
        });
    return common::listToString(lines, "\n", common::emptyString());
}

std::string Frame::getInputMessages() const
{
    if (!hasIdLayer()) {
        return common::emptyString();
    }

    return
        "typename TAllMessages = " + m_generator.scopeForInput(common::allMessagesStr(), true, true) + "<TMessage>,";
}

std::string Frame::getInputMessagesDoc() const
{
    if (!hasIdLayer()) {
        return common::emptyString();
    }

    return "/// @tparam TAllMessages All supported input messages.";
}

bool Frame::hasIdLayer() const
{
    return
        std::any_of(
            m_layers.begin(), m_layers.end(),
            [](auto& l)
            {
                if (l->kind() == commsdsl::Layer::Kind::Id) {
                    return true;
                }

                if (l->kind() != commsdsl::Layer::Kind::Custom) {
                    return false;
                }

                return static_cast<const CustomLayer*>(l.get())->isIdReplacement();
    });
}

unsigned Frame::calcBackPayloadOffset() const
{
    auto layers = m_dslObj.layers();
    auto payloadIter =
        std::find_if(
            layers.rbegin(), layers.rend(),
            [](auto& l)
            {
                return l.kind() == commsdsl::Layer::Kind::Payload;
            });
    assert(payloadIter != layers.rend());

    return
        std::accumulate(
            layers.rbegin(), payloadIter, std::size_t(0),
            [](std::size_t soFar, auto& l)
            {
                assert(l.field().valid());
                return soFar + l.field().minLength();
            });
}

std::string Frame::getOptions(GetLayerOptionsFunc func) const
{
    common::StringsList layersOpts;
    layersOpts.reserve(m_layers.size());
    auto scope = m_generator.scopeForFrame(m_externalRef, true, true) + common::layersSuffixStr() + "::";
    for (auto iter = m_layers.rbegin(); iter != m_layers.rend(); ++iter) {
        auto opt = ((*iter).get()->*func)(scope);
        if (!opt.empty()) {
            layersOpts.push_back(std::move(opt));
        }
    }

    if (layersOpts.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Extra options for Layers of\n"
        "///     @ref #^#FRAME_SCOPE#$# frame.\n"
        "struct #^#CLASS_NAME#$#Layers\n"
        "{\n"
        "    #^#LAYERS_OPTS#$#\n"
        "}; // struct #^#CLASS_NAME#$#Layers\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(m_dslObj.name())));
    replacements.insert(std::make_pair("FRAME_SCOPE", m_generator.scopeForFrame(externalRef(), true, true)));
    replacements.insert(std::make_pair("LAYERS_OPTS", common::listToString(layersOpts, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

bool Frame::hasCommonDefinition() const
{
    return
        std::any_of(
            m_layers.begin(), m_layers.end(),
            [](auto& l)
            {
                return l->hasCommonDefinition();
            });
}

} // namespace commsdsl2comms
