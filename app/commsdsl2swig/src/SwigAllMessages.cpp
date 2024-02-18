//
// Copyright 2022 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "SwigAllMessages.h"

#include "SwigGenerator.h"
#include "SwigInterface.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

void SwigAllMessages::swigAddCode(const SwigGenerator& generator, StringsList& list)
{
    auto allMessages = generator.getAllMessagesIdSorted();
    util::StringsList msgList;
    msgList.reserve(allMessages.size());

    auto* iFace = generator.swigMainInterface();
    assert(iFace != nullptr);
    auto interfaceClassName = generator.swigClassName(*iFace);


    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }
        msgList.push_back(generator.swigClassName(*m));
    }

    const std::string Templ = 
        "using #^#NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#MESSAGES#$#\n"
        "    >;\n";

    util::ReplacementMap repl = {
        {"NAME", strings::allMessagesStr()},
        {"MESSAGES", util::strListToString(msgList, ",\n", "")}
    };

    list.push_back(util::processTemplate(Templ, repl));
}

} // namespace commsdsl2swig
