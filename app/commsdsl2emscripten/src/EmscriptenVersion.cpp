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
    return
        obj.emscriptenWriteHeaderInternal() &&
        obj.emscriptenWriteSrcInternal();
}

void EmscriptenVersion::emscriptenAddSourceFiles(const EmscriptenGenerator& generator, GenStringsList& sources)
{

    for (auto idx = 0U; idx < generator.genSchemas().size(); ++idx) {
        auto& schema = generator.genSchemas()[idx];
        if ((schema.get() != &generator.genCurrentSchema()) && (!schema->genHasAnyReferencedComponent())) {
            continue;
        }

        sources.push_back(generator.emscriptenSchemaRelSourceForRoot(idx, strings::genVersionFileNameStr()));
    }
}

std::string EmscriptenVersion::emscriptenRelHeader(const EmscriptenGenerator& generator)
{
    return generator.emscriptenRelHeaderForRoot(strings::genVersionFileNameStr());
}

bool EmscriptenVersion::emscriptenWriteHeaderInternal() const
{
    auto filePath = m_emscriptenGenerator.emscriptenAbsHeaderForRoot(strings::genVersionFileNameStr());
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
        "#include \"#^#HEADER#$#\"\n\n"
        "#define CC_EMSCRIPTEN_#^#NS#$#_SPEC_VERSION (#^#VERSION#$#)\n"
        "static_assert(CC_EMSCRIPTEN_#^#NS#$#_SPEC_VERSION == #^#NS#$#_SPEC_VERSION, \"Specification versions mismatch\");\n"
        "\n"
        "#^#CODE_VER#$#\n"
        "\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::emscriptenFileGeneratedComment()},
        {"HEADER", comms::genRelHeaderForRoot(strings::genVersionFileNameStr(), m_emscriptenGenerator)},
        {"NS", util::genStrToUpper(m_emscriptenGenerator.genCurrentSchema().genMainNamespace())},
        {"VERSION", util::genNumToString(m_emscriptenGenerator.genCurrentSchema().genSchemaVersion())},
        {"CODE_VER", emscriptenCodeVersionInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_emscriptenGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
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
        "    #^#CODE_VER#$#\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::emscriptenFileGeneratedComment()},
        {"HEADER", emscriptenRelHeader(m_emscriptenGenerator)},
        {"NAME", m_emscriptenGenerator.emscriptenScopeNameForRoot(strings::genVersionFileNameStr())},
        {"PROT", emscriptenProtConstantsInternal()},
        {"CODE_VER", emscriptenCodeVerConstantsInternal()},
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
        "emscripten::constant(\"#^#NS#$#_SPEC_VERSION\", #^#NS#$#_SPEC_VERSION);\n"
        "emscripten::constant(\"CC_EMSCRIPTEN_#^#NS#$#_SPEC_VERSION\", CC_EMSCRIPTEN_#^#NS#$#_SPEC_VERSION);";

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_emscriptenGenerator.genCurrentSchema().genMainNamespace())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenVersion::emscriptenProtConstantsInternal() const
{
    if (!m_emscriptenGenerator.emscriptenHasCodeVersion()) {
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

std::string EmscriptenVersion::emscriptenCodeVerConstantsInternal() const
{
    auto& codeVersion = m_emscriptenGenerator.genGetCodeVersion();
    if (codeVersion.empty()) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "emscripten::constant(\"CC_EMSCRIPTEN_#^#NS#$#_MAJOR_VERSION\", CC_EMSCRIPTEN_#^#NS#$#_MAJOR_VERSION);\n"
        "emscripten::constant(\"CC_EMSCRIPTEN_#^#NS#$#_MINOR_VERSION\", CC_EMSCRIPTEN_#^#NS#$#_MINOR_VERSION);\n"
        "emscripten::constant(\"CC_EMSCRIPTEN_#^#NS#$#_PATCH_VERSION\", CC_EMSCRIPTEN_#^#NS#$#_PATCH_VERSION);"
        ;

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_emscriptenGenerator.genCurrentSchema().genMainNamespace())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenVersion::emscriptenCodeVersionInternal() const
{
    auto tokens = m_emscriptenGenerator.genGetCodeVersionTokens();
    if (tokens.empty()) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "#define CC_EMSCRIPTEN_#^#NS#$#_MAJOR_VERSION (#^#MAJOR_VERSION#$#)\n"
        "#define CC_EMSCRIPTEN_#^#NS#$#_MINOR_VERSION (#^#MINOR_VERSION#$#)\n"
        "#define CC_EMSCRIPTEN_#^#NS#$#_PATCH_VERSION (#^#PATCH_VERSION#$#)\n"
        "\n"
        "#define CC_EMSCRIPTEN_#^#NS#$#_MAKE_VERSION(major_, minor_, patch_) \\\n"
        "    ((static_cast<unsigned>(major_) << 24) | \\\n"
        "     (static_cast<unsigned>(minor_) << 8) | \\\n"
        "     (static_cast<unsigned>(patch_)))\n\n"
        "#define CC_EMSCRIPTEN_#^#NS#$#_VERSION CC_EMSCRIPTEN_#^#NS#$#_MAKE_VERSION(CC_EMSCRIPTEN_#^#NS#$#_MAJOR_VERSION, CC_EMSCRIPTEN_#^#NS#$#_MINOR_VERSION, CC_EMSCRIPTEN_#^#NS#$#_PATCH_VERSION)\n"
        "static_assert(CC_EMSCRIPTEN_#^#NS#$#_VERSION == #^#NS#$#_VERSION, \"Versions mismatch\");\n"
        ;

    util::GenReplacementMap repl = {
        {"NS", util::genStrToUpper(m_emscriptenGenerator.genCurrentSchema().genMainNamespace())},
        {"MAJOR_VERSION", tokens[EmscriptenGenerator::GenVersionIdx_Major]},
        {"MINOR_VERSION", tokens[EmscriptenGenerator::GenVersionIdx_Minor]},
        {"PATCH_VERSION", tokens[EmscriptenGenerator::GenVersionIdx_Patch]},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten