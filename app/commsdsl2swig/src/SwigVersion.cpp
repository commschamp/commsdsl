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

#include "SwigVersion.h"

#include "SwigGenerator.h"
#include "SwigSchema.h"

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

namespace commsdsl2swig
{

namespace
{

const std::string SpecVersionFunc("specVersion");
const std::string MajorVersionFunc("versionMajor");
const std::string MinorVersionFunc("versionMinor");
const std::string PatchVersionFunc("versionPatch");

} // namespace

bool SwigVersion::swigWrite(SwigGenerator& generator)
{
    if ((!generator.genIsCurrentProtocolSchema()) && (!generator.genCurrentSchema().genHasAnyReferencedComponent())) {
        return true;
    }

    SwigVersion obj(generator);
    return obj.swigWriteInternal();
}

void SwigVersion::swigAddCodeIncludes(SwigGenerator& generator, GenStringsList& list)
{
    assert(generator.genIsCurrentProtocolSchema());

    auto& schemas = generator.genSchemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        generator.genChooseCurrentSchema(idx);
        if ((!generator.genIsCurrentProtocolSchema()) && (!generator.genCurrentSchema().genHasAnyReferencedComponent())) {
            continue;
        }
        list.push_back(comms::genRelHeaderForRoot(strings::genVersionFileNameStr(), generator));
    }

    generator.genChooseProtocolSchema();
}

void SwigVersion::swigAddDef(SwigGenerator& generator, GenStringsList& list)
{
    assert(generator.genIsCurrentProtocolSchema());
    auto& schemas = generator.genSchemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        generator.genChooseCurrentSchema(idx);

        if ((!generator.genIsCurrentProtocolSchema()) && (!generator.genCurrentSchema().genHasAnyReferencedComponent())) {
            continue;
        }

        const std::string Templ =
            "%constant unsigned #^#NS#$#_#^#NAME#$# = #^#NS#$#_#^#NAME#$#;";

        util::GenReplacementMap repl = {
            {"NS", util::genStrToUpper(schemas[idx]->genMainNamespace())},
            {"NAME", "SPEC_VERSION"}
        };
        list.push_back(util::genProcessTemplate(Templ, repl));

        if (generator.genIsCurrentProtocolSchema() && generator.swigHasProtocolVersion()) {
            repl["NAME"] = "MAJOR_VERSION";
            list.push_back(util::genProcessTemplate(Templ, repl));

            repl["NAME"] = "MINOR_VERSION";
            list.push_back(util::genProcessTemplate(Templ, repl));

            repl["NAME"] = "PATCH_VERSION";
            list.push_back(util::genProcessTemplate(Templ, repl));
        }

        list.push_back(SwigGenerator::swigDefInclude(comms::genRelHeaderForRoot(strings::genVersionFileNameStr(), generator)));
    }
    assert(generator.genIsCurrentProtocolSchema());
}

void SwigVersion::swigAddCode(SwigGenerator& generator, GenStringsList& list)
{
    const std::string Templ =
        "unsigned #^#NAME#$#()\n"
        "{\n"
        "    return #^#COMMS_SCOPE#$#();\n"
        "}\n";

    assert(generator.genIsCurrentProtocolSchema());
    auto& schemas = generator.genSchemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        generator.genChooseCurrentSchema(idx);
        if ((!generator.genIsCurrentProtocolSchema()) && (!generator.genCurrentSchema().genHasAnyReferencedComponent())) {
            continue;
        }

        util::GenReplacementMap specRepl = {
            {"NAME", generator.swigScopeNameForRoot(SpecVersionFunc)},
            {"COMMS_SCOPE", comms::genScopeForRoot(SpecVersionFunc, generator)}
        };

        list.push_back(util::genProcessTemplate(Templ, specRepl));

        if ((!generator.genIsCurrentProtocolSchema()) || (!generator.swigHasProtocolVersion())) {
            continue;
        }

        util::GenReplacementMap majorRepl = {
            {"NAME", generator.swigScopeNameForRoot(MajorVersionFunc)},
            {"COMMS_SCOPE", comms::genScopeForRoot(MajorVersionFunc, generator)}
        };

        list.push_back(util::genProcessTemplate(Templ, majorRepl));

        util::GenReplacementMap minorRepl = {
            {"NAME", generator.swigScopeNameForRoot(MinorVersionFunc)},
            {"COMMS_SCOPE", comms::genScopeForRoot(MinorVersionFunc, generator)}
        };

        list.push_back(util::genProcessTemplate(Templ, minorRepl));

        util::GenReplacementMap patchRepl = {
            {"NAME", generator.swigScopeNameForRoot(PatchVersionFunc)},
            {"COMMS_SCOPE", comms::genScopeForRoot(PatchVersionFunc, generator)}
        };

        list.push_back(util::genProcessTemplate(Templ, patchRepl));
    }

    assert(generator.genIsCurrentProtocolSchema());
}

bool SwigVersion::swigWriteInternal() const
{
    auto filePath = comms::genHeaderPathRoot(strings::genVersionFileNameStr(), m_swigGenerator);
    m_swigGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_swigGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_swigGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "unsigned #^#SPEC_VERSION#$#();\n"
        "#^#PROTOCOL#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", SwigGenerator::swigFileGeneratedComment()},
        {"SPEC_VERSION", m_swigGenerator.swigScopeNameForRoot(SpecVersionFunc)}
    };

    if (m_swigGenerator.genIsCurrentProtocolSchema() && m_swigGenerator.swigHasProtocolVersion()) {
        const std::string ProtTempl =
            "unsigned #^#MAJOR#$#();\n"
            "unsigned #^#MINOR#$#();\n"
            "unsigned #^#PATCH#$#();\n"
        ;

        util::GenReplacementMap protRepl = {
            {"MAJOR", m_swigGenerator.swigScopeNameForRoot(MajorVersionFunc)},
            {"MINOR", m_swigGenerator.swigScopeNameForRoot(MinorVersionFunc)},
            {"PATCH", m_swigGenerator.swigScopeNameForRoot(PatchVersionFunc)},
        };

        repl["PROTOCOL"] = util::genProcessTemplate(ProtTempl, protRepl);
    }

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_swigGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2swig