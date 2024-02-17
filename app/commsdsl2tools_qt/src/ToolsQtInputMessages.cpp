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

#include "ToolsQtInputMessages.h"

#include "ToolsQtGenerator.h"

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

namespace commsdsl2tools_qt
{

namespace 
{

using CheckFunction = std::function<bool (const commsdsl::gen::Message&)>;
bool writeFileInternal(
    const std::string& name,
    ToolsQtGenerator& generator,
    CheckFunction&& func)
{
    auto filePath = 
        generator.getOutputDir() + '/' + 
        generator.getTopNamespace() + '/' + 
        util::strReplace(comms::scopeForInput(name, generator), "::", "/") + 
        strings::cppHeaderSuffixStr();
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

    auto allMessages = generator.getAllMessagesIdSorted();
    util::StringsList includes = {
        "<tuple>",
    };

    util::StringsList scopes;

    std::string interfaceTempl;
    auto interfaces = generator.toolsGetSelectedInterfaces();
    if (1U < interfaces.size()) {
        interfaceTempl = "<TInterface>";
    }

    for (auto* m : allMessages) {
        assert(m != nullptr);
        if (!func(*m)) {
            continue;
        }

        auto scope = generator.getTopNamespace() + "::" + comms::scopeFor(*m, generator);
        includes.push_back(util::strReplace(scope, "::", "/") + strings::cppHeaderSuffixStr());
        scopes.push_back(scope + interfaceTempl);
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "namespace #^#TOP_NS#$#\n"
        "{\n\n"        
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace input\n"
        "{\n\n"
        "#^#TEMPLATE_PARAM#$#\n"
        "using #^#NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#MESSAGES#$#\n"
        "    >;\n\n"
        "} // namespace input\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"
        "} // namespace #^#TOP_NS#$#\n";

    comms::prepareIncludeStatement(includes);
    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::toolsFileGeneratedComment()},
        {"PROT_NAMESPACE", generator.protocolSchema().mainNamespace()},
        {"NAME", name},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"MESSAGES", util::strListToString(scopes, ",\n", "")},
        {"TOP_NS", generator.getTopNamespace()}
    };

    if (!interfaceTempl.empty()) {
        repl["TEMPLATE_PARAM"] = "template <typename TInterface>";
    }

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

} // namespace 
    

bool ToolsQtInputMessages::write(ToolsQtGenerator& generator)
{
    ToolsQtInputMessages obj(generator);
    return obj.testWriteInternal();
}

std::string ToolsQtInputMessages::toolsRelHeaderPath(const ToolsQtGenerator& generator)
{
    return
        generator.getTopNamespace() + '/' + 
        util::strReplace(comms::scopeForInput(strings::allMessagesStr(), generator), "::", "/") + 
        strings::cppHeaderSuffixStr();

}

bool ToolsQtInputMessages::testWriteInternal() const
{
    return
        toolsWriteAllMessagesInternal() &&
        toolsWritePlatformInputMessagesInternal() /* &&
        toolsWriteExtraInputMessagesInternal() */;
}

bool ToolsQtInputMessages::toolsWriteAllMessagesInternal() const
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

bool ToolsQtInputMessages::toolsWritePlatformInputMessagesInternal() const
{
    auto& platforms = m_generator.protocolSchema().platformNames();
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
    }

    return true;
}

// bool ToolsQtInputMessages::toolsWriteExtraInputMessagesInternal() const
// {
//     auto& extraBundles = m_generator.commsExtraMessageBundles();
//     for (auto& b : extraBundles) {

//         auto bundleCheckFunc = 
//             [&b](const commsdsl::gen::Message& msg)
//             {
//                 return std::find(b.second.begin(), b.second.end(), &msg) != b.second.end();
//             };

//         auto allCheckFunc = 
//             [&bundleCheckFunc](const commsdsl::gen::Message& msg)
//             {
//                 return bundleCheckFunc(msg);
//             };

//         if (!writeFileInternal(comms::className(b.first) + "Messages", m_generator, allCheckFunc)) {
//             return false;
//         }
//     };        

//     return true;
// }


} // namespace commsdsl2tools_qt