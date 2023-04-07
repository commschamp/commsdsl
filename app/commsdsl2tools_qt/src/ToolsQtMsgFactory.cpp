//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtMsgFactory.h"

#include "ToolsQtGenerator.h"
#include "ToolsQtInputMessages.h"
#include "ToolsQtInterface.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

const std::string MsgFactoryName = "AllMessagesDynMemMsgFactory";

using MessagesAccessList = std::vector<const commsdsl::gen::Message*>;
using MessagesMap = std::map<std::uintmax_t, MessagesAccessList>;

std::string toolsCommonHeaderTypesInternal()
{
    return 
        "using MsgIdParamType = typename Message::MsgIdParamType;\n"
        "using MsgIdType = typename Message::MsgIdType;\n"
        "using MsgPtr = std::unique_ptr<Message>;\n"
        "using CreateFailureReason = comms::MsgFactoryCreateFailureReason;\n"
        "using GenericMessage = void;\n";
}

std::string toolsCommonHeaderFuncsInternal(bool hasUniqueIds)
{
    const std::string Templ =
        "MsgPtr createGenericMsg(MsgIdParamType id, unsigned idx = 0U) const\n"
        "{\n"
        "    static_cast<void>(id);\n"
        "    static_cast<void>(idx);\n"
        "    return MsgPtr();\n"
        "}\n\n"    
        "bool canAllocate() const\n"
        "{\n"
        "    return true;\n"
        "}\n\n"
        "static constexpr bool hasUniqueIds()\n"
        "{\n"
        "    return #^#HAS_UNIQUE_IDS#$#;\n"
        "}\n\n"
        "static constexpr bool isDispatchPolymorphic()\n"
        "{\n"
        "    return false;\n"
        "}\n\n"
        "static constexpr bool isDispatchStaticBinSearch()\n"
        "{\n"
        "    return false;\n"
        "}\n\n"
        "static constexpr bool isDispatchLinearSwitch()\n"
        "{\n"
        "    return false;\n"
        "}\n\n"
        "static constexpr bool hasInPlaceAllocation()\n"
        "{\n"
        "    return false;\n"
        "}\n\n"
        "static constexpr bool hasGenericMessageSupport()\n"
        "{\n"
        "    return false;\n"
        "}\n\n"
        "static constexpr bool hasForcedDispatch()\n"
        "{\n"
        "    return true;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"HAS_UNIQUE_IDS", util::boolToString(hasUniqueIds)}
    };

    return util::processTemplate(Templ, repl);
}

MessagesMap toolsCreateMessagesMapInternal(const ToolsQtGenerator& generator)
{
    MessagesMap mappedMessages;

    auto allMessages = generator.getAllMessagesIdSorted();
    for (auto* m : allMessages) {
        mappedMessages[m->dslObj().id()].push_back(m);
    }  

    return mappedMessages;
}

bool toolsCheckUniqueIdsInternal(const MessagesMap& mappedMessages)
{
    return 
        std::all_of(
            mappedMessages.begin(), mappedMessages.end(),
            [](auto& elem)
            {
                return elem.second.size() <= 1U;
            });       
}

std::string toolsDynMemAllocCodeFuncInternal(
    const commsdsl::gen::Message& msg, 
    const ToolsQtGenerator& generator, 
    int idx,
    bool hasMultipleInterfaces)
{
    if (idx < 0) {
        static const std::string Templ = 
            "return MsgPtr(new #^#MSG_TYPE#$##^#TEMPL#$#);";

        util::ReplacementMap repl = {
            {"MSG_TYPE", generator.getTopNamespace() + "::" + comms::scopeFor(msg, generator)},
        };  

        if (hasMultipleInterfaces) {
            repl["TEMPL"] = "<TInterface>";
        }          

        return util::processTemplate(Templ, repl);
    }

    static const std::string Templ = 
        "if (idx == #^#IDX#$#) {\n"
        "    return MsgPtr(new #^#MSG_TYPE#$##^#TEMPL#$#);\n"
        "}";

    util::ReplacementMap repl = {
        {"MSG_TYPE", comms::scopeFor(msg, generator)},
        {"IDX", util::numToString(static_cast<std::intmax_t>(idx))},
    };       

    if (hasMultipleInterfaces) {
        repl["TEMPL"] = "<TInterface>";
    }             

    return util::processTemplate(Templ, repl);
}

std::string toolsMsgAllocCodeInternal(
    const MessagesMap& map, 
    const ToolsQtGenerator& generator,
    bool hasUniqueIds,
    bool hasMultipleInterfaces)
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
        "switch (id) {\n"
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
                "case ::#^#ID#$#: #^#CODE#$#";

            util::ReplacementMap caseRepl = {
                {"ID", comms::messageIdStrFor(*elem.second.front(), generator)},
                {"CODE", toolsDynMemAllocCodeFuncInternal(*elem.second.front(), generator, -1, hasMultipleInterfaces)},
            };

            cases.push_back(util::processTemplate(CaseTempl, caseRepl));
            continue;
        }

        util::StringsList allocs;
        for (auto idx = 0U; idx < elem.second.size(); ++idx) {
            allocs.push_back(toolsDynMemAllocCodeFuncInternal(*elem.second[idx], generator, static_cast<int>(idx), hasMultipleInterfaces));
        }

        static const std::string CaseTempl = 
            "case ::#^#ID#$#: \n"
            "    #^#CODE#$#\n"
            "    break;\n"
            ;

        util::ReplacementMap caseRepl = {
            {"ID", comms::messageIdStrFor(*elem.second.front(), generator)},
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

std::string toolsMsgCountCodeInternal(const MessagesMap& map, const ToolsQtGenerator& generator)
{
    static const std::string Templ = 
        "switch (id)\n"
        "{\n"
        "    #^#CASES#$#\n"
        "    default: break;\n"
        "}\n\n"
        "return 0U;\n";

    util::StringsList cases;
    for (auto& elem : map) {
        assert(!elem.second.empty());

        static const std::string CaseTempl = 
            "case ::#^#ID#$#: return #^#SIZE#$#;";

        util::ReplacementMap caseRepl = {
            {"ID", comms::messageIdStrFor(*elem.second.front(), generator)},
            {"SIZE", util::numToString(elem.second.size())},
        };

        cases.push_back(util::processTemplate(CaseTempl, caseRepl));
    }

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace 
    

bool ToolsQtMsgFactory::write(ToolsQtGenerator& generator)
{
    ToolsQtMsgFactory obj(generator);
    return obj.toolsWriteInternal();
}

std::string ToolsQtMsgFactory::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    return 
        generator.getTopNamespace() + '/' + 
        util::strReplace(comms::scopeForFactory(MsgFactoryName, generator), "::", "/") + 
        strings::cppHeaderSuffixStr();
}

ToolsQtMsgFactory::StringsList ToolsQtMsgFactory::toolsSourceFiles(const ToolsQtGenerator& generator)
{
    StringsList result;
    auto thisObj = ToolsQtMsgFactory(const_cast<ToolsQtGenerator&>(generator));
    if (thisObj.toolsIsGeneratedInternal() && thisObj.toolsHasSourceInternal()) {
        result.push_back(
            generator.getTopNamespace() + '/' + 
            util::strReplace(comms::scopeForFactory(MsgFactoryName, generator), "::", "/") + 
            strings::cppSourceSuffixStr());
    };

    return result;
}

std::string ToolsQtMsgFactory::toolsClassScope(const ToolsQtGenerator& generator)
{
    return generator.getTopNamespace() + "::" + comms::scopeForFactory(MsgFactoryName, generator);
}

bool ToolsQtMsgFactory::toolsWriteInternal() const
{
    if (!toolsIsGeneratedInternal()) {
        return true;
    }

    return
        toolsWriteHeaderInternal() &&
        toolsWriteSourceInternal();
}

bool ToolsQtMsgFactory::toolsWriteHeaderInternal() const
{
    auto filePath = m_generator.getOutputDir() + '/' + toolsRelHeaderPath(m_generator);
    auto& logger = m_generator.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }        

    util::StringsList includes = {
        "<memory>",
        "comms/MsgFactoryCreateFailureReason.h",
    };

    auto interfaces = m_generator.toolsGetSelectedInterfaces();
    if (1U == interfaces.size()) {
        includes.push_back(ToolsQtInterface::cast(interfaces.front())->toolsHeaderFilePath());
    }
    else {
        includes.push_back(ToolsQtInputMessages::toolsRelHeaderPath(m_generator));        
    }

    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"        
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace factory\n"
        "{\n\n"
        "#^#DEF#$#\n\n"
        "} // namespace factory\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n";
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"PROT_NAMESPACE", m_generator.protocolSchema().mainNamespace()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"DEF", toolsHeaderCodeInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool ToolsQtMsgFactory::toolsWriteSourceInternal() const
{
    if (m_generator.toolsHasMulitpleInterfaces()) {
        return true;
    }

    auto filePath = 
        m_generator.getOutputDir() + '/' + 
        m_generator.getTopNamespace() + '/' + 
        util::strReplace(comms::scopeForFactory(MsgFactoryName, m_generator), "::", "/") + 
        strings::cppSourceSuffixStr();

    auto& logger = m_generator.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }        

    util::StringsList includes = {
        toolsRelHeaderPath(m_generator),
        comms::relHeaderForRoot(strings::msgIdEnumNameStr(), m_generator),
        ToolsQtInputMessages::toolsRelHeaderPath(m_generator)
    };

    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"        
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace factory\n"
        "{\n\n"
        "#^#CODE#$#\n\n"
        "} // namespace factory\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n";
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"PROT_NAMESPACE", m_generator.protocolSchema().mainNamespace()},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"CODE", toolsSourceCodeInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

bool ToolsQtMsgFactory::toolsHasUniqueIdsInternal() const
{
    auto mappedMessages = toolsCreateMessagesMapInternal(m_generator);
    return toolsCheckUniqueIdsInternal(mappedMessages);
}

bool ToolsQtMsgFactory::toolsIsGeneratedInternal() const
{
    return m_generator.isCurrentProtocolSchema() || m_generator.currentSchema().hasAnyReferencedMessage();
}

bool ToolsQtMsgFactory::toolsHasSourceInternal() const
{
    return !m_generator.toolsHasMulitpleInterfaces();
}

std::string ToolsQtMsgFactory::toolsHeaderCodeInternal() const
{
    if (m_generator.toolsHasMulitpleInterfaces()) {
        return toolsHeaderMultipleInterfacesCodeInternal();
    }

    return toolsHeaderSingleInterfaceCodeInternal();
}

std::string ToolsQtMsgFactory::toolsHeaderSingleInterfaceCodeInternal() const
{
    const std::string Templ = 
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    using Message = #^#INTERFACE#$#;\n"
        "    #^#COMMON_TYPES#$#\n"
        "    MsgPtr createMsg(MsgIdParamType id, unsigned idx = 0U, CreateFailureReason* reason = nullptr) const;\n"
        "    std::size_t msgCount(MsgIdParamType id) const;\n\n"
        "    #^#COMMON_FUNCS#$#\n"
        "};\n";    

    auto interfaces = m_generator.toolsGetSelectedInterfaces();
    assert(!interfaces.empty());

    util::ReplacementMap repl = {
        {"INTERFACE", m_generator.getTopNamespace() + "::" + comms::scopeFor(*interfaces.front(), m_generator)},
        {"CLASS_NAME", MsgFactoryName},
        {"COMMON_TYPES", toolsCommonHeaderTypesInternal()},
        {"COMMON_FUNCS", toolsCommonHeaderFuncsInternal(toolsHasUniqueIdsInternal())},
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtMsgFactory::toolsHeaderMultipleInterfacesCodeInternal() const
{
    const std::string Templ = 
        "template <typename TInterface>\n"
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    using Message = TInterface;\n"
        "    #^#COMMON_TYPES#$#\n"
        "    MsgPtr createMsg(MsgIdParamType id, unsigned idx = 0U, CreateFailureReason* reason = nullptr) const\n"
        "    {\n"
        "        #^#ALLOC_CODE#$#\n"
        "    }\n\n"
        "    std::size_t msgCount(MsgIdParamType id) const\n"
        "    {\n"
        "        #^#COUNT_CODE#$#\n"
        "    }\n\n"        
        "    #^#COMMON_FUNCS#$#\n"
        "};\n"; 

    auto mappedMessages = toolsCreateMessagesMapInternal(m_generator);
    bool hasUniqueIds = toolsHasUniqueIdsInternal();
    util::ReplacementMap repl = {
        {"CLASS_NAME", MsgFactoryName},
        {"COMMON_TYPES", toolsCommonHeaderTypesInternal()},
        {"COMMON_FUNCS", toolsCommonHeaderFuncsInternal(hasUniqueIds)},
        {"ALLOC_CODE", toolsMsgAllocCodeInternal(mappedMessages, m_generator, hasUniqueIds, true)},
        {"COUNT_CODE", toolsMsgCountCodeInternal(mappedMessages, m_generator)}

    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtMsgFactory::toolsSourceCodeInternal() const
{
    auto mappedMessages = toolsCreateMessagesMapInternal(m_generator);
    bool hasUniqueIds = toolsCheckUniqueIdsInternal(mappedMessages);
    assert(!m_generator.toolsHasMulitpleInterfaces());

    static const std::string Templ = 
        "#^#CLASS_NAME#$#::MsgPtr #^#CLASS_NAME#$#::createMsg(MsgIdParamType id, unsigned idx, CreateFailureReason* reason) const\n"
        "{\n"
        "    #^#ALLOC_CODE#$#\n"
        "}\n\n"
        "std::size_t #^#CLASS_NAME#$#::msgCount(MsgIdParamType id) const\n"
        "{\n"
        "    #^#COUNT_CODE#$#\n"
        "}\n"
        ;    

    util::ReplacementMap repl = {
        {"CLASS_NAME", MsgFactoryName},
        {"ALLOC_CODE", toolsMsgAllocCodeInternal(mappedMessages, m_generator, hasUniqueIds, false)},
        {"COUNT_CODE", toolsMsgCountCodeInternal(mappedMessages, m_generator)}
    };
    
    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt
