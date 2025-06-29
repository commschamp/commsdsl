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

#include "CommsFieldBase.h"

#include "CommsGenerator.h"
#include "CommsSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <fstream>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

bool CommsFieldBase::write(CommsGenerator& generator)
{
    auto& thisSchema = static_cast<CommsSchema&>(generator.genCurrentSchema());
    if ((!generator.genIsCurrentProtocolSchema()) && (!thisSchema.commsHasAnyField())) {
        return true;
    }

    CommsFieldBase obj(generator);
    return obj.commsWriteInternal();
}

bool CommsFieldBase::commsWriteInternal() const
{
    auto filePath = comms::genHeaderPathForField(strings::genFieldBaseClassStr(), m_generator);

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
        "/// @file\n"
        "/// @brief Contains definition of base class of all the fields.\n\n"
        "#pragma once\n\n"
        "#include \"comms/Field.h\"\n"
        "#include \"comms/options.h\"\n"
        "#include \"#^#PROT_NAMESPACE#$#/Version.h\"\n\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n\n"
        "namespace field\n"
        "{\n\n"
        "/// @brief Common base class for all the fields.\n"
        "/// @tparam TOpt Extra options.\n"
        "template <typename... TOpt>\n"
        "using FieldBase =\n"
        "    comms::Field<\n"
        "        TOpt...,\n"
        "        #^#OPTIONS#$#\n"
        "    >;\n\n"
        "} // namespace field\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n";    

    util::StringsList options;
    options.push_back(comms::genParseEndianToOpt(m_generator.genCurrentSchema().genSchemaEndian()));
    // TODO: version type

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"PROT_NAMESPACE", m_generator.genCurrentSchema().genMainNamespace()},
        {"OPTIONS", util::genStrListToString(options, ",\n", "")},
    };        
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2comms