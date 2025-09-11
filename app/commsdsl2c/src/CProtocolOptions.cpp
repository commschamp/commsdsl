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

#include "CProtocolOptions.h"

#include "CGenerator.h"

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

namespace commsdsl2c
{

namespace 
{

std::string cCodeInternal(const util::GenStringsList& opts, std::size_t idx)
{
    if (idx == 0U) {
        return opts[idx];
    }

    auto wrappedCode = cCodeInternal(opts, idx - 1U);
    if (wrappedCode.empty()) {
        return opts[idx];
    }

    static const std::string Templ = 
        "#^#OPT#$#T<\n"
        "    #^#NEXT#$#\n"
        ">";

    util::GenReplacementMap repl = {
        {"OPT", opts[idx]},
        {"NEXT", std::move(wrappedCode)}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

} // namespace 
  
std::string CProtocolOptions::cClassName(const CGenerator& generator)
{
    return generator.cNamesPrefix() + "ProtocolOptions";
}

std::string CProtocolOptions::cRelHeaderPath(const CGenerator& generator)
{
    return 
        generator.genGetTopNamespace() + '/' + 
        generator.genProtocolSchema().genMainNamespace() + '/' +
        strings::genOptionsNamespaceStr() + '/' +
        cClassName(generator) + strings::genCppHeaderSuffixStr();
}

bool CProtocolOptions::cWrite(CGenerator& generator)
{
    CProtocolOptions obj(generator);
    return obj.cWriteHeaderInternal();
}

CProtocolOptions::CProtocolOptions(CGenerator& generator) :
    m_cGenerator(generator)
{
}    

bool CProtocolOptions::cWriteHeaderInternal()
{
    auto filePath = m_cGenerator.genGetOutputDir() + '/' + cRelHeaderPath(m_cGenerator);
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
        "#^#INCLUDES#$#\n"
        "#^#DEF#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cIncludesInternal()},
        {"DEF", cTypeDefInternal()}
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string CProtocolOptions::cTypeDefInternal()
{
    assert(m_cGenerator.genIsCurrentProtocolSchema());

    const std::string Templ = 
        "using #^#OPT_TYPE#$# =\n"
        "    #^#CODE#$#;\n\n"
        ;

    auto& opts = m_cGenerator.cProtocolOptions();
    util::GenReplacementMap repl = {
        {"OPT_TYPE", cClassName(m_cGenerator)},
        {"CODE", cCodeInternal(opts, opts.size() - 1U)},
    };

    m_cGenerator.genChooseProtocolSchema();
    return util::genProcessTemplate(Templ, repl);
}

std::string CProtocolOptions::cIncludesInternal()
{
    assert(m_cGenerator.genIsCurrentProtocolSchema());
    auto includes = m_cGenerator.cProtocolOptions(); // copy
    for (auto& i : includes) {
        i = util::genScopeToRelPath(i) + strings::genCppHeaderSuffixStr();
    }

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

} // namespace commsdsl2c