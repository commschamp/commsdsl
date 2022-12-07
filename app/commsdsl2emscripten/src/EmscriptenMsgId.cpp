//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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


bool EmscriptenMsgId::emscriptenWrite(EmscriptenGenerator& generator)
{
    auto& thisSchema = generator.currentSchema();
    if ((!generator.isCurrentProtocolSchema()) && (!thisSchema.hasReferencedMessageIdField()) && (!thisSchema.hasAnyReferencedMessage())) {
        return true;
    }

    EmscriptenMsgId obj(generator);
    return obj.emscriptenWriteSrcInternal();
}

bool EmscriptenMsgId::emscriptenWriteSrcInternal() const
{
    auto name = m_generator.emscriptenScopeNameForRoot(strings::msgIdEnumNameStr());
    auto filePath = m_generator.emscriptenOutputCodeSrcPathForRoot(strings::msgIdEnumNameStr());
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
        "    enum_<#^#SCOPE#$#>(\"#^#NAME#$#\")\n"
        "        #^#VALUES#$#\n"
        "        ;\n"
        "}\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"NAME", name},
        {"SCOPE", comms::scopeForRoot(strings::msgIdEnumNameStr(), m_generator)},
        {"VALUES", emscriptenIdsInternal()},
        {"HEADER", comms::relHeaderForRoot(strings::msgIdEnumNameStr(), m_generator)},
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}


std::string EmscriptenMsgId::emscriptenIdsInternal() const
{
    auto* msgIdField = m_generator.currentSchema().getMessageIdField();
    if (msgIdField != nullptr) {
        assert(msgIdField->dslObj().kind() == commsdsl::parse::Field::Kind::Enum);
        auto* castedMsgIdField = EmscriptenEnumField::cast(msgIdField);
        return castedMsgIdField->emscriptenBindValues();
    }

    auto allMessages = m_generator.getAllMessagesIdSorted();
    util::StringsList ids;
    ids.reserve(allMessages.size());

    static const std::string Templ = 
        ".value(\"#^#MSG#$#\", #^#SCOPE#$#_#^#MSG#$#)";
            
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeForRoot(strings::msgIdEnumNameStr(), m_generator)}   
    };

    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        repl["MSG"] = comms::fullNameFor(*m);
        ids.push_back(util::processTemplate(Templ, repl));
    }
    return util::strListToString(ids, ",\n", "");
}


} // namespace commsdsl2emscripten