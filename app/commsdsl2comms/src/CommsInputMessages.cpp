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

#include "CommsInputMessages.h"

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

const std::string ClientInputSuffixStr = "ClientInputMessages";    
const std::string ServerInputSuffixStr = "ServerInputMessages";

using CheckFunction = std::function<bool (const commsdsl::gen::Message&)>;
bool writeFileInternal(
    const std::string& name,
    const std::string& desc,
    CommsGenerator& generator,
    const CommsNamespace& parent,
    CheckFunction&& func)
{
    auto filePath = comms::headerPathForInput(name, generator, parent);
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

    auto allMessages = parent.getAllMessagesIdSorted();
    util::StringsList includes = {
        "<tuple>",
        comms::relHeaderForOptions(strings::defaultOptionsClassStr(), generator)
    };

    util::StringsList scopes;
    util::StringsList aliases;

    for (auto* m : allMessages) {
        assert(m != nullptr);
        if (!func(*m)) {
            continue;
        }

        auto scopeStr = comms::scopeFor(*m, generator);
        auto aliasStr = 
            "using prefix_ ## " + comms::className(m->dslObj().name()) + " ## suffix_ = " + scopeStr + "<interface_, opts_>;";
        includes.push_back(comms::relHeaderPathFor(*m, generator));
        scopes.push_back(scopeStr + "<TBase, TOpt>");
        aliases.push_back(std::move(aliasStr));
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of the #^#NAME#$# messages bundle.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "namespace input\n"
        "{\n\n"
        "/// @brief #^#DESC#$# messages of the protocol in ascending order.\n"
        "/// @tparam TBase Base class of all the messages.\n"
        "/// @tparam TOpt Protocol definition options.\n"
        "template <typename TBase, typename TOpt = #^#OPTIONS#$#>\n"
        "using #^#NAME#$##^#ORIG#$# =\n"
        "    std::tuple<\n"
        "        #^#MESSAGES#$#\n"
        "    >;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace input\n\n"
        "#^#NS_END#$#\n"
        "/// @brief Create type aliases for the #^#LOW_DESC#$# messages of the protocol.\n"
        "/// @param prefix_ Prefix of the alias message type.\n"
        "/// @param suffix_ Suffix of the alias message type.\n"
        "/// @param interface_ Type of the common message interface.\n"
        "/// @param opts_ Type of the used protocol definition options.\n"
        "#define #^#PROT_PREFIX#$#_ALIASES_FOR_#^#MACRO_NAME#$#(prefix_, suffix_, interface_, opts_) \\\n"
        "    #^#ALIASES#$#\n\n"
        "/// @brief Create type aliases for the #^#LOW_DESC#$# messages of the protocol using default options.\n"
        "/// @param prefix_ Prefix of the alias message type.\n"
        "/// @param suffix_ Suffix of the alias message type.\n"
        "/// @param interface_ Type of the common message interface.\n"        
        "#define #^#PROT_PREFIX#$#_ALIASES_FOR_#^#MACRO_NAME#$#_DEFAULT_OPTIONS(prefix_, suffix_, interface_) \\\n"
        "    #^#PROT_PREFIX#$#_ALIASES_FOR_#^#MACRO_NAME#$#(prefix_, suffix_, interface_, #^#OPTIONS#$#)\n"
        ;

    comms::prepareIncludeStatement(includes);
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"NAME", name},
        {"NS_BEGIN", comms::namespaceBeginFor(parent, generator)},
        {"NS_END", comms::namespaceEndFor(parent, generator)},          
        {"OPTIONS", comms::scopeForOptions(strings::defaultOptionsClassStr(), generator)},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"MESSAGES", util::strListToString(scopes, ",\n", "")},
        {"EXTEND", util::readFileContents(comms::inputCodePathForInput(name, generator, parent) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForInput(name, generator, parent) + strings::appendFileSuffixStr())},
        {"PROT_PREFIX", util::strToUpper(util::strReplace(comms::scopeFor(parent, generator), "::", "_"))},
        {"MACRO_NAME", util::strToMacroName(name)},
        {"ALIASES", util::strListToString(aliases, " \\\n", "\n")},
        {"DESC", desc},
        {"LOW_DESC", util::strToLower(desc)},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

} // namespace 
    
CommsInputMessages::CommsInputMessages(CommsGenerator& generator, const CommsNamespace& parent) : 
    m_generator(generator),
    m_parent(parent) 
{
}

bool CommsInputMessages::commsWrite() const
{
    return
        commsWriteAllMessagesInternal() &&
        commsWriteClientInputMessagesInternal() &&
        commsWriteServerInputMessagesInternal() &&
        commsWritePlatformInputMessagesInternal() &&
        commsWriteExtraInputMessagesInternal();
}

bool CommsInputMessages::commsWriteAllMessagesInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg) noexcept
        {
            static_cast<void>(msg);
            return true;
        };

    return 
        writeFileInternal(
            strings::allMessagesStr(),
            "All",
            m_generator,
            m_parent,
            checkFunc);
}


bool CommsInputMessages::commsWriteClientInputMessagesInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg)
        {
            return msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client;
        };

    return 
        writeFileInternal(
            ClientInputSuffixStr,
            "Client input",
            m_generator,
            m_parent,
            checkFunc);
}

bool CommsInputMessages::commsWriteServerInputMessagesInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg)
        {
            return msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server;
        };

    return 
        writeFileInternal(
            ServerInputSuffixStr,
            "Server input",
            m_generator,
            m_parent,
            checkFunc);
}

bool CommsInputMessages::commsWritePlatformInputMessagesInternal() const
{
    auto& platforms = m_generator.currentSchema().platformNames();
    for (auto& p : platforms) {

        auto platformCheckFunc = 
            [&p](const commsdsl::gen::Message& msg)
            {
                auto& msgPlatforms = msg.dslObj().platforms();
                if (msgPlatforms.empty()) {
                    return true;
                }

                return std::find(msgPlatforms.begin(), msgPlatforms.end(), p) != msgPlatforms.end();
            };

        auto allCheckFunc = 
            [&platformCheckFunc](const commsdsl::gen::Message& msg)
            {
                return platformCheckFunc(msg);
            };

        if (!writeFileInternal(comms::className(p) + "Messages", "All " + p + " platform", m_generator, m_parent, allCheckFunc)) {
            return false;
        }

        auto clientCheckFunc = 
            [&platformCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    platformCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client);
            };

        if (!writeFileInternal(comms::className(p) + ClientInputSuffixStr, "Client input " + p + " platform", m_generator, m_parent, clientCheckFunc)) {
            return false;
        }  

        auto serverCheckFunc = 
            [&platformCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    platformCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server);
            };

        if (!writeFileInternal(comms::className(p) + ServerInputSuffixStr, "Server input " + p + " platform", m_generator, m_parent, serverCheckFunc)) {
            return false;
        }                     
    };        

    return true;
}

bool CommsInputMessages::commsWriteExtraInputMessagesInternal() const
{
    auto& extraBundles = m_generator.commsExtraMessageBundles();
    for (auto& b : extraBundles) {

        auto bundleCheckFunc = 
            [&b](const commsdsl::gen::Message& msg)
            {
                return std::find(b.second.begin(), b.second.end(), &msg) != b.second.end();
            };

        auto allCheckFunc = 
            [&bundleCheckFunc](const commsdsl::gen::Message& msg)
            {
                return bundleCheckFunc(msg);
            };

        if (!writeFileInternal(comms::className(b.first) + "Messages", "All " + b.first + " bundle", m_generator, m_parent, allCheckFunc)) {
            return false;
        }

        auto clientCheckFunc = 
            [&bundleCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    bundleCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client);
            };

        if (!writeFileInternal(comms::className(b.first) + ClientInputSuffixStr, "Client input " + b.first + " bundle", m_generator, m_parent, clientCheckFunc)) {
            return false;
        }  

        auto serverCheckFunc = 
            [&bundleCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    bundleCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server);
            };

        if (!writeFileInternal(comms::className(b.first) + ServerInputSuffixStr, "Server input " + b.first + " bundle", m_generator, m_parent, serverCheckFunc)) {
            return false;
        }                     
    };        

    return true;
}


} // namespace commsdsl2comms