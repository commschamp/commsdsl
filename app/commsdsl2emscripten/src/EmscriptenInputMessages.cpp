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

#include "EmscriptenInputMessages.h"

#include "EmscriptenGenerator.h"
#include "EmscriptenMessage.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

namespace 
{

const std::string FwdSuffix("Fwd");

} // namespace 

EmscriptenInputMessages::EmscriptenInputMessages(EmscriptenGenerator& generator, const EmscriptenNamespace& parent) :
    m_generator(generator),
    m_parent(parent)
{
}

bool EmscriptenInputMessages::emscriptenWrite() const
{
    return 
        emscriptenWriteHeaderInternal() && 
        emscriptenWriteHeaderFwdInternal();
}

std::string EmscriptenInputMessages::emscriptenClassName() const
{
    return m_generator.emscriptenScopeNameForNamespaceMember(strings::genAllMessagesStr(), m_parent);
}

std::string EmscriptenInputMessages::emscriptenRelHeader() const
{
    return m_generator.emscriptenProtocolRelHeaderForNamespaceMember(strings::genAllMessagesStr(), m_parent);
}

std::string EmscriptenInputMessages::emscriptenRelFwdHeader() const
{
    return m_generator.emscriptenProtocolRelHeaderForNamespaceMember(strings::genAllMessagesStr() + FwdSuffix, m_parent);
}

bool EmscriptenInputMessages::emscriptenWriteHeaderInternal() const
{
    auto filePath = m_generator.emscriptenAbsHeaderForNamespaceMember(strings::genAllMessagesStr(), m_parent);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    m_generator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    util::GenStringsList includes = {
        "<tuple>"
    };
    util::GenStringsList msgs;

    auto allMessages = m_generator.genGetAllMessagesIdSorted();
    includes.reserve(includes.size() + allMessages.size());
    msgs.reserve(allMessages.size());

    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        auto* castMsg = EmscriptenMessage::cast(m);
        includes.push_back(castMsg->emscriptenRelHeader());
        msgs.push_back(m_generator.emscriptenClassName(*castMsg));
    }

    comms::genPrepareIncludeStatement(includes);

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#^#INCLUDES#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#MSGS#$#\n"
        "    >;\n"
        ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"CLASS_NAME", emscriptenClassName()},
        {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
        {"MSGS", util::genStrListToString(msgs, ",\n", "")},
    };

    auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool EmscriptenInputMessages::emscriptenWriteHeaderFwdInternal() const
{
    auto filePath = m_generator.emscriptenAbsHeaderForNamespaceMember(strings::genAllMessagesStr() + FwdSuffix, m_parent);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    m_generator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    util::GenStringsList msgs;

    auto allMessages = m_generator.genGetAllMessagesIdSorted();
    msgs.reserve(allMessages.size());

    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        auto* castMsg = EmscriptenMessage::cast(m);
        msgs.push_back("class " + m_generator.emscriptenClassName(*castMsg) + ";");
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#^#MSGS#$#\n"
        ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"MSGS", util::genStrListToString(msgs, "\n", "\n")},
    };

    auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}


} // namespace commsdsl2emscripten
