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

const std::string ClassName("DataBuf");


} // namespace 
    


bool SwigDataBuf::write(SwigGenerator& generator)
{
    SwigDataBuf obj(generator);
    return obj.swigWriteInternal();
}

void SwigDataBuf::swigAddDef(const SwigGenerator& generator, StringsList& list)
{
    static const std::string Templ = 
        "%template(#^#CLASS_NAME#$#) std::vector<#^#UINT8_T#$#>;";

    util::ReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)},
        {"UINT8_T", SwigGenerator::cast(generator).swigConvertCppType("std::uint8_t")}
    };

    list.push_back(util::processTemplate(Templ, repl));        

    list.push_back(SwigGenerator::swigDefInclude(ClassName + strings::cppHeaderSuffixStr()));
}

void SwigDataBuf::swigAddCode(const SwigGenerator& generator, StringsList& list)
{
    const std::string Templ = 
        "using #^#CLASS_NAME#$# = std::vector<#^#UINT8_T#$#>;\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)},
        {"UINT8_T", SwigGenerator::cast(generator).swigConvertCppType("std::uint8_t")}
    };

    list.push_back(util::processTemplate(Templ, repl));
}

std::string SwigDataBuf::swigClassName(const SwigGenerator& generator)
{
    return generator.swigProtocolClassNameForRoot(ClassName);
}

bool SwigDataBuf::swigWriteInternal() const
{
    auto subPath = util::pathAddElem(strings::includeDirStr(), ClassName + strings::cppHeaderSuffixStr());
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), subPath);
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
        "using #^#CLASS_NAME#$# = std::vector<#^#UINT8_T#$#>;\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"CLASS_NAME", swigClassName(m_generator)},
        {"UINT8_T", SwigGenerator::cast(m_generator).swigConvertCppType("std::uint8_t")}
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2swig