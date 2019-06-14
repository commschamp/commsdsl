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
#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2comms
{

bool Dispatch::write(Generator& generator)
{
    Dispatch obj(generator);
    return obj.writeProtocolDefinition();
}


bool Dispatch::writeProtocolDefinition() const
{
    struct MessagesInfo
    {
        DslMessagesList m_messages;
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

    auto& platforms = m_generator.platforms();
    for (auto& p : platforms) {
        platformsMap.insert(std::make_pair(p, PlatformInfo()));
    };

    auto allMessages = m_generator.getAllDslMessages();

    for (auto& p : platformsMap) {
        auto updateFunc =
            [this, &allMessages](auto& msgInfo, const std::string& inputName)
            {
                msgInfo.m_messages.reserve(allMessages.size());
                common::mergeInclude("<type_traits>", msgInfo.m_includes);
                auto inputHeader = m_generator.headerfileForInput(inputName, false);
                common::mergeInclude(inputHeader, msgInfo.m_includes);
                auto msgIdHeader = m_generator.headerfileForRoot(common::msgIdEnumNameStr(), false);
                common::mergeInclude(msgIdHeader, msgInfo.m_includes);

                auto interfaces = m_generator.getAllInterfaces();
                for (auto& i : interfaces) {
                    auto inc = m_generator.headerfileForInterface(i->externalRef(), false);
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

        if (!m_generator.doesElementExist(m.sinceVersion(), m.deprecatedSince(), m.isDeprecatedRemoved())) {
            continue;
        }

        auto extRef = m.externalRef();
        assert(!extRef.empty());

        auto addToMessageInfoFunc =
            [m](MessagesInfo& info)
            {
                info.m_messages.push_back(m);
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
        [this](const MessagesInfo& info,
               const std::string& fileName,
               const std::string& platName = common::emptyString(),
               const std::string& inputName = common::emptyString())
        {
            auto startInfo = m_generator.startDispatchProtocolWrite(fileName);
            auto& filePath = startInfo.first;
            auto& funcName = startInfo.second;
            static_cast<void>(funcName); // TODO: remove

            if (filePath.empty()) {
                return true;
            }

            std::ofstream stream(filePath);
            if (!stream) {
                m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
                return false;
            }

            common::StringsList funcs;
            for (auto& i : m_generator.getAllInterfaces()) {
                funcs.push_back(
                    getDispatchFunc(
                        common::nameToAccessCopy(fileName),
                        i->externalRef(),
                        info.m_messages));
            }

            common::ReplacementMap replacements;
            auto namespaces = m_generator.namespacesForDispatch();
            replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
            replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
            replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(info.m_includes)));
            replacements.insert(std::make_pair("FUNCS", common::listToString(funcs, "\n", common::emptyString())));

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
                "#^#FUNCS#$#\n"
                "#^#END_NAMESPACE#$#\n";

            auto str = common::processTemplate(Templ, replacements);
            stream << str;

            stream.flush();
            if (!stream.good()) {
                m_generator.logger().error("Failed to write \"" + filePath + "\".");
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
                m_generator.logger().error("Invalid platform name: \"" + p.first + "\".");
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

std::string Dispatch::getDispatchFunc(
    const std::string& funcName,
    const std::string& interface,
    const DslMessagesList& messages) const
{
    using MsgMap = std::map<std::uintmax_t, DslMessagesList>;
    MsgMap msgMap;
    for (auto& m : messages) {
        msgMap[m.id()].push_back(m);
    }

    common::StringsList cases;
    for (auto& elem : msgMap) {
        auto& msgList = elem.second;
        assert(!msgList.empty());
        auto idStr =
                common::msgIdPrefixStr() +
                ba::replace_all_copy(msgList.front().externalRef(), ".", "_");

        static const std::string MsgCaseTempl =
            "case #^#MSG_ID#$#:\n"
            "{\n"
            "    using MsgType = #^#MSG_TYPE#$#<InterfaceType, TProtOptions>;\n"
            "    auto& castedMsg = static_cast<MsgType&>(msg);\n"
            "    return handler.handle(castedMsg);\n"
            "}";

        if (msgList.size() == 1) {
            common::ReplacementMap repl;
            repl.insert(std::make_pair("MSG_ID", idStr));
            repl.insert(std::make_pair("MSG_TYPE", m_generator.scopeForMessage(msgList.front().externalRef(), true, true)));
            cases.push_back(common::processTemplate(MsgCaseTempl, repl));
            continue;
        }

        common::StringsList offsetCases;
        for (auto idx=0U; idx < msgList.size(); ++idx) {
            common::ReplacementMap repl;
            repl.insert(std::make_pair("MSG_ID", common::numToString(idx)));
            repl.insert(std::make_pair("MSG_TYPE", m_generator.scopeForMessage(msgList[idx].externalRef(), true, true)));
            offsetCases.push_back(common::processTemplate(MsgCaseTempl, repl));
        }

        common::ReplacementMap repl;
        repl.insert(std::make_pair("MSG_ID", idStr));
        repl.insert(std::make_pair("IDX_CASES", common::listToString(offsetCases, "\n", common::emptyString())));

        static const std::string Templ =
            "case #^#MSG_ID#$#:\n"
            "{\n"
            "    switch (idx) {\n"
            "    #^#IDX_CASES#$#\n"
            "    default:\n"
            "        return handler.dispatch(msg);\n"
            "    };\n"
            "}";
        cases.push_back(common::processTemplate(Templ, repl));
    }

    common::ReplacementMap repl;
    repl.insert(std::make_pair("FUNC", funcName));
    repl.insert(std::make_pair("MSG_ID_TYPE", m_generator.scopeForRoot(common::msgIdEnumNameStr(), true, true)));
    repl.insert(std::make_pair("INTERFACE", m_generator.scopeForInterface(interface, true, true)));
    repl.insert(std::make_pair("CASES", common::listToString(cases, "\n", common::emptyString())));

    static const std::string Templ =
        "template<tepename TProtOptions, typename THandler, typename... TOpt>\n"
        "auto #^#FUNC#$#(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    std::size_t idx,\n"
        "    #^#INTERFACE#$#<TOpt...>& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    using InterfaceType = typename std::decay<decltype(msg)>::type;"
        "    switch(id) {\n"
        "    #^#CASES#$#\n"
        "    default:\n"
        "        break;\n"
        "    };\n\n"
        "    return handler.dispatch(msg);\n"
        "}\n";

    return common::processTemplate(Templ, repl);
}


} // namespace commsdsl2comms
