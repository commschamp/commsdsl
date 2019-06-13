//
// Copyright 2019 (C). Alex Robenko. All rights reserved.
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

#include "Dispatch.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool Dispatch::write(Generator& generator)
{
    struct MessagesInfo
    {
        common::StringsList m_messages;
        common::StringsList m_includes;
    };

    struct PlatformInfo
    {
        MessagesInfo m_all;
        MessagesInfo m_serverInput;
        MessagesInfo m_clientInput;
    };

    using PlatformsMap = std::map<std::string, PlatformInfo>;
    PlatformsMap platformsMap;
    platformsMap.insert(std::make_pair(std::string(), PlatformInfo()));

    auto& platforms = generator.platforms();
    for (auto& p : platforms) {
        platformsMap.insert(std::make_pair(p, PlatformInfo()));
    };

    auto allMessages = generator.getAllDslMessages();

    for (auto& p : platformsMap) {
        auto updateFunc =
            [&generator, &allMessages](auto& msgInfo, const std::string& inputName)
            {
                msgInfo.m_messages.reserve(allMessages.size());
                auto inputHeader = generator.headerfileForInput(inputName, false);
                common::mergeInclude(inputHeader, msgInfo.m_includes);
                auto msgIdHeader = generator.headerfileForRoot(common::msgIdEnumNameStr(), false);
                common::mergeInclude(msgIdHeader, msgInfo.m_includes);

                auto interfaces = generator.getAllInterfaces();
                for (auto& i : interfaces) {
                    auto inc = generator.headerfileForInterface(i->externalRef(), false);
                    common::mergeInclude(inc, msgInfo.m_includes);
                }
            };

        auto& inputPrefix = p.first;
        if (inputPrefix.empty()) {
            updateFunc(p.second.m_all, common::allMessagesStr());
        }
        else {
            updateFunc(p.second.m_all, inputPrefix + "Messages");
        }

        updateFunc(p.second.m_serverInput, inputPrefix + common::clientInputMessagesStr());
        updateFunc(p.second.m_clientInput, inputPrefix + common::serverInputMessagesStr());
    }

    for (auto m : allMessages) {
        assert(m.valid());

        if (!generator.doesElementExist(m.sinceVersion(), m.deprecatedSince(), m.isDeprecatedRemoved())) {
            continue;
        }

        auto extRef = m.externalRef();
        assert(!extRef.empty());

        auto msgStr = generator.scopeForMessage(extRef, true, true) + "<TBase, TOpt>";
        auto addToMessageInfoFunc =
            [&msgStr](MessagesInfo& info)
            {
                info.m_messages.push_back(msgStr);
            };

        bool serverInput = m.sender() != commsdsl::Message::Sender::Server;
        bool clientInput = m.sender() != commsdsl::Message::Sender::Client;

        auto addToPlatformInfoFunc =
            [&addToMessageInfoFunc, serverInput, clientInput](PlatformInfo& info)
            {
                addToMessageInfoFunc(info.m_all);
                if (serverInput) {
                    addToMessageInfoFunc(info.m_serverInput);
                }

                if (clientInput) {
                    addToMessageInfoFunc(info.m_clientInput);
                }
            };

        addToPlatformInfoFunc(platformsMap[common::emptyString()]);

        if (platformsMap.size() == 1U) {
            continue;
        }

        auto& msgPlatforms = m.platforms();
        if (msgPlatforms.empty()) {
            for (auto& p : platformsMap) {
                if (p.first.empty()) {
                    continue;
                }
                addToPlatformInfoFunc(p.second);
            }
            continue;
        }

        for (auto& p : msgPlatforms) {
            auto iter = platformsMap.find(p);
            if (iter == platformsMap.end()) {
                assert(!"Should not happen");
                continue;
            }

            addToPlatformInfoFunc(iter->second);
        }
    }


    auto writeFileFunc =
        [&generator](const MessagesInfo& info,
               const std::string& fileName,
               const std::string& platName = common::emptyString(),
               const std::string& inputName = common::emptyString())
        {
            auto startInfo = generator.startDispatchProtocolWrite(fileName);
            auto& filePath = startInfo.first;
            auto& funcName = startInfo.second;
            static_cast<void>(funcName); // TODO: remove

            if (filePath.empty()) {
                return true;
            }

            std::ofstream stream(filePath);
            if (!stream) {
                generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
                return false;
            }

            common::ReplacementMap replacements;
            auto namespaces = generator.namespacesForDispatch();
            replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
            replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
            replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(info.m_includes)));

            if (!platName.empty()) {
                replacements.insert(std::make_pair("PLAT_NAME", '\"' + platName + "\" "));
            }

            if (!inputName.empty()) {
                replacements.insert(std::make_pair("INPUT", inputName + " input "));
            }

            static const std::string Templ =
                "/// @file\n"
                "/// @brief Contains dispatch to handling function(s) for #^#PLAT_NAME#$##^#INPUT#$#messages.\n\n"
                "#pragma once\n\n"
                "#^#INCLUDES#$#\n"
                "#^#BEG_NAMESPACE#$#\n"
                "#^#END_NAMESPACE#$#\n";

            auto str = common::processTemplate(Templ, replacements);
            stream << str;

            stream.flush();
            if (!stream.good()) {
                generator.logger().error("Failed to write \"" + filePath + "\".");
                return false;
            }
            return true;
        };

    for (auto& p : platformsMap) {
        static const std::string DispatchPrefix = "Dispatch";
        static const std::string AllPrefix = "All";
        static const std::string MessagesSuffix = "Messages";
        static const std::string ServerInputStr = "ServerInput";
        static const std::string ClientInputStr = "ClientInput";
        auto allName = common::nameToClassCopy(p.first);
        std::string serverName;
        std::string clientName;
        do {
            if (allName.empty()) {
                allName = DispatchPrefix + AllPrefix + MessagesSuffix;
                serverName = DispatchPrefix + ServerInputStr + MessagesSuffix;
                clientName = DispatchPrefix + ClientInputStr + MessagesSuffix;
                break;
            }

            if (allName == AllPrefix) {
                generator.logger().error("Invalid platform name: \"" + p.first + "\".");
                return false;
            }

            serverName = (DispatchPrefix + allName + ServerInputStr + MessagesSuffix);
            clientName = (DispatchPrefix + allName + ClientInputStr + MessagesSuffix);
            allName = DispatchPrefix + allName + MessagesSuffix;
        } while (false);

        if (!writeFileFunc(p.second.m_all, allName, p.first)) {
            return false;
        }

        if (!writeFileFunc(p.second.m_serverInput, serverName, p.first, "server")) {
            return false;
        }

        if (!writeFileFunc(p.second.m_clientInput, clientName, p.first, "client")) {
            return false;
        }
    }

    return true;
}

} // namespace commsdsl2comms
