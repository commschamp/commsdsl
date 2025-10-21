//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CInputMessages.h"

#include "CGenerator.h"
#include "CInterface.h"
#include "CNamespace.h"
#include "CProtocolOptions.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <cassert>
#include <fstream>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2c
{

namespace 
{

const std::string CName = "InputMessages";

} // namespace 
    

CInputMessages::CInputMessages(CGenerator& generator, const CNamespace& parent) :
    m_cGenerator(generator),
    m_parent(parent)
{
}

std::string CInputMessages::cRelHeader() const
{
    return m_cGenerator.cRelHeaderForInput(CName, m_parent);
}

std::string CInputMessages::cName() const
{
    return m_parent.cPrefixName() + "_input_" + CName;
}

bool CInputMessages::cWrite() const
{
    auto filePath = m_cGenerator.cAbsHeaderForInput(CName, m_parent);

    m_cGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "using #^#NAME#$# = #^#SCOPE#$#<#^#INTERFACE#$#, #^#OPTS#$#>;\n"
    ;

    auto* iFace = m_parent.cInterface();
    assert(iFace != nullptr);

    util::GenStringsList includes = {
        comms::genRelHeaderForInput(m_cGenerator.cInputName(), m_cGenerator, m_parent), 
        CProtocolOptions::cRelHeader(m_cGenerator),
        iFace->cRelCommsHeader(),
    };


    comms::genPrepareIncludeStatement(includes);

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
        {"NAME", cName()},
        {"INTERFACE", iFace->cCommsTypeName()},
        {"OPTS", CProtocolOptions::cName(m_cGenerator)},
        {"SCOPE", comms::genScopeForInput(m_cGenerator.cInputName(), m_cGenerator, m_parent)},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2c