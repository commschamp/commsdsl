//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenProtocolOptions.h"

#include "EmscriptenGenerator.h"

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

namespace 
{

std::string emscriptenCodeInternal(EmscriptenGenerator& generator, std::size_t idx)
{
    assert(idx < generator.schemas().size());

    generator.chooseCurrentSchema(static_cast<unsigned>(idx));
    if (!generator.currentSchema().hasAnyReferencedComponent()) {
        if (idx == 0U) {
            return strings::emptyString();
        }

        return emscriptenCodeInternal(generator, idx - 1U);
    }

    auto scope = comms::scopeForOptions(strings::defaultOptionsClassStr(), generator);

    if (idx == 0U) {
        return scope;
    }

    auto nextScope = emscriptenCodeInternal(generator, idx - 1U);
    if (nextScope.empty()) {
        return scope;
    }

    static const std::string Templ = 
        "#^#SCOPE#$#T<\n"
        "    #^#NEXT#$#\n"
        ">";

    util::ReplacementMap repl = {
        {"SCOPE", std::move(scope)},
        {"NEXT", std::move(nextScope)}
    };
    
    return util::processTemplate(Templ, repl);
}

} // namespace 
  
std::string EmscriptenProtocolOptions::emscriptenClassName(const EmscriptenGenerator& generator)
{
    if (!emscriptenIsDefined(generator)) {
        return strings::emptyString();
    }

    return generator.protocolSchema().mainNamespace() + "_ProtocolOptions";
}

bool EmscriptenProtocolOptions::emscriptenIsDefined([[maybe_unused]] const EmscriptenGenerator& generator)
{
    // Always use message factory options.
    return true;
}

void EmscriptenProtocolOptions::emscriptenAddInclude(const EmscriptenGenerator& generator, StringsList& list)
{
    if (!emscriptenIsDefined(generator)) {
        return;
    }

    auto name = emscriptenClassName(generator);
    list.push_back(generator.emscriptenProtocolRelHeaderForRoot(name));
}

bool EmscriptenProtocolOptions::emscriptenWrite(EmscriptenGenerator& generator)
{
    if (!emscriptenIsDefined(generator)) {
        return true;
    }

    EmscriptenProtocolOptions obj(generator);
    return obj.emsciptenWriteHeaderInternal();
}

EmscriptenProtocolOptions::EmscriptenProtocolOptions(EmscriptenGenerator& generator) :
    m_generator(generator)
{
}    

bool EmscriptenProtocolOptions::emsciptenWriteHeaderInternal()
{
    auto name = emscriptenClassName(m_generator);
    auto filePath = m_generator.emscriptenAbsHeaderForRoot(name);
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
        "#^#INCLUDES#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"INCLUDES", emscriptenIncludesInternal()},
        {"DEF", emscriptenTypeDefInternal()}
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string EmscriptenProtocolOptions::emscriptenTypeDefInternal()
{
    assert(m_generator.isCurrentProtocolSchema());

    const std::string Templ = 
        "using #^#OPT_TYPE#$# =\n"
        "    #^#MSG_FACT_OPTS#$#T<\n"
        "        #^#CODE#$#\n"
        "    >;\n\n";

    auto msgFactOptions = comms::scopeForOptions(strings::allMessagesDynMemMsgFactoryDefaultOptionsClassStr(), m_generator);
    util::ReplacementMap repl = {
        {"OPT_TYPE", emscriptenClassName(m_generator)},
        {"CODE", emscriptenCodeInternal(m_generator, m_generator.schemas().size() - 1U)},
        {"MSG_FACT_OPTS", std::move(msgFactOptions)}
    };

    m_generator.chooseProtocolSchema();
    return util::processTemplate(Templ, repl);
}

std::string EmscriptenProtocolOptions::emscriptenIncludesInternal()
{
    assert(m_generator.isCurrentProtocolSchema());

    util::StringsList list;
    list.push_back(comms::relHeaderForOptions(strings::allMessagesDynMemMsgFactoryDefaultOptionsClassStr(), m_generator));
    auto& schemas = m_generator.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        m_generator.chooseCurrentSchema(idx);
        if (!m_generator.currentSchema().hasAnyReferencedComponent()) {
            continue;
        }

        list.push_back(comms::relHeaderForOptions(strings::defaultOptionsClassStr(), m_generator));
    }

    m_generator.chooseProtocolSchema();
    comms::prepareIncludeStatement(list);

    return util::strListToString(list, "\n", "\n");
}

} // namespace commsdsl2emscripten