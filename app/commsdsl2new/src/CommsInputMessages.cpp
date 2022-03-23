//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2new
{

namespace 
{

const std::string ClientInputSuffixStr = "ClientInputMessages";    
const std::string ServerInputSuffixStr = "ServerInputMessages";

using CheckFunction = std::function<bool (const commsdsl::gen::Message&)>;
bool writeFileInternal(
    const std::string& name,
    CommsGenerator& generator,
    CheckFunction&& func)
{
    auto filePath = comms::headerPathForInput(name, generator);
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

    auto allMessages = generator.getAllMessages();
    util::StringsList includes = {
        "<tuple>",
        comms::relHeaderForOptions(strings::defaultOptionsClassStr(), generator)
    };

    util::StringsList scopes;

    for (auto* m : allMessages) {
        assert(m != nullptr);
        if (!func(*m)) {
            continue;
        }

        includes.push_back(comms::relHeaderPathFor(*m, generator));
        scopes.push_back(comms::scopeFor(*m, generator) + "<TBase, TOpt>");
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of the #^#NAME#$# messages bundle.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace input\n"
        "{\n\n"
        "/// @brief Messages of the protocol in ascending order.\n"
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
        "} // namespace #^#PROT_NAMESPACE#$#\n";

    comms::prepareIncludeStatement(includes);
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"PROT_NAMESPACE", generator.mainNamespace()},
        {"NAME", name},
        {"OPTIONS", comms::scopeForOptions(strings::defaultOptionsClassStr(), generator)},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"MESSAGES", util::strListToString(scopes, ",\n", "")},
        {"EXTEND", util::readFileContents(comms::inputCodePathForInput(name, generator) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForInput(name, generator) + strings::appendFileSuffixStr())},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

} // namespace 
    

bool CommsInputMessages::write(CommsGenerator& generator)
{
    CommsInputMessages obj(generator);
    return obj.writeInternal();
}

bool CommsInputMessages::writeInternal() const
{
    return
        writeAllMessagesInternal() &&
        writeClientInputMessagesInternal() &&
        writeServerInputMessagesInternal() &&
        writePlatformInputMessagesInternal() ;
}

bool CommsInputMessages::writeAllMessagesInternal() const
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
            m_generator,
            checkFunc);
}


bool CommsInputMessages::writeClientInputMessagesInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg)
        {
            return msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server;
        };

    return 
        writeFileInternal(
            ClientInputSuffixStr,
            m_generator,
            checkFunc);
}

bool CommsInputMessages::writeServerInputMessagesInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg)
        {
            return msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client;
        };

    return 
        writeFileInternal(
            ServerInputSuffixStr,
            m_generator,
            checkFunc);
}

bool CommsInputMessages::writePlatformInputMessagesInternal() const
{
    auto& platforms = m_generator.platformNames();
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

        if (!writeFileInternal(comms::className(p) + "Messages", m_generator, allCheckFunc)) {
            return false;
        }

        auto clientCheckFunc = 
            [&platformCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    platformCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server);
            };

        if (!writeFileInternal(comms::className(p) + ClientInputSuffixStr, m_generator, clientCheckFunc)) {
            return false;
        }  

        auto serverCheckFunc = 
            [&platformCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    platformCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client);
            };

        if (!writeFileInternal(comms::className(p) + ServerInputSuffixStr, m_generator, serverCheckFunc)) {
            return false;
        }                     
    };        

    return true;
}

} // namespace commsdsl2new