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

#include "EmscriptenVersion.h"

#include "EmscriptenGenerator.h"
#include "EmscriptenSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <limits>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

bool EmscriptenVersion::emscriptenWrite(EmscriptenGenerator& generator)
{
    if ((!generator.genIsCurrentProtocolSchema()) && (!generator.genCurrentSchema().genHasAnyReferencedComponent())) {
        return true;
    }

    EmscriptenVersion obj(generator);
    return obj.emscriptenWriteSrcInternal();
}

void EmscriptenVersion::emscriptenAddSourceFiles(const EmscriptenGenerator& generator, GenStringsList& sources)
{

    for (auto idx = 0U; idx < generator.genSchemas().size(); ++idx) {
        auto& schema = generator.genSchemas()[idx];
        if ((schema.get() != &generator.genProtocolSchema()) && (!schema->genHasAnyReferencedComponent())) {
            continue;
        }

        sources.push_back(generator.emscriptenSchemaRelSourceForRoot(idx, strings::genVersionFileNameStr()));
    }
}

bool EmscriptenVersion::emscriptenWriteSrcInternal() const
{
    auto filePath = m_emscriptenGenerator.emscriptenAbsSourceForRoot(strings::genVersionFileNameStr());
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
        "    #^#SPEC#$#\n"
        "    #^#PROT#$#\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::emscriptenFileGeneratedComment()},
        {"HEADER", comms::genRelHeaderForRoot(strings::genVersionFileNameStr(), m_emscriptenGenerator)},
        {"NAME", m_emscriptenGenerator.emscriptenScopeNameForRoot(strings::genVersionFileNameStr())},
        {"SPEC", emscriptenSpecConstantsInternal()},
        {"PROT", emscriptenProtConstantsInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_emscriptenGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string EmscriptenVersion::emscriptenSpecConstantsInternal() const
{
    const std::string Templ =
        "emscripten::constant(\"#^#NS#$#_SPEC_VERSION\", #^#NS#$#_SPEC_VERSION);";

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_emscriptenGenerator.genCurrentSchema().genMainNamespace())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenVersion::emscriptenProtConstantsInternal() const
{
    if (!m_emscriptenGenerator.emscriptenHasProtocolVersion()) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "emscripten::constant(\"#^#NS#$#_MAJOR_VERSION\", #^#NS#$#_MAJOR_VERSION);\n"
        "emscripten::constant(\"#^#NS#$#_MINOR_VERSION\", #^#NS#$#_MINOR_VERSION);\n"
        "emscripten::constant(\"#^#NS#$#_PATCH_VERSION\", #^#NS#$#_PATCH_VERSION);"
        ;

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_emscriptenGenerator.genCurrentSchema().genMainNamespace())}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten