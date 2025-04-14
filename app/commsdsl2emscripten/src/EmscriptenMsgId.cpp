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
    auto& thisSchema = m_generator.currentSchema();
    if ((!m_generator.isCurrentProtocolSchema()) && (!thisSchema.hasReferencedMessageIdField()) && (!thisSchema.hasAnyReferencedMessage())) {
        return true;
    }

    auto name = m_generator.emscriptenScopeNameForMsgId(strings::msgIdEnumNameStr(), m_parent);
    auto filePath = m_generator.emscriptenAbsSourceForMsgId(name, m_parent);
    m_generator.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
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
        {"SCOPE", comms::scopeForMsgId(strings::msgIdEnumNameStr(), m_generator, m_parent)},
        {"VALUES", emscriptenIdsInternal()},
        {"HEADER", comms::relHeaderForMsgId(strings::msgIdEnumNameStr(), m_generator, m_parent)},
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true; 
}

void EmscriptenMsgId::emscriptenAddSourceFiles(StringsList& sources) const
{
    auto name = m_generator.emscriptenScopeNameForMsgId(strings::msgIdEnumNameStr(), m_parent);
    sources.push_back(m_generator.emscriptenRelSourceForMsgId(name, m_parent));
}

std::string EmscriptenMsgId::emscriptenIdsInternal() const
{
    auto allMsgIdFields = m_generator.currentSchema().getAllMessageIdFields();
    if (allMsgIdFields.size() == 1U) {  
        auto* msgIdField = allMsgIdFields.front();
        assert(msgIdField->dslObj().kind() == commsdsl::parse::Field::Kind::Enum);
        auto* castedMsgIdField = EmscriptenEnumField::cast(msgIdField);
        return castedMsgIdField->emscriptenBindValues(&m_parent);        
    }

    auto allMessages = m_generator.getAllMessagesIdSorted();
    util::StringsList ids;
    ids.reserve(allMessages.size());

    static const std::string Templ = 
        ".value(\"#^#MSG#$#\", #^#SCOPE#$#_#^#MSG#$#)";
            
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeForMsgId(strings::msgIdEnumNameStr(), m_generator, m_parent)}   
    };

    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        repl["MSG"] = comms::fullNameFor(*m);
        ids.push_back(util::processTemplate(Templ, repl));
    }
    return util::strListToString(ids, "\n", "");
}


} // namespace commsdsl2emscripten