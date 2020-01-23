//
// Copyright 2019 - 2020 (C). Alex Robenko. All rights reserved.
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
#include "EnumField.h"

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
            [this, &allMessages](auto& msgInfo, const std::string& inputName)
            {
                msgInfo.m_messages.reserve(allMessages.size());
                common::mergeInclude("<type_traits>", msgInfo.m_includes);
                auto inputHeader = m_generator.headerfileForInput(inputName, false);
                common::mergeInclude(inputHeader, msgInfo.m_includes);
                auto msgIdHeader = m_generator.headerfileForRoot(common::msgIdEnumNameStr(), false);
                common::mergeInclude(msgIdHeader, msgInfo.m_includes);
            };

        auto& inputPrefix = p.first;
        if (inputPrefix.empty()) {
            updateFunc(p.second.m_all, common::allMessagesStr());
        }
        else {
            updateFunc(p.second.m_all, inputPrefix + "Messages");
        }

        updateFunc(p.second.m_serverInput, inputPrefix + common::serverInputMessagesStr());
        updateFunc(p.second.m_clientInput, inputPrefix + common::clientInputMessagesStr());
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
                if (p.first.empty() || (!p.second.m_realPlatform)) {
                    continue;
                }
                addToPlatformInfoFunc(p.second);
            }
        }

        for (auto& p : msgPlatforms) {
            auto iter = platformsMap.find(p);
            if (iter == platformsMap.end()) {
                assert(!"Should not happen");
                continue;
            }

            addToPlatformInfoFunc(iter->second);
        }

        auto inBundles = m_generator.bundlesForMessage(extRef);
        for (auto& b : inBundles) {
            auto iter = platformsMap.find(b);
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

            if (filePath.empty()) {
                return true;
            }

            std::ofstream stream(filePath);
            if (!stream) {
                m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
                return false;
            }

            auto func =
                getDispatchFunc(
                    common::nameToAccessCopy(fileName),
                    info.m_messages);

            common::ReplacementMap replacements;
            auto namespaces = m_generator.namespacesForDispatch();
            replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
            replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
            replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
            replacements.insert(std::make_pair("INCLUDES", common::includesToStatements(info.m_includes)));
            replacements.insert(std::make_pair("FUNCS", std::move(func)));
            replacements.insert(std::make_pair("DISPATCHERS", getMsgDispatcher(common::nameToClassCopy(fileName))));


            if (!platName.empty()) {
                replacements.insert(std::make_pair("PLAT_NAME", '\"' + platName + "\" "));
            }

            if (!inputName.empty()) {
                replacements.insert(std::make_pair("INPUT", inputName + " input "));
            }

            static const std::string Templ =
                "#^#GEN_COMMENT#$#\n"
                "/// @file\n"
                "/// @brief Contains dispatch to handling function(s) for #^#PLAT_NAME#$##^#INPUT#$#messages.\n\n"
                "#pragma once\n\n"
                "#^#INCLUDES#$#\n"
                "#^#BEG_NAMESPACE#$#\n"
                "#^#FUNCS#$#\n"
                "#^#DISPATCHERS#$#\n"
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
        static const std::string MessageSuffix = "Message";
        auto platformName = common::nameToClassCopy(p.first);
        std::string allName = DispatchPrefix + platformName + MessageSuffix;
        std::string serverName = (DispatchPrefix + platformName + common::serverInputStr() + MessageSuffix);
        std::string clientName = (DispatchPrefix + platformName + common::clientInputStr() + MessageSuffix);

        if (!writeFileFunc(p.second.m_all, allName, p.first, "all")) {
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
    const DslMessagesList& messages) const
{
    using MsgMap = std::map<std::uintmax_t, DslMessagesList>;
    MsgMap msgMap;
    for (auto& m : messages) {
        auto& msgList = msgMap[m.id()];
        assert((msgList.empty()) || (msgList.back().name() != m.name())); // Make sure message is not inserted twice
        msgList.push_back(m);
    }

    bool hasMultipleMessagesWithSameId =
        std::any_of(
            msgMap.begin(), msgMap.end(),
            [](auto& elem)
            {
                return 1U < elem.second.size();
            });

    common::StringsList cases;
    for (auto& elem : msgMap) {
        auto& msgList = elem.second;
        assert(!msgList.empty());
        auto idStr = getIdString(elem.first);

        static const std::string MsgCaseTempl =
            "case #^#MSG_ID#$#:\n"
            "{\n"
            "    using MsgType = #^#MSG_TYPE#$#<InterfaceType, TProtOptions>;\n"
            "    return handler.handle(static_cast<MsgType&>(msg));\n"
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
            "        return handler.handle(msg);\n"
            "    };\n"
            "    break;\n"
            "}";
        cases.push_back(common::processTemplate(Templ, repl));
    }

    auto allInterfaces = m_generator.getAllInterfaces();
    assert(!allInterfaces.empty());

    std::string msg1Name;
    std::string msg2Name;
    if (2 <= messages.size()) {
        msg1Name = messages[0].externalRef();
        msg2Name = messages[1].externalRef();
    }
    else if (1 == messages.size()){
        msg1Name = messages[0].externalRef();
        msg2Name = "SomeOtherMessage";
    }
    else {
        msg1Name = "SomeMessage";
        msg2Name = "SomeOtherMessage";
    }


    common::ReplacementMap repl;
    repl.insert(std::make_pair("FUNC", funcName));
    repl.insert(std::make_pair("MSG_ID_TYPE", m_generator.scopeForRoot(common::msgIdEnumNameStr(), true, true)));
    repl.insert(std::make_pair("CASES", common::listToString(cases, "\n", common::emptyString())));
    repl.insert(std::make_pair("DEFAULT_OPTIONS", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    repl.insert(std::make_pair("INTERFACE", m_generator.scopeForInterface(allInterfaces.front()->externalRef(), true, true)));
    repl.insert(std::make_pair("MSG1_NAME", msg1Name));
    repl.insert(std::make_pair("MSG2_NAME", msg2Name));
    repl.insert(std::make_pair("MSG1", m_generator.scopeForMessage(msg1Name, true, true)));
    repl.insert(std::make_pair("MSG2", m_generator.scopeForMessage(msg2Name, true, true)));
    repl.insert(std::make_pair("HEADERFILE", m_generator.headerfileForDispatch(common::nameToClassCopy(funcName), false)));

    static const std::string SingleMessagePerIdTempl =
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details @b switch statement based (on message ID) cast and dispatch functionality.\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object. Must define\n"
        "///     @b handle() member function for every message type it exects\n"
        "///     to handle and one for the interface class as well.\n"
        "///     @code\n"
        "///     using MyInterface = #^#INTERFACE#$#<...>;\n"
        "///     using My#^#MSG1_NAME#$# = #^#MSG1#$#<MyInterface, #^#DEFAULT_OPTIONS#$#>;\n"
        "///     using My#^#MSG2_NAME#$# = #^#MSG2#$#<MyInterface, #^#DEFAULT_OPTIONS#$#>;\n"
        "///     struct MyHandler {\n"
        "///         void handle(My#^#MSG1_NAME#$#& msg) {...}\n"
        "///         void handle(My#^#MSG2_NAME#$#& msg) {...}\n"
        "///         ...\n"
        "///         // Handle all unexpected or irrelevant messages.\n"
        "///         void handle(MyInterface& msg) {...}\n"
        "///     };\n"
        "///     @endcode\n"
        "///     Every @b handle() function may return a value, but every\n"
        "///     function must return the @b same type.\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TProtOptions, typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    using InterfaceType = typename std::decay<decltype(msg)>::type;\n"
        "    switch(id) {\n"
        "    #^#CASES#$#\n"
        "    default:\n"
        "        break;\n"
        "    };\n\n"
        "    return handler.handle(msg);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other #^#FUNC#$#(), but receives extra @b idx parameter.\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] idx Index of the message among messages with the same ID.\n"
        "///     Expected to be @b 0.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see #^#FUNC#$#()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TProtOptions, typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    if (idx != 0U) {\n"
        "        return handler.handle(msg);\n"
        "    }\n"
        "    return #^#FUNC#$#<TProtOptions>(id, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other #^#FUNC#$#(), but passing\n"
        "///     #^#DEFAULT_OPTIONS#$# as first template parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see #^#FUNC#$#()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#DefaultOptions(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return #^#FUNC#$#<#^#DEFAULT_OPTIONS#$#>(id, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other #^#FUNC#$#DefaultOptions(), \n"
        "///     but receives extra @b idx parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] idx Index of the message among messages with the same ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see #^#FUNC#$#DefaultOptions()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#DefaultOptions(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return #^#FUNC#$#<#^#DEFAULT_OPTIONS#$#>(id, idx, msg, handler);\n"
        "}\n";

    static const std::string MultipleMessagesPerIdTempl =
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details @b switch statement based (on message ID) cast and dispatch functionality.\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] idx Index of the message among messages with the same ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object. Must define\n"
        "///     @b handle() member function for every message type it exects\n"
        "///     to handle and one for the interface class as well.\n"
        "///     @code\n"
        "///     using MyInterface = #^#INTERFACE#$#<...>;\n"
        "///     using My#^#MSG1_NAME#$# = #^#MSG1#$#<MyInterface, #^#DEFAULT_OPTIONS#$#>;\n"
        "///     using My#^#MSG2_NAME#$# = #^#MSG2#$#<MyInterface, #^#DEFAULT_OPTIONS#$#>;\n"
        "///     struct MyHandler {\n"
        "///         void handle(My#^#MSG1_NAME#$#& msg) {...}\n"
        "///         void handle(My#^#MSG2_NAME#$#& msg) {...}\n"
        "///         ...\n"
        "///         // Handle all unexpected or irrelevant messages.\n"
        "///         void handle(MyInterface& msg) {...}\n"
        "///     };\n"
        "///     @endcode\n"
        "///     Every @b handle() function may return a value, but every\n"
        "///     function must return the @b same type.\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TProtOptions, typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    using InterfaceType = typename std::decay<decltype(msg)>::type;\n"
        "    switch(id) {\n"
        "    #^#CASES#$#\n"
        "    default:\n"
        "        break;\n"
        "    };\n\n"
        "    return handler.handle(msg);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other #^#FUNC#$#(), but without @b idx parameter.\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see #^#FUNC#$#()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TProtOptions, typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return #^#FUNC#$#<TProtOptions>(id, 0U, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other #^#FUNC#$#(), but passing\n"
        "///     #^#DEFAULT_OPTIONS#$# as first template parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] idx Index of the message among messages with the same ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see #^#FUNC#$#()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#DefaultOptions(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return #^#FUNC#$#<#^#DEFAULT_OPTIONS#$#>(id, idx, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other #^#FUNC#$#DefaultOptions(), \n"
        "///     but without @b idx parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see #^#FUNC#$#DefaultOptions()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TMsg, typename THandler>\n"
        "auto #^#FUNC#$#DefaultOptions(\n"
        "    #^#MSG_ID_TYPE#$# id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return #^#FUNC#$#<#^#DEFAULT_OPTIONS#$#>(id, msg, handler);\n"
        "}\n";

    auto* templ = &SingleMessagePerIdTempl;
    if (hasMultipleMessagesWithSameId) {
        templ = &MultipleMessagesPerIdTempl;
    }


    return common::processTemplate(*templ, repl);
}

std::string Dispatch::getMsgDispatcher(
    const std::string& fileName) const
{
    auto name = ba::replace_first_copy(fileName, "Dispatch", "");
    ba::replace_last(name, "Message", "");
    static const std::string Templ =
        "/// @brief Message dispatcher class to be used with\n"
        "///     @b comms::processAllWithDispatchViaDispatcher() function (or similar).\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @headerfile #^#HEADERFILE#$#\n"
        "template <typename TProtOptions>\n"
        "struct #^#NAME#$#MsgDispatcher\n"
        "{\n"
        "    /// @brief Class detection tag\n"
        "    using MsgDispatcherTag = void;\n\n"
        "    /// @brief Dispatch message to its handler.\n"
        "    /// @details Uses appropriate @ref dispatch#^#NAME#$#Message() function.\n"
        "    /// @param[in] id ID of the message.\n"
        "    /// @param[in] idx Index (or offset) of the message among those having the same numeric ID.\n"
        "    /// @param[in] msg Reference to message object.\n"
        "    /// @param[in] handler Reference to handler object.\n"
        "    /// @return What the @ref dispatch#^#NAME#$#Message() function returns.\n"
        "    template <typename TMsg, typename THandler>\n"
        "    static auto dispatch(#^#MAIN_NS#$#::MsgId id, std::size_t idx, TMsg& msg, THandler& handler) ->\n"
        "        decltype(#^#MAIN_NS#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, idx, msg, handler))\n"
        "    {\n"
        "        return #^#MAIN_NS#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, idx, msg, handler);\n"
        "    }\n\n"
        "    /// @brief Complementary dispatch function.\n"
        "    /// @details Same as other dispatch without @b TAllMessages template parameter,\n"
        "    ///     used by  @b comms::processAllWithDispatchViaDispatcher().\n"
        "    template <typename TAllMessages, typename TMsg, typename THandler>\n"
        "    static auto dispatch(#^#MAIN_NS#$#::MsgId id, std::size_t idx, TMsg& msg, THandler& handler) ->\n"
        "        decltype(dispatch(id, idx, msg, handler))\n"
        "    {\n"
        "        return dispatch(id, idx, msg, handler);\n"
        "    }\n\n"
        "    /// @brief Dispatch message to its handler.\n"
        "    /// @details Uses appropriate @ref dispatch#^#NAME#$#Message() function.\n"
        "    /// @param[in] id ID of the message.\n"
        "    /// @param[in] msg Reference to message object.\n"
        "    /// @param[in] handler Reference to handler object.\n"
        "    /// @return What the @ref dispatch#^#NAME#$#Message() function returns.\n"
        "    template <typename TMsg, typename THandler>\n"
        "    static auto dispatch(#^#MAIN_NS#$#::MsgId id, TMsg& msg, THandler& handler) ->\n"
        "        decltype(#^#MAIN_NS#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, msg, handler))\n"
        "    {\n"
        "        return #^#MAIN_NS#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, msg, handler);\n"
        "    }\n\n"
        "    /// @brief Complementary dispatch function.\n"
        "    /// @details Same as other dispatch without @b TAllMessages template parameter,\n"
        "    ///     used by  @b comms::processAllWithDispatchViaDispatcher().\n"
        "    template <typename TAllMessages, typename TMsg, typename THandler>\n"
        "    static auto dispatch(#^#MAIN_NS#$#::MsgId id, TMsg& msg, THandler& handler) ->\n"
        "        decltype(dispatch(id, msg, handler))\n"
        "    {\n"
        "        return dispatch(id, msg, handler);\n"
        "    }\n"
        "};\n\n"
        "/// @brief Message dispatcher class to be used with\n"
        "///     @b comms::processAllWithDispatchViaDispatcher() function (or similar).\n"
        "/// @details Same as @ref #^#NAME#$#MsgDispatcher, but passing\n"
        "///     @ref #^#DEFAULT_OPTIONS#$# as template parameter.\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "using #^#NAME#$#MsgDispatcherDefaultOptions =\n"
        "    #^#NAME#$#MsgDispatcher<#^#DEFAULT_OPTIONS#$#>;\n";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("NAME", name));
    repl.insert(std::make_pair("MAIN_NS", m_generator.mainNamespace()));
    repl.insert(std::make_pair("DEFAULT_OPTIONS", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    repl.insert(std::make_pair("HEADERFILE", m_generator.headerfileForDispatch(fileName)));
    return common::processTemplate(Templ, repl);
}


std::string Dispatch::getIdString(std::uintmax_t value) const
{
    auto numValueFunc =
        [this, value]()
        {
            return
                "static_cast<" +
                m_generator.scopeForRoot(common::msgIdEnumNameStr(), true, true) +
                ">(" +
                common::numToString(value) +
                ")";
        };

    auto* idField = m_generator.getMessageIdField();
    if (idField == nullptr) {
        return numValueFunc();
    }

    if (idField->kind() != commsdsl::Field::Kind::Enum) {
        return numValueFunc();
    }

    auto* castedMsgIdField = static_cast<const EnumField*>(idField);
    auto valStr = castedMsgIdField->getValueName(static_cast<std::intmax_t>(value));
    if (valStr.empty()) {
        return numValueFunc();
    }

    return m_generator.scopeForRoot(common::msgIdPrefixStr() + valStr, true, true);
}

} // namespace commsdsl2comms
