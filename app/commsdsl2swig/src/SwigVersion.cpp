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

const std::string FileName("Version");
const std::string SpecVersionFunc("specVersion");
const std::string MajorVersionFunc("versionMajor");
const std::string MinorVersionFunc("versionMinor");
const std::string PatchVersionFunc("versionPatch");

} // namespace 
    

bool SwigVersion::swigWrite(SwigGenerator& generator)
{
    SwigVersion obj(generator);
    return obj.swigWriteInternal();
}

void SwigVersion::swigAddCodeIncludes(SwigGenerator& generator, StringsList& list)
{
    assert(generator.isCurrentProtocolSchema());

    auto& schemas = generator.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        generator.chooseCurrentSchema(idx);
        list.push_back(comms::relHeaderForRoot(FileName, generator));
    }

    generator.chooseProtocolSchema();
}

void SwigVersion::swigAddDef(SwigGenerator& generator, StringsList& list)
{
    assert(generator.isCurrentProtocolSchema());
    auto& schemas = generator.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        generator.chooseCurrentSchema(idx);

        const std::string Templ = 
            "%constant unsigned #^#NS#$#_#^#NAME#$# = #^#NS#$#_#^#NAME#$#;";

        util::ReplacementMap repl = {
            {"NS", util::strToUpper(schemas[idx]->mainNamespace())},
            {"NAME", "SPEC_VERSION"}
        };
        list.push_back(util::processTemplate(Templ, repl));

        if (generator.isCurrentProtocolSchema() && generator.swigHasProtocolVersion()) {
            repl["NAME"] = "MAJOR_VERSION";
            list.push_back(util::processTemplate(Templ, repl));

            repl["NAME"] = "MINOR_VERSION";
            list.push_back(util::processTemplate(Templ, repl));

            repl["NAME"] = "PATCH_VERSION";
            list.push_back(util::processTemplate(Templ, repl));
        }          

        list.push_back(SwigGenerator::swigDefInclude(comms::relHeaderForRoot(FileName, generator)));
    }
    assert(generator.isCurrentProtocolSchema());
}

void SwigVersion::swigAddCode(SwigGenerator& generator, StringsList& list)
{
    const std::string Templ = 
        "unsigned #^#NAME#$#()\n"
        "{\n"
        "    return #^#COMMS_SCOPE#$#();\n"
        "}\n";

    assert(generator.isCurrentProtocolSchema());
    auto& schemas = generator.schemas();
    for (auto idx = 0U; idx < schemas.size(); ++idx) {
        generator.chooseCurrentSchema(idx);
        util::ReplacementMap specRepl = {
            {"NAME", generator.swigScopeNameForRoot(SpecVersionFunc)},
            {"COMMS_SCOPE", comms::scopeForRoot(SpecVersionFunc, generator)}
        };

        list.push_back(util::processTemplate(Templ, specRepl));

        if ((!generator.isCurrentProtocolSchema()) || (!generator.swigHasProtocolVersion())) {
            continue;
        }

        util::ReplacementMap majorRepl = {
            {"NAME", generator.swigScopeNameForRoot(MajorVersionFunc)},
            {"COMMS_SCOPE", comms::scopeForRoot(MajorVersionFunc, generator)}
        };

        list.push_back(util::processTemplate(Templ, majorRepl));

        util::ReplacementMap minorRepl = {
            {"NAME", generator.swigScopeNameForRoot(MinorVersionFunc)},
            {"COMMS_SCOPE", comms::scopeForRoot(MinorVersionFunc, generator)}
        }; 

        list.push_back(util::processTemplate(Templ, minorRepl));  

        util::ReplacementMap patchRepl = {
            {"NAME", generator.swigScopeNameForRoot(PatchVersionFunc)},
            {"COMMS_SCOPE", comms::scopeForRoot(PatchVersionFunc, generator)}
        }; 

        list.push_back(util::processTemplate(Templ, patchRepl));  
    }

    assert(generator.isCurrentProtocolSchema());
}

bool SwigVersion::swigWriteInternal() const
{
    auto filePath = comms::headerPathRoot(FileName, m_generator);
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
        "#pragma once\n\n"
        "unsigned #^#SPEC_VERSION#$#();\n"
        "#^#PROTOCOL#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"SPEC_VERSION", m_generator.swigScopeNameForRoot(SpecVersionFunc)}
    };

    if (m_generator.isCurrentProtocolSchema() && m_generator.swigHasProtocolVersion()) {
        const std::string ProtTempl = 
            "unsigned #^#MAJOR#$#();\n"
            "unsigned #^#MINOR#$#();\n"
            "unsigned #^#PATCH#$#();\n"
        ;  

        util::ReplacementMap protRepl = {
            {"MAJOR", m_generator.swigScopeNameForRoot(MajorVersionFunc)},
            {"MINOR", m_generator.swigScopeNameForRoot(MinorVersionFunc)},
            {"PATCH", m_generator.swigScopeNameForRoot(PatchVersionFunc)},
        };

        repl["PROTOCOL"] = util::processTemplate(ProtTempl, protRepl);
    }    

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}


} // namespace commsdsl2swig