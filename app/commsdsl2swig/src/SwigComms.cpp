//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "SwigComms.h"

#include "SwigGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

bool SwigComms::swigWrite(SwigGenerator& generator)
{
    SwigComms obj(generator);
    return obj.swigWriteInternal();
}

const std::string& SwigComms::swigRelHeader()
{
    static const std::string Str = strings::includeDirStr() + "/comms.h";
    return Str;
}

bool SwigComms::swigWriteInternal() const
{
    auto& schema = m_generator.protocolSchema();        
    auto swigName = schema.mainNamespace() + ".i";
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), swigRelHeader());
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }       

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "enum class comms_ErrorStatus\n"
        "{\n"
        "    Success,\n"
        "    UpdateRequired,\n"
        "    NotEnoughData,\n"
        "    ProtocolError,\n"
        "    BufferOverflow,\n"
        "    InvalidMsgId,\n"
        "    InvalidMsgData,\n"
        "    MsgAllocFailure,\n"
        "    NotSupported,\n"
        "    NumOfErrorStatuses\n"
        "};\n"
        ;      

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
    };

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2swig
