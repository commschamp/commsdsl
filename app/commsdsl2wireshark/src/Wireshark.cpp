//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "Wireshark.h"

#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <fstream>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

bool Wireshark::wiresharkWrite(WiresharkGenerator& generator)
{
    Wireshark obj(generator);
    return obj.wiresharkWriteInternal();
}

std::string Wireshark::wiresharkFileName(const WiresharkGenerator& generator)
{
    return generator.genProtocolSchema().genMainNamespace() + ".lua";
}

bool Wireshark::wiresharkWriteInternal()
{
    auto fileName = wiresharkFileName(m_wiresharkGenerator);
    auto filePath = util::genPathAddElem(m_wiresharkGenerator.genGetOutputDir(), fileName);

    m_wiresharkGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_wiresharkGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    do {
        const std::string Templ =
            "#^#GEN_COMMENT#$#\n"
            ;

        util::GenReplacementMap repl = {
            {"GEN_COMMENT", m_wiresharkGenerator.wiresharkFileGeneratedComment()},
        };

        auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
        stream << str;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        m_wiresharkGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2wireshark

