//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsMsgFactory.h"

#include "CommsGenerator.h"
#include "CommsNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <fstream>
#include <map>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

const std::string ClientPrefixStr = "ClientInputMessages";    
const std::string ServerPrefixStr = "ServerInputMessages";
const std::string DynMemStr = "DynMem";
const std::string InPlaceStr = "InPlace";
const std::string DynMemAllocPolicyStr("dynamic");
const std::string InPlacePolicyStr("in place");
const std::string AllMessagesDesc("all the");
const std::string ClientDesc("the client input");
const std::string ServerDesc("the server input");

using MessagesAccessList = std::vector<const commsdsl::gen::Message*>;
using MessagesMap = std::map<std::uintmax_t, MessagesAccessList>;

using CheckFunction = std::function<bool (const commsdsl::gen::Message&)>;
using CodeFunction = std::function<std::string (const commsdsl::gen::Message&, const CommsGenerator&, int)>;

std::string commsDynMemAllocCodeFuncInternal(const commsdsl::gen::Message& msg, const CommsGenerator& generator, int idx)
{
    if (idx < 0) {
        static const std::string Templ = 
            "return MsgPtr(new #^#MSG_TYPE#$#<TInterface, TProtOptions>);";

        util::ReplacementMap repl = {
            {"MSG_TYPE", comms::scopeFor(msg, generator)},
        };            

        return util::processTemplate(Templ, repl);
    }

    static const std::string Templ = 
        "if (idx == #^#IDX#$#) {\n"
        "    return MsgPtr(new #^#MSG_TYPE#$#<TInterface, TProtOptions>);\n"
        "}";

    util::ReplacementMap repl = {
        {"MSG_TYPE", comms::scopeFor(msg, generator)},
        {"IDX", util::numToString(static_cast<std::intmax_t>(idx))},
    };            

    return util::processTemplate(Templ, repl);
}

std::string commsInPlaceAllocCodeFuncInternal(
    [[maybe_unused]] const commsdsl::gen::Message& msg, 
    [[maybe_unused]] const CommsGenerator& generator, 
    [[maybe_unused]] int idx)
{
    assert(false); // Not implemented
    return std::string();
}

std::string commsGetMsgAllocCodeInternal(
    const MessagesMap& map, 
    const CommsGenerator& generator,
    CodeFunction&& func,
    bool hasUniqueIds)
{
    static const std::string Templ = 
        "auto updateReasonFunc =\n"
        "    [reason](CreateFailureReason val)\n"
        "    {\n"
        "        if (reason != nullptr) {\n"
        "            *reason = val;\n"
        "        }\n"
        "    };\n\n"
        "#^#CHECK_IDX#$#\n"
        "updateReasonFunc(CreateFailureReason::None);\n"
        "switch (static_cast<std::intmax_t>(id)) {\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "updateReasonFunc(CreateFailureReason::InvalidId);\n"
        "return MsgPtr();\n";

    util::StringsList cases;
    for (auto& elem : map) {
        assert(!elem.second.empty());

        if (hasUniqueIds) {
            assert(elem.second.size() == 1U);
            static const std::string CaseTempl = 
                "case #^#ID#$#: #^#CODE#$#";

            util::ReplacementMap caseRepl = {
                {"ID", util::numToStringWithHexComment(elem.first)},
                {"CODE", func(*elem.second.front(), generator, -1)},
            };

            cases.push_back(util::processTemplate(CaseTempl, caseRepl));
            continue;
        }

        util::StringsList allocs;
        for (auto idx = 0U; idx < elem.second.size(); ++idx) {
            allocs.push_back(func(*elem.second[idx], generator, static_cast<int>(idx)));
        }

        static const std::string CaseTempl = 
            "case #^#ID#$#: \n"
            "    #^#CODE#$#\n"
            "    break;\n"
            ;

        util::ReplacementMap caseRepl = {
            {"ID", util::numToStringWithHexComment(elem.first)},
            {"CODE", util::strListToString(allocs, "\n", "")},
        };

        cases.push_back(util::processTemplate(CaseTempl, caseRepl));
    }        

    util::ReplacementMap repl {
        {"CASES", util::strListToString(cases, "\n", "")},
    };

    if (hasUniqueIds) {
        repl["CHECK_IDX"] = 
            "if (1U <= idx) {\n"
            "    updateReasonFunc(CreateFailureReason::InvalidId);\n"
            "    return MsgPtr();\n"
            "}\n";
    }

    return util::processTemplate(Templ, repl);
}

std::string commsGetMsgCountCodeInternal(const MessagesMap& map)
{
    static const std::string Templ = 
        "switch (static_cast<std::intmax_t>(id))\n"
        "{\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return 0U;\n";

    util::StringsList cases;
    for (auto& elem : map) {
        assert(!elem.second.empty());

        static const std::string CaseTempl = 
            "case #^#ID#$#: return #^#SIZE#$#;";

        util::ReplacementMap caseRepl = {
            {"ID", util::numToStringWithHexComment(elem.first)},
            {"SIZE", util::numToString(elem.second.size())},
        };

        cases.push_back(util::processTemplate(CaseTempl, caseRepl));
    }

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

bool commsWriteFileInternal(
    const std::string& prefix,
    const std::string& desc,
    const CommsGenerator& generator,
    const CommsNamespace& parent,
    CheckFunction&& checkFunc,
    bool inPlaceAlloc)
{
    auto* typeStr = &DynMemStr;
    auto* policyStr = &DynMemAllocPolicyStr;
    [[maybe_unused]] auto codeFunc = &commsDynMemAllocCodeFuncInternal;
    if (inPlaceAlloc) {
        typeStr = &InPlaceStr;
        policyStr = &InPlacePolicyStr;
        codeFunc = &commsInPlaceAllocCodeFuncInternal;
    }

    auto name = prefix + *typeStr + strings::msgFactorySuffixStr();
    auto filePath = comms::headerPathForFactory(name, generator, parent);
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

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains message factory with #^#POLICY#$# memory allocation for #^#DESC#$# messages.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "namespace #^#FACTORY_NAMESPACE#$#\n"
        "{\n\n"
        "/// @brief Message factory with #^#POLICY#$# memory allocation for #^#DESC#$# messages.\n"
        "/// @details Defines the same public interface as @b comms::MsgFactory and intended for\n"
        "///     its replacement.\n"
        "/// @tparam TInterface Interface class of the messages.\n"
        "/// @tparam TProtOptions Application specific protocol definition options.\n"
        "template<typename TInterface, typename TProtOptions>\n"
        "class #^#NAME#$##^#ORIG#$#\n"
        "{\n"
        "public:\n"
        "    /// @brief Type of the common base class of all the messages.\n"
        "    using Message = TInterface;\n\n"
        "    /// @brief Type of the message ID when passed as a parameter.\n"
        "    using MsgIdParamType = typename Message::MsgIdParamType;\n\n"
        "    /// @brief Type of the message ID.\n"
        "    using MsgIdType = typename Message::MsgIdType;\n\n"
        "    /// @brief Smart pointer to @ref Message which holds allocated message object.\n"
        "    using MsgPtr = std::unique_ptr<Message#^#DELETER_SUFFIX#$#>;\n\n"
        "    /// @brief Reason for message creation failure\n"
        "    using CreateFailureReason = comms::MsgFactoryCreateFailureReason;\n\n"
        "    /// @brief Type of generic message.\n"
        "    /// @details Not supported\n"
        "    using GenericMessage = void;\n\n"
        "    /// @brief Create message object given the ID of the message.\n"
        "    /// @param id ID of the message.\n"
        "    /// @param idx Relative index (or offset) of the message with the same ID.\n"
        "    /// @param[out] reason Failure reason in case creation has failed.\n"
        "    MsgPtr createMsg(MsgIdParamType id, unsigned idx = 0U, CreateFailureReason* reason = nullptr) const\n"
        "    {\n"
        "        #^#CREATE_CODE#$#\n"
        "    }\n\n"
        "    /// @brief Allocate and initialise @b comms::GenericMessage object.\n"
        "    MsgPtr createGenericMsg(MsgIdParamType id, unsigned idx = 0U) const\n"
        "    {\n"
        "        static_cast<void>(id);\n"
        "        static_cast<void>(idx);\n"
        "        return MsgPtr();\n"
        "    }\n\n"
        "    /// @brief Inquiry whether allocation is possible\n"
        "    bool canAllocate() const\n"
        "    {\n"
        "        return #^#CAN_ALLOCATE#$#;\n"
        "    }\n\n"
        "    /// @brief Get number of message types that have the specified ID.\n"
        "    /// @param id ID of the message.\n"
        "    /// @return Number of message classes that report same ID.\n"
        "    std::size_t msgCount(MsgIdParamType id) const\n"
        "    {\n"
        "        #^#MSG_COUNT_CODE#$#\n"
        "    }\n\n"
        "    /// @brief Compile time inquiry whether all the message classes have unique IDs.\n"
        "    static constexpr bool hasUniqueIds()\n"
        "    {\n"
        "        return #^#HAS_UNIQUE_IDS#$#;\n"
        "    }\n\n"
        "    /// @brief Compile time inquiry whether polymorphic dispatch tables are\n"
        "    ///     generated internally to map message ID to actual type.\n"
        "    static constexpr bool isDispatchPolymorphic()\n"
        "    {\n"
        "        return false;\n"
        "    }\n\n"
        "    /// @brief Compile time inquiry whether static binary search dispatch is\n"
        "    ///     generated internally to map message ID to actual type.\n"
        "    static constexpr bool isDispatchStaticBinSearch()\n"
        "    {\n"
        "        return false;\n"
        "    }\n\n"
        "    /// @brief Compile time inquiry whether linear switch dispatch is\n"
        "    ///     generated internally to map message ID to actual type.\n"
        "    static constexpr bool isDispatchLinearSwitch()\n"
        "    {\n"
        "        return false;\n"
        "    }\n\n"
        "    /// @brief Compile time inquiry whether factory supports in-place allocation\n"
        "    static constexpr bool hasInPlaceAllocation()\n"
        "    {\n"
        "        return #^#IN_PLACE_ALLOC#$#;\n"
        "    }\n\n"
        "    /// @brief Compile time inquiry whether factory supports @b comms::GenericMessage allocation.\n"
        "    static constexpr bool hasGenericMessageSupport()\n"
        "    {\n"
        "        return false;\n"
        "    }\n\n"
        "    /// @brief Compile time inquiry whether factory has forced dispatch method\n"
        "    static constexpr bool hasForcedDispatch()\n"
        "    {\n"
        "        return true;\n"
        "    }\n"    
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "} // namespace #^#FACTORY_NAMESPACE#$#\n\n"
        "#^#NS_END#$#\n";

    util::StringsList includes = {
        "<cstdint>",
        "<memory>",
        "comms/MsgFactoryCreateFailureReason.h",
        comms::relHeaderForInput(prefix, generator, parent),
        
    };

    comms::prepareIncludeStatement(includes);

    auto allMessages = parent.getAllMessagesIdSorted();

    MessagesMap mappedMessages;

    for (auto* m : allMessages) {
        if (!checkFunc(*m)) {
            continue;
        }

        mappedMessages[m->dslObj().id()].push_back(m);
    }    

    bool hasUniqueIds = 
        std::all_of(
            mappedMessages.begin(), mappedMessages.end(),
            [](auto& elem)
            {
                return elem.second.size() <= 1U;
            });

        util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"NS_BEGIN", comms::namespaceBeginFor(parent, generator)},
        {"NS_END", comms::namespaceEndFor(parent, generator)},         
        {"FACTORY_NAMESPACE", strings::factoryNamespaceStr()},
        {"NAME", name},
        {"POLICY", *policyStr},
        {"DESC", desc},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"EXTEND", util::readFileContents(comms::inputCodePathForFactory(name, generator, parent) + strings::extendFileSuffixStr())},
        {"APPEND", util::readFileContents(comms::inputCodePathForFactory(name, generator, parent) + strings::appendFileSuffixStr())},
        {"HAS_UNIQUE_IDS", util::boolToString(hasUniqueIds)},
        {"IN_PLACE_ALLOC", util::boolToString(inPlaceAlloc)},
        {"CAN_ALLOCATE", "true"},
        {"MSG_COUNT_CODE", commsGetMsgCountCodeInternal(mappedMessages)},
        {"CREATE_CODE", commsGetMsgAllocCodeInternal(mappedMessages, generator, codeFunc, hasUniqueIds)},
    };

    if (!repl["EXTEND"].empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    if (inPlaceAlloc) {
        // TODO: impelement
        assert(false); // Not implemented
    }

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

} // namespace 
    

CommsMsgFactory::CommsMsgFactory(CommsGenerator& generator, const CommsNamespace& parent) : 
    m_generator(generator),
    m_parent(parent)
{
}

bool CommsMsgFactory::commsWrite() const
{
    return
        commsWriteAllMsgFactoryInternal() &&
        commsWriteClientMsgFactoryInternal() &&
        commsWriteServerMsgFactoryInternal() &&
        commsWritePlatformMsgFactoryInternal() &&
        commsWriteExtraMsgFactoryInternal();
}

std::string CommsMsgFactory::commsScope(const std::string& prefix) const
{
    return comms::scopeForFactory(prefix + strings::msgFactorySuffixStr(), m_generator, m_parent);
}

std::string CommsMsgFactory::commsRelHeaderPath(const std::string& prefix) const
{
    return comms::relHeaderForFactory(prefix + strings::msgFactorySuffixStr(), m_generator, m_parent);
}

bool CommsMsgFactory::commsWriteAllMsgFactoryInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg) noexcept
        {
            static_cast<void>(msg);
            return true;
        };

    auto dynMemWrite = 
        commsWriteFileInternal(
            strings::allMessagesStr(),
            AllMessagesDesc,
            m_generator,
            m_parent,
            checkFunc,
            false);   

    return dynMemWrite;
}

bool CommsMsgFactory::commsWriteClientMsgFactoryInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg)
        {
            return msg.dslObj().sender() != commsdsl::parse::ParseMessage::Sender::Client;
        };

    auto dynMemWrite = 
        commsWriteFileInternal(
            ClientPrefixStr,
            ClientDesc,
            m_generator,
            m_parent,
            checkFunc,
            false);   

    return dynMemWrite;        
}

bool CommsMsgFactory::commsWriteServerMsgFactoryInternal() const
{
    auto checkFunc = 
        [](const commsdsl::gen::Message& msg)
        {
            return msg.dslObj().sender() != commsdsl::parse::ParseMessage::Sender::Server;
        };

    auto dynMemWrite = 
        commsWriteFileInternal(
            ServerPrefixStr,
            ServerDesc,
            m_generator,
            m_parent,
            checkFunc,
            false);   

    return dynMemWrite;           
}

bool CommsMsgFactory::commsWritePlatformMsgFactoryInternal() const
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

        auto allDynMemWrite = 
            commsWriteFileInternal(
                comms::className(p) + "Messages",
                AllMessagesDesc + " \"" + p + "\" platform specific",
                m_generator,
                m_parent,
                allCheckFunc,
                false);  

        if (!allDynMemWrite) {
            return false;
        }

        auto clientCheckFunc = 
            [&platformCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    platformCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::ParseMessage::Sender::Client);
            };

        auto clientDynMemWrite = 
            commsWriteFileInternal(
                comms::className(p) + ClientPrefixStr,
                ClientDesc + " \"" + p + "\" platform specific",
                m_generator,
                m_parent,
                clientCheckFunc,
                false);  

        if (!clientDynMemWrite) {
            return false;
        }            

        auto serverCheckFunc = 
            [&platformCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    platformCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::ParseMessage::Sender::Server);
            };

        auto serverDynMemWrite = 
            commsWriteFileInternal(
                comms::className(p) + ServerPrefixStr,
                ServerDesc + " \"" + p + "\" platform specific",
                m_generator,
                m_parent,
                serverCheckFunc,
                false);  

        if (!serverDynMemWrite) {
            return false;
        }            
    };        

    return true;
}

bool CommsMsgFactory::commsWriteExtraMsgFactoryInternal() const
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

        auto allDynMemWrite = 
            commsWriteFileInternal(
                comms::className(b.first) + "Messages",
                AllMessagesDesc + " \"" + b.first+ "\" bundle specific",
                m_generator,
                m_parent,
                allCheckFunc,
                false);  

        if (!allDynMemWrite) {
            return false;
        }            

        auto clientCheckFunc = 
            [&bundleCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    bundleCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::ParseMessage::Sender::Client);
            };

        auto clientDynMemWrite = 
            commsWriteFileInternal(
                comms::className(b.first) + ClientPrefixStr,
                ClientDesc + " \"" + b.first+ "\" bundle specific",
                m_generator,
                m_parent,
                clientCheckFunc,
                false);  

        if (!clientDynMemWrite) {
            return false;
        }            

        auto serverCheckFunc = 
            [&bundleCheckFunc](const commsdsl::gen::Message& msg)
            {
                return 
                    bundleCheckFunc(msg) &&
                    (msg.dslObj().sender() != commsdsl::parse::ParseMessage::Sender::Server);
            };

        auto serverDynMemWrite = 
            commsWriteFileInternal(
                comms::className(b.first) + ServerPrefixStr,
                ServerDesc + " \"" + b.first + "\" bundle specific",
                m_generator,
                m_parent,
                serverCheckFunc,
                false);  

        if (!serverDynMemWrite) {
            return false;
        }              
    };        

    return true;
}


} // namespace commsdsl2comms