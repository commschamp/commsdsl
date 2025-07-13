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

#include "SwigDataBuf.h"

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

const std::string SwigClassName("DataBuf");


} // namespace 
    
bool SwigDataBuf::swigWrite(SwigGenerator& generator)
{
    SwigDataBuf obj(generator);
    return obj.swigWriteInternal();
}

void SwigDataBuf::swigAddDef(const SwigGenerator& generator, GenStringsList& list)
{
    static const std::string Templ = 
        "%template(#^#CLASS_NAME#$#) std::vector<#^#UINT8_T#$#>;\n"
        "%feature(\"valuewrapper\") #^#CLASS_NAME#$#;";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)},
        {"UINT8_T", SwigGenerator::swigCast(generator).swigConvertCppType("std::uint8_t")}
    };

    list.push_back(util::genProcessTemplate(Templ, repl));        

    list.push_back(SwigGenerator::swigDefInclude(SwigClassName + strings::genCppHeaderSuffixStr()));
}

void SwigDataBuf::swigAddCode(const SwigGenerator& generator, GenStringsList& list)
{
    const std::string Templ = 
        "using #^#CLASS_NAME#$# = std::vector<#^#UINT8_T#$#>;\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)},
        {"UINT8_T", SwigGenerator::swigCast(generator).swigConvertCppType("std::uint8_t")}
    };

    list.push_back(util::genProcessTemplate(Templ, repl));
}

std::string SwigDataBuf::swigClassName(const SwigGenerator& generator)
{
    return generator.swigProtocolClassNameForRoot(SwigClassName);
}

bool SwigDataBuf::swigWriteInternal() const
{
    auto subPath = util::genPathAddElem(strings::genIncludeDirStr(), SwigClassName + strings::genCppHeaderSuffixStr());
    auto filePath = util::genPathAddElem(m_swigGenerator.genGetOutputDir(), subPath);
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
        "using #^#CLASS_NAME#$# = std::vector<#^#UINT8_T#$#>;\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", SwigGenerator::swigFileGeneratedComment()},
        {"CLASS_NAME", swigClassName(m_swigGenerator)},
        {"UINT8_T", SwigGenerator::swigCast(m_swigGenerator).swigConvertCppType("std::uint8_t")}
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_swigGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2swig