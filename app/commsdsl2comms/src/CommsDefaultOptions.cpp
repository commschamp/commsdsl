//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsDefaultOptions.h"

#include "CommsGenerator.h"
#include "CommsNamespace.h"
#include "CommsSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

const std::string MsgFactoryOptionsSuffix("MsgFactoryDefaultOptions");

using NamespaceOptionsFunc = std::string (CommsNamespace::*)() const;
std::string optionsBodyInternal(
    CommsGenerator& generator,
    NamespaceOptionsFunc nsFunc,
    bool hasBase)
{
    auto& allNs = generator.currentSchema().namespaces();
    util::StringsList opts;
    for (auto& nsPtr : allNs) {
        auto elem = (static_cast<const CommsNamespace*>(nsPtr.get())->*nsFunc)();
        if (!elem.empty()) {
            opts.push_back(std::move(elem));
        }
    }

    if (opts.empty()) {
        return strings::emptyString();
    }

    if (!generator.commsHasMainNamespaceInOptions()) {
        return util::strListToString(opts, "\n", "");
    }

    static const std::string Templ = 
        "struct #^#NS#$##^#EXT#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}; // struct #^#NS#$#\n";

    util::ReplacementMap repl = {
        {"NS", generator.currentSchema().mainNamespace()},
        {"BODY", util::strListToString(opts, "\n", "")}
    };

    if (hasBase) {
        repl["EXT"] = " : public TBase::" + repl["NS"];
    }

    return util::processTemplate(Templ, repl);
}

bool writeFileInternal(
    const std::string& name,
    CommsGenerator& generator,
    const std::string& data)
{
    auto filePath = comms::headerPathForOptions(name, generator);
    generator.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }      

    std::ofstream stream(filePath);
    if (!stream) {
        generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }    

    stream << data;
    stream.flush();
    return stream.good();
}

const std::string& extOptionsTempl()
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol #^#DESC#$# default options.\n\n"
        "#pragma once\n\n"
        "#include \"#^#PROT_NAMESPACE#$#/options/DefaultOptions.h\"\n\n"
        "#^#EXTRA#$#\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "/// @brief Default #^#DESC#$# options of the protocol.\n"
        "template <typename TBase = #^#DEFAULT_OPTS#$#>\n"
        "struct #^#NAME#$#DefaultOptionsT : public TBase\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "/// @brief Alias to @ref #^#NAME#$#DefaultOptionsT with default template parameter.\n"
        "using #^#NAME#$#DefaultOptions#^#ORIG#$# = #^#NAME#$#DefaultOptionsT<>;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n";

    return Templ;
}

util::ReplacementMap extInitialRepl(CommsGenerator& generator)
{
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"PROT_NAMESPACE", generator.currentSchema().mainNamespace()},
        {"DEFAULT_OPTS", comms::scopeForOptions(strings::defaultOptionsClassStr(), generator)}
    };
    return repl;
}

const std::string& msgFactoryOptionsTempl()
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol #^#DESC#$# message factory options.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "/// @brief Provided #^#DESC#$# message factory options of the protocol.\n"
        "/// @details Must be used as the outermost wrapper of the protocol options.\n"
        "template <typename TBase = #^#DEFAULT_OPTS#$#>\n"
        "struct #^#NAME#$#MsgFactoryDefaultOptionsT : public TBase\n"
        "{\n"
        "    #^#MSG_FACTORIES#$#\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "/// @brief Alias to @ref #^#NAME#$#MsgFactoryDefaultOptionsT with default template parameter.\n"
        "using #^#NAME#$#MsgFactoryDefaultOptions#^#ORIG#$# = #^#NAME#$#MsgFactoryDefaultOptionsT<>;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n";

    return Templ;
}


} // namespace 
    

bool CommsDefaultOptions::write(CommsGenerator& generator)
{
    auto& thisSchema = static_cast<CommsSchema&>(generator.currentSchema());
    if ((!generator.isCurrentProtocolSchema()) && (!thisSchema.commsHasAnyGeneratedCode())) {
        return true;
    }

    CommsDefaultOptions obj(generator);
    return obj.commsWriteInternal();
}

bool CommsDefaultOptions::commsWriteInternal() const
{
    return
        commsWriteDefaultOptionsInternal() &&
        commsWriteClientDefaultOptionsInternal() &&
        commsWriteServerDefaultOptionsInternal() &&
        commsWriteDataViewDefaultOptionsInternal() &&
        commsWriteBareMetalDefaultOptionsInternal() &&
        commsWriteMsgFactoryDefaultOptionsInternal();
}

bool CommsDefaultOptions::commsWriteDefaultOptionsInternal() const
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of protocol default options.\n\n"
        "#pragma once\n\n"
        "#include \"comms/options.h\"\n\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace options\n"
        "{\n\n"
        "/// @brief Empty class to serve as the base for options.\n"
        "struct EmptyOptions {};\n\n"
        "/// @brief Default (empty) options of the protocol.\n"
        "template <typename TBase = EmptyOptions>\n"
        "struct #^#CLASS_NAME#$##^#ORIG#$#T : public TBase\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "/// @brief Default (empty) options of the protocol.\n"
        "using #^#CLASS_NAME#$##^#ORIG#$# = #^#CLASS_NAME#$##^#ORIG#$#T<>;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace options\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n";

    auto& name = strings::defaultOptionsClassStr();
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"PROT_NAMESPACE", m_generator.currentSchema().mainNamespace()},
        {"CLASS_NAME", name},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsDefaultOptions, false)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(strings::defaultOptionsClassStr(), m_generator, util::processTemplate(Templ, repl, true));
    return true;
}

bool CommsDefaultOptions::commsWriteClientDefaultOptionsInternal() const
{
    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = "Client" + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "client"},
        {"NAME", "Client"},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsClientDefaultOptions, true)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl, true));
    return true;
}

bool CommsDefaultOptions::commsWriteServerDefaultOptionsInternal() const
{
    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = "Server" + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "server"},
        {"NAME", "Server"},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsServerDefaultOptions, true)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl, true));
    return true;
}

bool CommsDefaultOptions::commsWriteDataViewDefaultOptionsInternal() const
{
    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = strings::dataViewStr() + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "data view"},
        {"NAME", strings::dataViewStr()},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsDataViewDefaultOptions, true)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl, true));
    return true;
}

bool CommsDefaultOptions::commsWriteBareMetalDefaultOptionsInternal() const
{
    std::string extra = 
        "#ifndef DEFAULT_SEQ_FIXED_STORAGE_SIZE\n"
        "/// @brief Define default fixed size for various sequence fields\n"
        "/// @details May be defined during compile time to change the default value.\n"
        "#define DEFAULT_SEQ_FIXED_STORAGE_SIZE 32\n"
        "#endif\n";

    util::ReplacementMap repl = extInitialRepl(m_generator);
    auto name = strings::bareMetalStr() + strings::defaultOptionsClassStr();
    repl.insert({
        {"DESC", "bare metal"},
        {"NAME", strings::bareMetalStr()},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsBareMetalDefaultOptions, true)},
        {"EXTRA", std::move(extra)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(extOptionsTempl(), repl, true));
    return true;
}

bool CommsDefaultOptions::commsWriteMsgFactoryDefaultOptionsInternal() const
{
    if (!m_generator.isCurrentProtocolSchema()) {
        return true;
    }

    return 
        commsWriteAllMessagesDynMemMsgFactoryOptionsInternal() &&
        commsWriteClientInputMessagesDynMemMsgFactoryOptionsInternal() &&
        commsWriteServerInputMessagesDynMemMsgFactoryOptionsInternal() &&
        commsWritePlatformSpecificDynMemMsgFactoryOptionsInternal() &&
        commsWriteExtraBundlesDynMemMsgFactoryOptionsInternal();
}

bool CommsDefaultOptions::commsWriteAllMessagesDynMemMsgFactoryOptionsInternal() const
{
    return 
        commsWriteSingleMsgFactoryDefaultOptionsInternal(
            "AllMessagesDynMem",
            "all",
            "dynamic memory"
        );
}

bool CommsDefaultOptions::commsWriteClientInputMessagesDynMemMsgFactoryOptionsInternal() const
{
    return 
        commsWriteSingleMsgFactoryDefaultOptionsInternal(
            "ClientInputMessagesDynMem",
            "client input",
            "dynamic memory"
        );
}

bool CommsDefaultOptions::commsWriteServerInputMessagesDynMemMsgFactoryOptionsInternal() const
{
    return 
        commsWriteSingleMsgFactoryDefaultOptionsInternal(
            "ServerInputMessagesDynMem",
            "server input",
            "dynamic memory"
        );
}

bool CommsDefaultOptions::commsWritePlatformSpecificDynMemMsgFactoryOptionsInternal() const
{
    auto& platforms = m_generator.currentSchema().platformNames();
    for (auto& p : platforms) {
        bool result = 
            commsWriteSingleMsgFactoryDefaultOptionsInternal(
                comms::className(p) + "MessagesDynMem",
                "all \"" + p + "\" platform scpecific",
                "dynamic memory"
            );

        if (!result) {
            return false;
        }

        result = 
            commsWriteSingleMsgFactoryDefaultOptionsInternal(
                comms::className(p) + "ClientInputMessagesDynMem",
                "client input \"" + p + "\" platform scpecific",
                "dynamic memory"
            );       

        if (!result) {
            return false;
        }             
       
        result = 
            commsWriteSingleMsgFactoryDefaultOptionsInternal(
                comms::className(p) + "ServerInputMessagesDynMem",
                "server input \"" + p + "\" platform scpecific",
                "dynamic memory"
            );       

        if (!result) {
            return false;
        }        
    };        

    return true;
}

bool CommsDefaultOptions::commsWriteExtraBundlesDynMemMsgFactoryOptionsInternal() const
{
    auto& extraBundles = m_generator.commsExtraMessageBundles();
    for (auto& b : extraBundles) {        
        bool result = 
            commsWriteSingleMsgFactoryDefaultOptionsInternal(
                comms::className(b.first) + "MessagesDynMem",
                "all \"" + b.first + "\" bundle scpecific",
                "dynamic memory"
            );

        if (!result) {
            return false;
        }

        result = 
            commsWriteSingleMsgFactoryDefaultOptionsInternal(
                comms::className(b.first) + "ClientInputMessagesDynMem",
                "client input \"" + b.first + "\" bundle scpecific",
                "dynamic memory"
            );       

        if (!result) {
            return false;
        }             
       
        result = 
            commsWriteSingleMsgFactoryDefaultOptionsInternal(
                comms::className(b.first) + "ServerInputMessagesDynMem",
                "server input \"" + b.first + "\" bundle scpecific",
                "dynamic memory"
            );       

        if (!result) {
            return false;
        }        
    };        

    return true;
}

bool CommsDefaultOptions::commsWriteSingleMsgFactoryDefaultOptionsInternal(
    const std::string& prefix, 
    const std::string& messagesDesc,
    const std::string& allocDesc) const
{
    auto name = prefix + MsgFactoryOptionsSuffix;

    util::StringsList includes = {
        comms::relHeaderForOptions(strings::defaultOptionsClassStr(), m_generator, true),
    };

    util::StringsList allFactories;
    auto allNamespaces = m_generator.getAllNamespaces();
    for (auto* ns : allNamespaces) {
        auto suffix = "<TInterface, " + name + "T<TBase> >";
        auto factoryDef = CommsNamespace::cast(ns)->commsMsgFactoryAliasDef(prefix, suffix);
        if (factoryDef.empty()) {
            continue;
        }

        static const std::string AliasTempl = 
            "/// @brief Alias to actual message factory class.\n"
            "/// @details Exposes the same template parameters as @b comms::MsgFactory.\n"
            "template <typename TInterface, typename TAllMessages, typename... TOptions>\n"
            "#^#DEF#$#\n"
        ;

        util::ReplacementMap aliasRepl = {
            {"DEF", std::move(factoryDef)},
            {"REF", CommsNamespace::cast(ns)->commsMsgFactoryAliasType()},
        };

        allFactories.push_back(util::processTemplate(AliasTempl, aliasRepl));
        includes.push_back(CommsNamespace::cast(ns)->commsRelHeaderPath(prefix));
    }

    comms::prepareIncludeStatement(includes);

    util::ReplacementMap repl = extInitialRepl(m_generator);
    repl.insert({
        {"DESC", messagesDesc + " messages " + allocDesc + " allocation"},
        {"NAME", prefix},
        {"MSG_FACTORIES", util::strListToString(allFactories, "\n", "\n")},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"BODY", optionsBodyInternal(m_generator, &CommsNamespace::commsMsgFactoryDefaultOptions, true)},
        {"EXTEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForOptions(name, m_generator) + strings::appendFileSuffixStr())},
    });

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    writeFileInternal(name, m_generator, util::processTemplate(msgFactoryOptionsTempl(), repl, true));
    return true;
}

} // namespace commsdsl2comms