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

#include "EmscriptenMsgId.h"

#include "EmscriptenGenerator.h"
#include "EmscriptenEnumField.h"
#include "EmscriptenNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <limits>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

EmscriptenMsgId::EmscriptenMsgId(EmscriptenGenerator& generator, const EmscriptenNamespace& parent) :
    m_generator(generator),
    m_parent(parent)
{
}

bool EmscriptenMsgId::emscriptenWrite() const
{
    auto& thisSchema = m_generator.genCurrentSchema();
    if ((!m_generator.genIsCurrentProtocolSchema()) && (!thisSchema.genHasReferencedMessageIdField()) && (!thisSchema.genHasAnyReferencedMessage())) {
        return true;
    }

    auto name = m_generator.emscriptenScopeNameForNamespaceMember(strings::genMsgIdEnumNameStr(), m_parent);
    auto filePath = m_generator.emscriptenAbsSourceForNamespaceMember(name, m_parent);
    m_generator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n" 
        "#include <emscripten/bind.h>\n\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "EMSCRIPTEN_BINDINGS(#^#NAME#$#) {\n"
        "    emscripten::enum_<#^#SCOPE#$#>(\"#^#NAME#$#\")\n"
        "        #^#VALUES#$#\n"
        "        ;\n"
        "}\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"NAME", name},
        {"SCOPE", comms::genScopeForMsgId(strings::genMsgIdEnumNameStr(), m_generator, m_parent)},
        {"VALUES", emscriptenIdsInternal()},
        {"HEADER", comms::genRelHeaderForMsgId(strings::genMsgIdEnumNameStr(), m_generator, m_parent)},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true; 
}

void EmscriptenMsgId::emscriptenAddSourceFiles(StringsList& sources) const
{
    auto name = m_generator.emscriptenScopeNameForNamespaceMember(strings::genMsgIdEnumNameStr(), m_parent);
    sources.push_back(m_generator.emscriptenRelSourceForNamespaceMember(name, m_parent));
}

std::string EmscriptenMsgId::emscriptenIdsInternal() const
{
    auto allMsgIdFields = m_parent.genFindMessageIdFields();
    if (allMsgIdFields.empty() && m_parent.genName().empty()) {
        allMsgIdFields = m_generator.genCurrentSchema().genGetAllMessageIdFields();
    }

    if (allMsgIdFields.size() == 1U) {  
        auto* msgIdField = allMsgIdFields.front();
        assert(msgIdField->genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Enum);
        auto* castedMsgIdField = EmscriptenEnumField::cast(msgIdField);
        return castedMsgIdField->emscriptenBindValues(&m_parent);        
    }

    auto allMessages = m_parent.genGetAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.genName().empty()) {
        allMessages = m_generator.genCurrentSchema().genGetAllMessagesIdSorted();
    }    
    util::GenStringsList ids;
    ids.reserve(allMessages.size());

    static const std::string Templ = 
        ".value(\"#^#MSG#$#\", #^#SCOPE#$#_#^#MSG#$#)";
            
    util::ReplacementMap repl = {
        {"SCOPE", comms::genScopeForMsgId(strings::genMsgIdEnumNameStr(), m_generator, m_parent)}   
    };

    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        repl["MSG"] = comms::genFullNameFor(*m);
        ids.push_back(util::genProcessTemplate(Templ, repl));
    }
    return util::genStrListToString(ids, "\n", "");
}


} // namespace commsdsl2emscripten