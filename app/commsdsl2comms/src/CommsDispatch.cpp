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

#include "commsdsl/gen/GenEnumField.h"
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
    auto filePath = comms::genHeaderPathForDispatch(name, generator, ns);
    generator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.genCreateDirectory(dirPath)) {
        return false;
    }      

    std::ofstream stream(filePath);
    if (!stream) {
        generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
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

util::ReplacementMap initialRepl(const CommsGenerator& generator, const commsdsl::gen::GenElem& elem)
{
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"DISPATCH_NAMESPACE", strings::genCispatchNamespaceStr()},
        {"NS_BEGIN", comms::genNamespaceBeginFor(elem, generator)},
        {"NS_END", comms::genNamespaceEndFor(elem, generator)},        
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
    // auto& thisSchema = static_cast<CommsSchema&>(m_generator.genCurrentSchema());
    // if ((!m_generator.genIsCurrentProtocolSchema()) && (!thisSchema.commsHasAnyMessage())) {
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
        [](const commsdsl::gen::GenMessage& msg) noexcept
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
    
    return writeFileInternal(getFileName(), m_generator, m_parent, util::genProcessTemplate(dispatchTempl(), repl, true));
}

bool CommsDispatch::commsWriteClientDispatchInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::GenMessage& msg) noexcept
        {
            return msg.genParseObj().parseSender() != commsdsl::parse::ParseMessage::Sender::Client;
        };

    util::ReplacementMap repl = initialRepl(m_generator, m_parent);
    std::string inputPrefix = "ClientInput";
    repl.insert({
        {"DESC", "client input"},
        {"INCLUDES", commsIncludesInternal(inputPrefix)},
        {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(checkFunc))}
    });

    return writeFileInternal(getFileName(inputPrefix), m_generator, m_parent, util::genProcessTemplate(dispatchTempl(), repl, true));
}

bool CommsDispatch::commsWriteServerDispatchInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::GenMessage& msg) noexcept
        {
            return msg.genParseObj().parseSender() != commsdsl::parse::ParseMessage::Sender::Server;
        };

    util::ReplacementMap repl = initialRepl(m_generator, m_parent);
    std::string inputPrefix = "ServerInput";
    repl.insert({
        {"DESC", "client input"},
        {"INCLUDES", commsIncludesInternal(inputPrefix)},
        {"CODE", commsDispatchCodeInternal(inputPrefix, std::move(checkFunc))}
    });

    return writeFileInternal(getFileName(inputPrefix), m_generator, m_parent, util::genProcessTemplate(dispatchTempl(), repl, true));
}

bool CommsDispatch::commsWritePlatformDispatchInternal() const
{
    auto& platforms = m_generator.genCurrentSchema().platformNames();
    for (auto& p : platforms) {

        auto platformCheckFunc = 
            [&p](const commsdsl::gen::GenMessage& msg)
            {
                auto& msgPlatforms = msg.genParseObj().parsePlatforms();
                if (msgPlatforms.empty()) {
                    return true;
                }

                return std::find(msgPlatforms.begin(), msgPlatforms.end(), p) != msgPlatforms.end();
            };

        do {
            auto allCheckFunc = 
                [&platformCheckFunc](const commsdsl::gen::GenMessage& msg)
                {
                    return platformCheckFunc(msg);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::genClassName(p);
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
                    util::genProcessTemplate(dispatchTempl(), repl, true));
                    
            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto clientCheckFunc = 
                [&platformCheckFunc](const commsdsl::gen::GenMessage& msg)
                {
                    return 
                        platformCheckFunc(msg) &&
                        (msg.genParseObj().parseSender() != commsdsl::parse::ParseMessage::Sender::Client);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::genClassName(p) + "ClientInput";
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
                    util::genProcessTemplate(dispatchTempl(), repl, true));

            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto serverCheckFunc = 
                [&platformCheckFunc](const commsdsl::gen::GenMessage& msg)
                {
                    return 
                        platformCheckFunc(msg) &&
                        (msg.genParseObj().parseSender() != commsdsl::parse::ParseMessage::Sender::Server);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::genClassName(p) + "ServerInput";
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
                    util::genProcessTemplate(dispatchTempl(), repl, true));

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
            [&b](const commsdsl::gen::GenMessage& msg)
            {
                return std::find(b.second.begin(), b.second.end(), &msg) != b.second.end();
            };

        do {
            auto allCheckFunc = 
                [&bundleCheckFunc](const commsdsl::gen::GenMessage& msg)
                {
                    return bundleCheckFunc(msg);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::genClassName(b.first);
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
                    util::genProcessTemplate(dispatchTempl(), repl, true));
                    
            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto clientCheckFunc = 
                [&bundleCheckFunc](const commsdsl::gen::GenMessage& msg)
                {
                    return 
                        bundleCheckFunc(msg) &&
                        (msg.genParseObj().parseSender() != commsdsl::parse::ParseMessage::Sender::Client);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::genClassName(b.first) + "ClientInput";
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
                    util::genProcessTemplate(dispatchTempl(), repl, true));

            if (!result) {
                return false;
            }
        } while (false);

        do {
            auto serverCheckFunc = 
                [&bundleCheckFunc](const commsdsl::gen::GenMessage& msg)
                {
                    return 
                        bundleCheckFunc(msg) &&
                        (msg.genParseObj().parseSender() != commsdsl::parse::ParseMessage::Sender::Server);
                };

            util::ReplacementMap repl = initialRepl(m_generator, m_parent);
            std::string inputPrefix = comms::genClassName(b.first) + "ServerInput";
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
                    util::genProcessTemplate(dispatchTempl(), repl, true));

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
        comms::genRelHeaderForInput(inputPrefix + "Messages", m_generator, m_parent),
        comms::genRelHeaderForOptions(strings::genDefaultOptionsClassStr(), m_generator),
    };

    comms::genPrepareIncludeStatement(incs);
    return util::genStrListToString(incs, "\n", "\n");
}

std::string CommsDispatch::commsDispatchCodeInternal(const std::string& name, CheckMsgFunc&& func) const
{
    MessagesMap map;
    auto allMessages = m_parent.genGetAllMessagesIdSorted();
    bool hasMultipleMessagesWithSameId = false;
    const commsdsl::gen::GenMessage* firstMsg = nullptr;
    const commsdsl::gen::GenMessage* secondMsg = nullptr;
    for (auto* m : allMessages) {
        assert(m != nullptr);
        if (!func(*m)) {
            continue;
        }
        auto& mList = map[m->genParseObj().parseId()];
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

    auto allInterfaces = m_generator.genGetAllInterfaces();
    // assert(!allInterfaces.empty());

    util::ReplacementMap repl = {
        {"NAME", name},
        {"DEFAULT_OPTIONS", comms::genScopeForOptions(strings::genDefaultOptionsClassStr(), m_generator)},
        {"HEADERFILE", comms::genRelHeaderForDispatch(getFileName(name), m_generator, m_parent)},
        {"INTERFACE", (!allInterfaces.empty()) ? comms::genScopeFor(*allInterfaces.front(), m_generator) : std::string("SomeInterface")},
        {"MSG1", firstMsg != nullptr ? comms::genScopeFor(*firstMsg, m_generator) : std::string("SomeMessage")},
        {"MSG2", secondMsg != nullptr ? comms::genScopeFor(*secondMsg, m_generator) : std::string("SomeOtherMessage")},
        {"MSG1_NAME", firstMsg != nullptr ? comms::genClassName(firstMsg->genParseObj().parseName()) : std::string("SomeMessage")},
        {"MSG2_NAME", secondMsg != nullptr ? comms::genClassName(secondMsg->genParseObj().parseName()) : std::string("SomeOtherMessage")},
        {"CASES", commsCasesCodeInternal(map)},
        {"DISPATCHER", commsMsgDispatcherCodeInternal(name)},
    };

    auto& templ = hasMultipleMessagesWithSameId ? multipleMessagesPerIdTempl() : singleMessagePerIdTempl();
    return util::genProcessTemplate(templ, repl);
}

std::string CommsDispatch::commsCasesCodeInternal(const MessagesMap& map) const
{
    util::StringsList cases;
    for (auto& elem : map) {
        auto& msgList = elem.second;
        assert(!msgList.empty());
        auto idStr = util::genNumToStringWithHexComment(elem.first);

        static const std::string MsgCaseTempl =
            "case #^#MSG_ID#$#:\n"
            "{\n"
            "    using MsgType = #^#MSG_TYPE#$#<InterfaceType, TProtOptions>;\n"
            "    return handler.handle(static_cast<MsgType&>(msg));\n"
            "}";

        if (msgList.size() == 1) {
            util::ReplacementMap repl = {
                {"MSG_ID", idStr},
                {"MSG_TYPE", comms::genScopeFor(*msgList.front(), m_generator)},
            };
            cases.push_back(util::genProcessTemplate(MsgCaseTempl, repl));
            continue;
        }

        util::StringsList offsetCases;
        for (auto idx = 0U; idx < msgList.size(); ++idx) {
            util::ReplacementMap repl = {
                {"MSG_ID", util::genNumToString(idx)},
                {"MSG_TYPE", comms::genScopeFor(*msgList[idx], m_generator)},
            };
            offsetCases.push_back(util::genProcessTemplate(MsgCaseTempl, repl));
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
            {"IDX_CASES", util::genStrListToString(offsetCases, "\n", "")},
        };

        cases.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(cases, "\n", "");
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
        {"NS_SCOPE", comms::genScopeFor(m_parent, m_generator)},
        {"DEFAULT_OPTIONS", comms::genScopeForOptions(strings::genDefaultOptionsStr(), m_generator)},
        {"HEADERFILE", comms::genRelHeaderForDispatch(getFileName(inputPrefix), m_generator, m_parent)},
    };
    return util::genProcessTemplate(Templ, repl);
}


} // namespace commsdsl2comms
