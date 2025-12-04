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
    m_emscriptenGenerator(generator),
    m_parent(parent)
{
}

bool EmscriptenMsgId::emscriptenWrite() const
{
    auto& thisSchema = m_emscriptenGenerator.genCurrentSchema();
    if ((!m_emscriptenGenerator.genIsCurrentProtocolSchema()) && (!thisSchema.genHasReferencedMessageIdField()) && (!thisSchema.genHasAnyReferencedMessage())) {
        return true;
    }

    auto name = m_emscriptenGenerator.emscriptenScopeNameForNamespaceMember(strings::genMsgIdEnumNameStr(), m_parent);
    auto filePath = m_emscriptenGenerator.emscriptenAbsSourceForNamespaceMember(name, m_parent);
    m_emscriptenGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_emscriptenGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_emscriptenGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
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

    util::GenReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::emscriptenFileGeneratedComment()},
        {"NAME", name},
        {"SCOPE", comms::genScopeForMsgId(strings::genMsgIdEnumNameStr(), m_emscriptenGenerator, m_parent)},
        {"VALUES", emscriptenIdsInternal()},
        {"HEADER", comms::genRelHeaderForMsgId(strings::genMsgIdEnumNameStr(), m_emscriptenGenerator, m_parent)},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_emscriptenGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

void EmscriptenMsgId::emscriptenAddSourceFiles(GenStringsList& sources) const
{
    auto name = m_emscriptenGenerator.emscriptenScopeNameForNamespaceMember(strings::genMsgIdEnumNameStr(), m_parent);
    sources.push_back(m_emscriptenGenerator.emscriptenRelSourceForNamespaceMember(name, m_parent));
}

std::string EmscriptenMsgId::emscriptenIdsInternal() const
{
    auto allMsgIdFields = m_parent.genFindMessageIdFields();
    if (allMsgIdFields.empty() && m_parent.genName().empty()) {
        allMsgIdFields = m_emscriptenGenerator.genCurrentSchema().genGetAllMessageIdFields();
    }

    if (allMsgIdFields.size() == 1U) {
        auto* msgIdField = allMsgIdFields.front();
        assert(msgIdField->genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Enum);
        auto* castedMsgIdField = EmscriptenEnumField::emscriptenCast(msgIdField);
        return castedMsgIdField->emscriptenBindValues(&m_parent);
    }

    auto allMessages = m_parent.genGetAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.genName().empty()) {
        allMessages = m_emscriptenGenerator.genCurrentSchema().genGetAllMessagesIdSorted();
    }
    util::GenStringsList ids;
    ids.reserve(allMessages.size());

    static const std::string Templ =
        ".value(\"#^#MSG#$#\", #^#SCOPE#$#_#^#MSG#$#)";

    util::GenReplacementMap repl = {
        {"SCOPE", comms::genScopeForMsgId(strings::genMsgIdEnumNameStr(), m_emscriptenGenerator, m_parent)}
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