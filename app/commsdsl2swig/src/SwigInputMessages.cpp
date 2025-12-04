//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "SwigInputMessages.h"

#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigNamespace.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

SwigInputMessages::SwigInputMessages(SwigGenerator& generator, const SwigNamespace& parent) :
    m_swigGenerator(generator),
    m_parent(parent)
{
}

void SwigInputMessages::swigAddCode(GenStringsList& list) const
{
    auto allMessages = swigMessagesListInternal();
    util::GenStringsList msgList;
    msgList.reserve(allMessages.size());

    auto* iFace = m_parent.swigInterface();
    assert(iFace != nullptr);
    auto interfaceClassName = m_swigGenerator.swigClassName(*iFace);

    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }
        msgList.push_back(m_swigGenerator.swigClassName(*m));
    }

    const std::string Templ =
        "using #^#NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#MESSAGES#$#\n"
        "    >;\n";

    util::GenReplacementMap repl = {
        {"NAME", swigClassName()},
        {"MESSAGES", util::genStrListToString(msgList, ",\n", "")}
    };

    list.push_back(util::genProcessTemplate(Templ, repl));
}

std::string SwigInputMessages::swigClassName() const
{
    return m_swigGenerator.swigScopeNameForNamespaceMember(strings::genAllMessagesStr(), m_parent);
}

SwigInputMessages::GenMessagesAccessList SwigInputMessages::swigMessagesListInternal() const
{
    auto allMessages = m_parent.genGetAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.genName().empty()) {
        allMessages = m_swigGenerator.genCurrentSchema().genGetAllMessagesIdSorted();
    }

    return allMessages;
}

} // namespace commsdsl2swig
