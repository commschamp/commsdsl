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

#include "CommsDispatch.h"

#include "CommsGenerator.h"
#include "CommsNamespace.h"

#include "commsdsl/gen/EnumField.h"
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

auto getFileName(const std::string& desc = std::string())
{
    return "Dispatch" + desc + "Message";
}    

bool writeFileInternal(
    const std::string& name,
    CommsGenerator& generator,
    const CommsNamespace& ns,
    const std::string& data)
{
    auto filePath = comms::headerPathForDispatch(name, generator, ns);
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

const std::string& dispatchTempl()
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains dispatch to handling function(s) for #^#DESC#$# input messages.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n\n"
        "namespace #^#DISPATCH_NAMESPACE#$#\n"
        "{\n\n"
        "#^#CODE#$#\n\n"
        "} // namespace #^#DISPATCH_NAMESPACE#$#\n\n"
        "#^#NS_END#$#\n";

    return Templ;
}

util::ReplacementMap initialRepl(const CommsGenerator& generator, const commsdsl::gen::Elem& elem)
{
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"DISPATCH_NAMESPACE", strings::dispatchNamespaceStr()},
        {"NS_BEGIN", comms::namespaceBeginFor(elem, generator)},
        {"NS_END", comms::namespaceEndFor(elem, generator)},        
    };
    return repl;
}

const std::string& singleMessagePerIdTempl()
{
    static const std::string Templ =
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
        "template<typename TProtOptions, typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#Message(\n"
        "    TId id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    using InterfaceType = typename std::decay<decltype(msg)>::type;\n"
        "    switch(static_cast<std::intmax_t>(id)) {\n"
        "    #^#CASES#$#\n"
        "    default:\n"
        "        break;\n"
        "    };\n\n"
        "    return handler.handle(msg);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other dispatch#^#NAME#$#Message(), but receives extra @b idx parameter.\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] idx Index of the message among messages with the same ID.\n"
        "///     Expected to be @b 0.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see dispatch#^#NAME#$#Message()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TProtOptions, typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#Message(\n"
        "    TId id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    if (idx != 0U) {\n"
        "        return handler.handle(msg);\n"
        "    }\n"
        "    return dispatch#^#NAME#$#Message<TProtOptions>(id, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other dispatch#^#NAME#$#Message(), but passing\n"
        "///     #^#DEFAULT_OPTIONS#$# as first template parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see dispatch#^#NAME#$#Message()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#MessageDefaultOptions(\n"
        "    TId id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return dispatch#^#NAME#$#Message<#^#DEFAULT_OPTIONS#$#>(id, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other dispatch#^#NAME#$#MessageDefaultOptions(), \n"
        "///     but receives extra @b idx parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] idx Index of the message among messages with the same ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see dispatch#^#NAME#$#MessageDefaultOptions()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#MessageDefaultOptions(\n"
        "    TId id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return dispatch#^#NAME#$#Message<#^#DEFAULT_OPTIONS#$#>(id, idx, msg, handler);\n"
        "}\n\n"
        "#^#DISPATCHER#$#\n"; 
    return Templ;
}

const std::string& multipleMessagesPerIdTempl()
{
    static const std::string Templ =
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
        "template<typename TProtOptions, typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#Message(\n"
        "    TId id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    using InterfaceType = typename std::decay<decltype(msg)>::type;\n"
        "    switch(static_cast<std::intmax_t>(id)) {\n"
        "    #^#CASES#$#\n"
        "    default:\n"
        "        break;\n"
        "    };\n\n"
        "    return handler.handle(msg);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other dispatch#^#NAME#$#Message(), but without @b idx parameter.\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see dispatch#^#NAME#$#Message()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TProtOptions, typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#Message(\n"
        "    TId id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return dispatch#^#NAME#$#Message<TProtOptions>(id, 0U, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other dispatch#^#NAME#$#Message(), but passing\n"
        "///     #^#DEFAULT_OPTIONS#$# as first template parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] idx Index of the message among messages with the same ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see dispatch#^#NAME#$#Message()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#MessageDefaultOptions(\n"
        "    TId id,\n"
        "    std::size_t idx,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return dispatch#^#NAME#$#Message<#^#DEFAULT_OPTIONS#$#>(id, idx, msg, handler);\n"
        "}\n\n"
        "/// @brief Dispatch message object to its appropriate handling function.\n"
        "/// @details Same as other dispatch#^#NAME#$#MessageDefaultOptions(), \n"
        "///     but without @b idx parameter.\n"
        "/// @param[in] id Numeric message ID.\n"
        "/// @param[in] msg Message object held by reference to its interface class.\n"
        "/// @param[in] handler Reference to handling object.\n"
        "/// @see dispatch#^#NAME#$#MessageDefaultOptions()\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "template<typename TId, typename TMsg, typename THandler>\n"
        "auto dispatch#^#NAME#$#MessageDefaultOptions(\n"
        "    TId id,\n"
        "    TMsg& msg,\n"
        "    THandler& handler) -> decltype(handler.handle(msg))\n"
        "{\n"
        "    return dispatch#^#NAME#$#Message<#^#DEFAULT_OPTIONS#$#>(id, msg, handler);\n"
        "}\n\n"
        "#^#DISPATCHER#$#\n"; 
    return Templ;
}

} // namespace 
    
CommsDispatch::CommsDispatch(CommsGenerator& generator, const CommsNamespace& parent) :
    m_generator(generator),
    m_parent(parent)
{
}

bool CommsDispatch::commsWrite() const
{
    // auto& thisSchema = static_cast<CommsSchema&>(m_generator.currentSchema());
    // if ((!m_generator.isCurrentProtocolSchema()) && (!thisSchema.commsHasAnyMessage())) {
    //     return true;
    // }

    return
        commsWriteDispatchInternal() &&
        commsWriteClientDispatchInternal() &&
        commsWriteServerDispatchInternal() &&
        commsWritePlatformDispatchInternal() &&
        commsWriteExtraDispatchInternal();
}

bool CommsDispatch::commsWriteDispatchInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg) noexcept
        {
            static_cast<void>(msg);
            return true;
        };

    util::ReplacementMap repl = initialRepl(m_generator, m_parent);
    repl.insert({
        {"DESC", "all"},
        {"INCLUDES", commsIncludesInternal("All")},
        {"CODE", commsDispatchCodeInternal(std::string(), std::move(checkFunc))}
    });
    
    return writeFileInternal(getFileName(), m_generator, m_parent, util::processTemplate(dispatchTempl(), repl, true));
}

bool CommsDispatch::commsWriteClientDispatchInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg) noexcept
        {
            return msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client;
        };

    util::ReplacementMap repl = initialRepl(m_generator, m_parent);
    std::string inputPrefix = "ClientInput";
    repl.insert({
        {"DESC", "client input"},
        {"INCLUDES", commsIncludesInternal(inputPrefix)},
        {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(checkFunc))}
    });

    return writeFileInternal(getFileName(inputPrefix), m_generator, m_parent, util::processTemplate(dispatchTempl(), repl, true));
}

bool CommsDispatch::commsWriteServerDispatchInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg) noexcept
        {
            return msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server;
        };

    util::ReplacementMap repl = initialRepl(m_generator, m_parent);
    std::string inputPrefix = "ServerInput";
    repl.insert({
        {"DESC", "client input"},
        {"INCLUDES", commsIncludesInternal(inputPrefix)},
        {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(checkFunc))}
    });

    return writeFileInternal(getFileName(inputPrefix), m_generator, m_parent, util::processTemplate(dispatchTempl(), repl, true));
}

bool CommsDispatch::commsWritePlatformDispatchInternal() const
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

        do {
            auto allCheckFunc = 
                [&platformCheckFunc](const commsdsl::gen::Message& msg)
                {
                    return platformCheckFunc(msg);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::className(p);
            repl.insert({
                {"DESC", p + " platform"},
                {"INCLUDES", commsIncludesInternal(inputPrefix)},
                {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(allCheckFunc))}
            });

            bool result = 
                writeFileInternal(
                    getFileName(inputPrefix), 
                    m_generator, 
                    m_parent, 
                    util::processTemplate(dispatchTempl(), repl, true));
                    
            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto clientCheckFunc = 
                [&platformCheckFunc](const commsdsl::gen::Message& msg)
                {
                    return 
                        platformCheckFunc(msg) &&
                        (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::className(p) + "ClientInput";
            repl.insert({
                {"DESC", p + " platform client input"},
                {"INCLUDES", commsIncludesInternal(inputPrefix)},
                {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(clientCheckFunc))}
            });

            bool result = 
                writeFileInternal(
                    getFileName(inputPrefix), 
                    m_generator, 
                    m_parent, 
                    util::processTemplate(dispatchTempl(), repl, true));

            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto serverCheckFunc = 
                [&platformCheckFunc](const commsdsl::gen::Message& msg)
                {
                    return 
                        platformCheckFunc(msg) &&
                        (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::className(p) + "ServerInput";
            repl.insert({
                {"DESC", p + " platform server input"},
                {"INCLUDES", commsIncludesInternal(inputPrefix)},
                {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(serverCheckFunc))}
            });

            bool result = 
                writeFileInternal(
                    getFileName(inputPrefix), 
                    m_generator, 
                    m_parent, 
                    util::processTemplate(dispatchTempl(), repl, true));

            if (!result) {
                return false;
            }

        } while (false);        
    }

    return true;
}

bool CommsDispatch::commsWriteExtraDispatchInternal() const
{
    auto& extraBundles = m_generator.commsExtraMessageBundles();
    for (auto& b : extraBundles) {

        auto bundleCheckFunc = 
            [&b](const commsdsl::gen::Message& msg)
            {
                return std::find(b.second.begin(), b.second.end(), &msg) != b.second.end();
            };

        do {
            auto allCheckFunc = 
                [&bundleCheckFunc](const commsdsl::gen::Message& msg)
                {
                    return bundleCheckFunc(msg);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::className(b.first);
            repl.insert({
                {"DESC", b.first + " bundle"},
                {"INCLUDES", commsIncludesInternal(inputPrefix)},
                {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(allCheckFunc))}
            });

            bool result = 
                writeFileInternal(
                    getFileName(inputPrefix), 
                    m_generator, 
                    m_parent, 
                    util::processTemplate(dispatchTempl(), repl, true));
                    
            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto clientCheckFunc = 
                [&bundleCheckFunc](const commsdsl::gen::Message& msg)
                {
                    return 
                        bundleCheckFunc(msg) &&
                        (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Client);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::className(b.first) + "ClientInput";
            repl.insert({
                {"DESC", b.first + " bundle client input"},
                {"INCLUDES", commsIncludesInternal(inputPrefix)},
                {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(clientCheckFunc))}
            });

            bool result = 
                writeFileInternal(
                    getFileName(inputPrefix), 
                    m_generator, 
                    m_parent, 
                    util::processTemplate(dispatchTempl(), repl, true));

            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto serverCheckFunc = 
                [&bundleCheckFunc](const commsdsl::gen::Message& msg)
                {
                    return 
                        bundleCheckFunc(msg) &&
                        (msg.dslObj().sender() != commsdsl::parse::Message::Sender::Server);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::className(b.first) + "ServerInput";
            repl.insert({
                {"DESC", b.first + " bundle server input"},
                {"INCLUDES", commsIncludesInternal(inputPrefix)},
                {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(serverCheckFunc))}
            });

            bool result = 
                writeFileInternal(
                    getFileName(inputPrefix), 
                    m_generator, 
                    m_parent, 
                    util::processTemplate(dispatchTempl(), repl, true));

            if (!result) {
                return false;
            }

        } while (false);        
    }

    return true;
}

std::string CommsDispatch::commsIncludesInternal(const std::string& inputPrefix) const
{
    util::StringsList incs = {
        "<cstdint>",
        comms::relHeaderForInput(inputPrefix + "Messages", m_generator, m_parent),
        comms::relHeaderForOptions(strings::defaultOptionsClassStr(), m_generator),
    };

    comms::prepareIncludeStatement(incs);
    return util::strListToString(incs, "\n", "\n");
}

std::string CommsDispatch::commsDispatchCodeInternal(const std::string& name, CheckMsgFunc&& func) const
{
    MessagesMap map;
    auto allMessages = m_parent.getAllMessagesIdSorted();
    bool hasMultipleMessagesWithSameId = false;
    const commsdsl::gen::Message* firstMsg = nullptr;
    const commsdsl::gen::Message* secondMsg = nullptr;
    for (auto* m : allMessages) {
        assert(m != nullptr);
        if (!func(*m)) {
            continue;
        }
        auto& mList = map[m->dslObj().id()];
        hasMultipleMessagesWithSameId = hasMultipleMessagesWithSameId || (!mList.empty());
        mList.push_back(m);

        if (firstMsg == nullptr) {
            firstMsg = m;
            continue;
        }

        if (secondMsg == nullptr) {
            secondMsg = m;
            continue;
        }
    }

    auto allInterfaces = m_generator.getAllInterfaces();
    // assert(!allInterfaces.empty());

    util::ReplacementMap repl = {
        {"NAME", name},
        {"DEFAULT_OPTIONS", comms::scopeForOptions(strings::defaultOptionsClassStr(), m_generator)},
        {"HEADERFILE", comms::relHeaderForDispatch(getFileName(name), m_generator, m_parent)},
        {"INTERFACE", (!allInterfaces.empty()) ? comms::scopeFor(*allInterfaces.front(), m_generator) : std::string("SomeInterface")},
        {"MSG1", firstMsg != nullptr ? comms::scopeFor(*firstMsg, m_generator) : std::string("SomeMessage")},
        {"MSG2", secondMsg != nullptr ? comms::scopeFor(*secondMsg, m_generator) : std::string("SomeOtherMessage")},
        {"MSG1_NAME", firstMsg != nullptr ? comms::className(firstMsg->dslObj().name()) : std::string("SomeMessage")},
        {"MSG2_NAME", secondMsg != nullptr ? comms::className(secondMsg->dslObj().name()) : std::string("SomeOtherMessage")},
        {"CASES", commsCasesCodeInternal(map)},
        {"DISPATCHER", commsMsgDispatcherCodeInternal(name)},
    };

    auto& templ = hasMultipleMessagesWithSameId ? multipleMessagesPerIdTempl() : singleMessagePerIdTempl();
    return util::processTemplate(templ, repl);
}

std::string CommsDispatch::commsCasesCodeInternal(const MessagesMap& map) const
{
    util::StringsList cases;
    for (auto& elem : map) {
        auto& msgList = elem.second;
        assert(!msgList.empty());
        auto idStr = util::numToStringWithHexComment(elem.first);

        static const std::string MsgCaseTempl =
            "case #^#MSG_ID#$#:\n"
            "{\n"
            "    using MsgType = #^#MSG_TYPE#$#<InterfaceType, TProtOptions>;\n"
            "    return handler.handle(static_cast<MsgType&>(msg));\n"
            "}";

        if (msgList.size() == 1) {
            util::ReplacementMap repl = {
                {"MSG_ID", idStr},
                {"MSG_TYPE", comms::scopeFor(*msgList.front(), m_generator)},
            };
            cases.push_back(util::processTemplate(MsgCaseTempl, repl));
            continue;
        }

        util::StringsList offsetCases;
        for (auto idx = 0U; idx < msgList.size(); ++idx) {
            util::ReplacementMap repl = {
                {"MSG_ID", util::numToString(idx)},
                {"MSG_TYPE", comms::scopeFor(*msgList[idx], m_generator)},
            };
            offsetCases.push_back(util::processTemplate(MsgCaseTempl, repl));
        }

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


        util::ReplacementMap repl = {
            {"MSG_ID", idStr},
            {"IDX_CASES", util::strListToString(offsetCases, "\n", "")},
        };

        cases.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(cases, "\n", "");
}

std::string CommsDispatch::commsMsgDispatcherCodeInternal(const std::string& inputPrefix) const
{
    static const std::string Templ =
        "/// @brief Message dispatcher class to be used with\n"
        "///     @b comms::processAllWithDispatchViaDispatcher() function (or similar).\n"
        "/// @tparam TProtOptions Protocol options struct used for the application,\n"
        "///     like @ref #^#DEFAULT_OPTIONS#$#.\n"
        "/// @headerfile #^#HEADERFILE#$#\n"
        "template <typename TProtOptions = #^#DEFAULT_OPTIONS#$#>\n"
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
        "    template <typename TId, typename TMsg, typename THandler>\n"
        "    static auto dispatch(TId id, std::size_t idx, TMsg& msg, THandler& handler) ->\n"
        "        decltype(#^#NS_SCOPE#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, idx, msg, handler))\n"
        "    {\n"
        "        return #^#NS_SCOPE#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, idx, msg, handler);\n"
        "    }\n\n"
        "    /// @brief Complementary dispatch function.\n"
        "    /// @details Same as other dispatch without @b TAllMessages template parameter,\n"
        "    ///     used by  @b comms::processAllWithDispatchViaDispatcher().\n"
        "    template <typename TAllMessages, typename TId, typename TMsg, typename THandler>\n"
        "    static auto dispatch(TId id, std::size_t idx, TMsg& msg, THandler& handler) ->\n"
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
        "    template <typename TId, typename TMsg, typename THandler>\n"
        "    static auto dispatch(TId id, TMsg& msg, THandler& handler) ->\n"
        "        decltype(#^#NS_SCOPE#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, msg, handler))\n"
        "    {\n"
        "        return #^#NS_SCOPE#$#::dispatch::dispatch#^#NAME#$#Message<TProtOptions>(id, msg, handler);\n"
        "    }\n\n"
        "    /// @brief Complementary dispatch function.\n"
        "    /// @details Same as other dispatch without @b TAllMessages template parameter,\n"
        "    ///     used by  @b comms::processAllWithDispatchViaDispatcher().\n"
        "    template <typename TAllMessages, typename TId, typename TMsg, typename THandler>\n"
        "    static auto dispatch(TId id, TMsg& msg, THandler& handler) ->\n"
        "        decltype(dispatch(id, msg, handler))\n"
        "    {\n"
        "        return dispatch(id, msg, handler);\n"
        "    }\n"
        "};\n\n"
        "/// @brief Message dispatcher class to be used with\n"
        "///     @b comms::processAllWithDispatchViaDispatcher() function (or similar).\n"
        "/// @details Same as #^#NAME#$#MsgDispatcher, but passing\n"
        "///     @ref #^#DEFAULT_OPTIONS#$# as template parameter.\n"
        "/// @note Defined in #^#HEADERFILE#$#\n"
        "using #^#NAME#$#MsgDispatcherDefaultOptions =\n"
        "    #^#NAME#$#MsgDispatcher<>;\n";

    util::ReplacementMap repl = {
        {"NAME", inputPrefix},
        {"NS_SCOPE", comms::scopeFor(m_parent, m_generator)},
        {"DEFAULT_OPTIONS", comms::scopeForOptions(strings::defaultOptionsStr(), m_generator)},
        {"HEADERFILE", comms::relHeaderForDispatch(getFileName(inputPrefix), m_generator, m_parent)},
    };
    return util::processTemplate(Templ, repl);
}


} // namespace commsdsl2comms
