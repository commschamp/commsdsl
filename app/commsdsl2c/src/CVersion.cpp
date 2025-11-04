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

#include "CVersion.h"

#include "CGenerator.h"
#include "CSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

namespace
{

enum CVersionIdx
{
    CVersionIdx_major,
    CVersionIdx_minor,
    CVersionIdx_patch,
    CVersionIdx_numOfValues
};

} // namespace

bool CVersion::cWrite(CGenerator& generator)
{
    CVersion obj(generator);
    return obj.cWriteInternal();
}

bool CVersion::cWriteInternal() const
{
    auto filePath = m_cGenerator.cAbsRootHeaderFor(strings::genVersionFileNameStr());

    m_cGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "/// @brief Version of the protocol specification.\n"
        "#define #^#PREFIX#$#_SPEC_VERSION (#^#VERSION#$#)\n\n"
        "#^#PROT_VER_DEFINE#$#\n"
        "#^#APPEND#$#\n";

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"VERSION", util::genNumToString(m_cGenerator.genCurrentSchema().genSchemaVersion())},
        {"PREFIX", util::genStrToUpper(m_cGenerator.cNamesPrefix())},
        {"PROT_VER_DEFINE", cProtVersionDefineInternal()},
        {"APPEND", util::genReadFileContents(m_cGenerator.cInputAbsRootHeaderFor(strings::genVersionFileNameStr()))},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();

    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string CVersion::cProtVersionDefineInternal() const
{
    auto& protVersion = m_cGenerator.genGetCodeVersion();
    if (protVersion.empty()) {
        return strings::genEmptyString();
    }

    auto tokens = util::genStrSplitByAnyChar(protVersion, ".");
    while (tokens.size() < CVersionIdx_numOfValues) {
        tokens.push_back("0");
    }

    const std::string Templ =
        "/// @brief Major version of the protocol library.\n"
        "#define #^#PREFIX#$#_MAJOR_VERSION (#^#MAJOR_VERSION#$#)\n\n"
        "/// @brief Minor version of the protocol library.\n"
        "#define #^#PREFIX#$#_MINOR_VERSION (#^#MINOR_VERSION#$#)\n\n"
        "/// @brief Patch version of the protocol library.\n"
        "#define #^#PREFIX#$#_PATCH_VERSION (#^#PATCH_VERSION#$#)\n\n"
        "/// @brief Make version as a single number.\n"
        "#define #^#PREFIX#$#_MAKE_VERSION(major_, minor_, patch_) \\\n"
        "    ((static_cast<unsigned>(major_) << 24) | \\\n"
        "     (static_cast<unsigned>(minor_) << 8) | \\\n"
        "     (static_cast<unsigned>(pathch_)))\n\n"
        "/// @brief Full version of the protocol library as single number.\n"
        "#define #^#PREFIX#$#_VERSION #^#PREFIX#$#_MAKE_VERSION(#^#PREFIX#$#_MAJOR_VERSION, #^#PREFIX#$#_MINOR_VERSION, #^#PREFIX#$#_PATCH_VERSION)\n"
        ;

    util::GenReplacementMap repl = {
        {"PREFIX", util::genStrToUpper(m_cGenerator.cNamesPrefix())},
        {"MAJOR_VERSION", tokens[CVersionIdx_major]},
        {"MINOR_VERSION", tokens[CVersionIdx_minor]},
        {"PATCH_VERSION", tokens[CVersionIdx_patch]},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c