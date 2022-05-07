//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "AllMessages.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2old
{

bool AllMessages::write(Generator& generator)
{
    AllMessages obj(generator);
    return obj.writeProtocolDefinition() && obj.writePluginDefinition();
}

bool AllMessages::writeProtocolDefinition() const
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
        bool m_realPlatform = true;
    };

    using PlatformsMap = std::map<std::string, PlatformInfo>;
    PlatformsMap platformsMap;
    platformsMap.insert(std::make_pair(std::string(), PlatformInfo()));

    auto& platforms = m_generator.platforms();
    for (auto& p : platforms) {
        platformsMap.insert(std::make_pair(p, PlatformInfo()));
    };

    auto bundles = m_generator.extraMessagesBundles();
    for (auto& b : bundles) {
        auto& elem = platformsMap[b];
        elem.m_realPlatform = false;
    }

    auto allMessages = m_generator.getAllDslMessages();

    for (auto& p : platformsMap) {
        auto updateFunc = 
            [this, &allMessages](auto& msgInfo)
            {
                msgInfo.m_messages.reserve(allMessages.size());
                msgInfo.m_includes.reserve(allMessages.size() + 2);
                common::mergeInclude("<tuple>", msgInfo.m_includes);
                auto optionsHeader = m_generator.headerfileForOptions(common::defaultOptionsStr(), false);
                common::mergeInclude(optionsHeader, msgInfo.m_includes);
            };
        
        updateFunc(p.second.m_all);                
        updateFunc(p.second.m_serverInput);
        updateFunc(p.second.m_clientInput);
    }

    for (auto m : allMessages) {
        assert(m.valid());

        if (!m_generator.doesElementExist(m.sinceVersion(), m.deprecatedSince(), m.isDeprecatedRemoved())) {
            continue;
        }

        auto extRef = m.externalRef();
        assert(!extRef.empty());

        auto msgStr = m_generator.scopeForMessage(extRef, true, true) + "<TBase, TOpt>";
        auto incStr = m_generator.headerfileForMessage(extRef, false);

        auto addToMessageInfoFunc =
            [&msgStr, &incStr](MessagesInfo& info)
            {
                info.m_messages.push_back(msgStr);
                common::mergeInclude(incStr, info.m_includes);
            };

        bool serverInput = m.sender() != commsdsl::parse::Message::Sender::Server;            
        bool clientInput = m.sender() != commsdsl::parse::Message::Sender::Client;

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
                if ((p.first.empty()) || (!p.second.m_realPlatform)) {
                    continue;
                }
                addToPlatformInfoFunc(p.second);
            }
        }

        for (auto& p : msgPlatforms) {
            auto iter = platformsMap.find(p);
            if (iter == platformsMap.end()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
                assert(Should_not_happen);
                continue;
            }

            addToPlatformInfoFunc(iter->second);
        }

        auto inBundles = m_generator.bundlesForMessage(extRef);
        for (auto& b : inBundles) {
            auto iter = platformsMap.find(b);
            if (iter == platformsMap.end()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
                assert(Should_not_happen);
                continue;
            }
            addToPlatformInfoFunc(iter->second);
        }
    }

    auto writeFileFunc = 
        [this](const MessagesInfo& info, 
               const std::string& fileName,
               const std::string& platName = common::emptyString(),
               const std::string& inputName = common::emptyString())
        {
            auto startInfo = m_generator.startInputProtocolWrite(fileName);
            auto& filePath = startInfo.first;
            auto& className = startInfo.second;

            if (filePath.empty()) {
                return true;
            }

            std::ofstream stream(filePath);
            if (!stream) {
                m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
                return false;
            }

            common::ReplacementMap replacements;
            auto namespaces = m_generator.namespacesForInput();
            replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
            replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
            replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
            replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
            replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(info.m_includes)));
            replacements.insert(std::make_pair("MESSAGES", common::listToString(info.m_messages, ",\n", common::emptyString())));
            replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
            replacements.insert(std::make_pair("OPTIONS", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));

            if (!platName.empty()) {
                replacements.insert(std::make_pair("PLAT_NAME", '\"' + platName + "\" "));
            }

            if (!inputName.empty()) {
                replacements.insert(std::make_pair("INPUT", inputName + " input "));
            }

            const std::string Template(
                "#^#GEN_COMMENT#$#\n"
                "/// @file\n"
                "/// @brief Contains definition of all #^#PLAT_NAME#$##^#INPUT#$#messages bundle.\n\n"
                "#pragma once\n\n"
                "#^#INCLUDES#$#\n"
                "#^#BEG_NAMESPACE#$#\n"
                "/// @brief Messages of the protocol in ascending order.\n"
                "/// @tparam TBase Base class of all the messages.\n"
                "/// @tparam TOpt Protocol definition options.\n"
                "template <typename TBase, typename TOpt = #^#OPTIONS#$#>\n"
                "using #^#CLASS_NAME#$# =\n"
                "    std::tuple<\n"
                "        #^#MESSAGES#$#\n"
                "    >;\n\n"
                "#^#END_NAMESPACE#$#\n"
            );

            auto str = common::processTemplate(Template, replacements);
            stream << str;

            stream.flush();
            if (!stream.good()) {
                m_generator.logger().error("Failed to write \"" + filePath + "\".");
                return false;
            }
            return true;
        };

        for (auto& p : platformsMap) {
            static const std::string AllPrefix = "All";
            static const std::string MessagesSuffix = "Messages";
            static const std::string ServerInputStr = "ServerInput";
            static const std::string ClientInputStr = "ClientInput";
            auto allName = common::nameToClassCopy(p.first);
            std::string serverName;
            std::string clientName;
            do {
                if (allName.empty()) {
                    allName = AllPrefix + MessagesSuffix;
                    serverName = ServerInputStr + MessagesSuffix;
                    clientName = ClientInputStr + MessagesSuffix;
                    break;
                }

                if (allName == AllPrefix) {
                    m_generator.logger().error("Invalid platform name: \"" + p.first + "\".");
                    return false;
                }    

                serverName = (allName + ServerInputStr + MessagesSuffix);
                clientName = (allName + ClientInputStr + MessagesSuffix);
                allName += MessagesSuffix;
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

bool AllMessages::writePluginDefinition() const
{

    auto allMessages = m_generator.getAllDslMessages();
    common::StringsList messages;
    common::StringsList includes;
    messages.reserve(allMessages.size());
    includes.reserve(allMessages.size() + 1);
    common::mergeInclude("<tuple>", includes);

    std::string interfaceStr;
    auto* interface = m_generator.getDefaultInterface();
    if (interface == nullptr) {
        interfaceStr = "<TInterface>";
    }    

    struct PlatformInfo
    {
        common::StringsList m_messages;
        common::StringsList m_includes;        
    };

    using PlatformsMap = std::map<std::string, PlatformInfo>;
    PlatformsMap platformsMap;
    auto& platforms = m_generator.platforms();
    for (auto& p : platforms) {
        auto& info = platformsMap[p];
        info.m_messages.reserve(allMessages.size());
        info.m_includes.reserve(allMessages.size() + 1);
        common::mergeInclude("<tuple>", info.m_includes);
    }

    for (auto m : allMessages) {
        assert(m.valid());

        if (!m_generator.doesElementExist(m.sinceVersion(), m.deprecatedSince(), m.isDeprecatedRemoved())) {
            continue;
        }

        auto extRef = m.externalRef();
        assert(!extRef.empty());

        auto msgStr = m_generator.scopeForMessageInPlugin(extRef) + interfaceStr;
        auto incStr = m_generator.headerfileForMessageInPlugin(extRef, false);
        messages.push_back(msgStr);
        common::mergeInclude(incStr, includes);

        if (platformsMap.empty()) {
            continue;
        }

        auto addToPlatformFunc =
            [&msgStr, &incStr](PlatformInfo& info)
            {
                info.m_messages.push_back(msgStr);
                common::mergeInclude(incStr, info.m_includes);
            };

        auto& msgPlatforms = m.platforms();
        if (msgPlatforms.empty()) {
            for (auto& p : platformsMap) {
                addToPlatformFunc(p.second);
            }
            continue;
        }

        for (auto& p : msgPlatforms) {
            auto iter = platformsMap.find(p);
            if (iter == platformsMap.end()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
                assert(Should_not_happen);
                continue;
            }

            addToPlatformFunc(iter->second);
        }
    }

    auto writeFileFunc = 
        [this, interface](
                const common::StringsList& incs, 
                const common::StringsList& msgs, 
                const std::string& fileName)
        {
            if (msgs.empty()) {
                return true;
            }


            auto startInfo = m_generator.startInputPluginHeaderWrite(fileName);
            auto& filePath = startInfo.first;
            auto& className = startInfo.second;

            if (filePath.empty()) {
                return true;
            }

            std::ofstream stream(filePath);
            if (!stream) {
                m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
                return false;
            }

            common::ReplacementMap replacements;
            auto namespaces = m_generator.namespacesForInputInPlugin();
            replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
            replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
            replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
            replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
            replacements.insert(std::make_pair("CLASS_NAME", std::move(className)));
            replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(incs)));
            replacements.insert(std::make_pair("MESSAGES", common::listToString(msgs, ",\n", common::emptyString())));

            if (interface == nullptr) {
                replacements.insert(std::make_pair("TEMPLATE_PARAM", "template <typename TInterface>"));
            }

            const std::string Template(
                "#^#GEN_COMMENT#$#\n"
                "#pragma once\n\n"
                "#^#INCLUDES#$#\n"
                "#^#BEG_NAMESPACE#$#\n"
                "#^#TEMPLATE_PARAM#$#\n"
                "using #^#CLASS_NAME#$# =\n"
                "    std::tuple<\n"
                "        #^#MESSAGES#$#\n"
                "    >;\n\n"
                "#^#END_NAMESPACE#$#\n"
            );            

            auto str = common::processTemplate(Template, replacements);
            stream << str;

            stream.flush();
            if (!stream.good()) {
                m_generator.logger().error("Failed to write \"" + filePath + "\".");
                return false;
            }
            return true;
        };


    if (!writeFileFunc(includes, messages, common::allMessagesStr())) {
        return false;
    }

    for (auto& p : platformsMap) {
        auto n = common::nameToClassCopy(p.first) + "Messages";
        if (n == common::allMessagesStr()) {
            m_generator.logger().error("Invalid platform name: \"" + p.first + "\".");
            return false;
        }

        if (!writeFileFunc(p.second.m_includes, p.second.m_messages, n)) {
            return false;
        }
    }
    return true;
}

} // namespace commsdsl2old