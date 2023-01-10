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

#include "EmscriptenVersion.h"

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

bool EmscriptenVersion::emscriptenWrite(EmscriptenGenerator& generator)
{
    EmscriptenVersion obj(generator);
    return obj.emscriptenWriteSrcInternal();
}

void EmscriptenVersion::emscriptenAddSourceFiles(const EmscriptenGenerator& generator, StringsList& sources)
{
    for (auto idx = 0U; idx < generator.schemas().size(); ++idx) {
        sources.push_back(generator.emscriptenSchemaRelSourceForRoot(idx, strings::versionFileNameStr()));
    }
}

bool EmscriptenVersion::emscriptenWriteSrcInternal() const
{
    auto filePath = m_generator.emscriptenAbsSourceForRoot(strings::versionFileNameStr());
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
        "    #^#SPEC#$#\n"
        "    #^#PROT#$#\n"
        "}\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"HEADER", comms::relHeaderForRoot(strings::versionFileNameStr(), m_generator)},
        {"NAME", m_generator.emscriptenScopeNameForRoot(strings::versionFileNameStr())},
        {"SPEC", emscriptenSpecConstantsInternal()},
        {"PROT", emscriptenProtConstantsInternal()},
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string EmscriptenVersion::emscriptenSpecConstantsInternal() const
{
    const std::string Templ = 
        "emscripten::constant(\"#^#NS#$#_SPEC_VERSION\", #^#NS#$#_SPEC_VERSION);";

    util::ReplacementMap repl = {
        {"NS", util::strToUpper(m_generator.currentSchema().mainNamespace())}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenVersion::emscriptenProtConstantsInternal() const
{
    if (!m_generator.emscriptenHasProtocolVersion()) {
        return strings::emptyString();
    }

    const std::string Templ = 
        "emscripten::constant(\"#^#NS#$#_MAJOR_VERSION\", #^#NS#$#_MAJOR_VERSION);\n"
        "emscripten::constant(\"#^#NS#$#_MINOR_VERSION\", #^#NS#$#_MINOR_VERSION);\n"
        "emscripten::constant(\"#^#NS#$#_PATCH_VERSION\", #^#NS#$#_PATCH_VERSION);"
        ;

    util::ReplacementMap repl = {
        {"NS", util::strToUpper(m_generator.currentSchema().mainNamespace())}
    };

    return util::processTemplate(Templ, repl);    
}

} // namespace commsdsl2emscripten