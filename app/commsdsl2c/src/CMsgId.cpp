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

#include "CMsgId.h"

#include "CGenerator.h"
#include "CEnumField.h"
#include "CNamespace.h"
#include "CSchema.h"

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

CMsgId::CMsgId(CGenerator& generator, const CNamespace& parent) :
    m_cGenerator(generator),
    m_parent(parent)
{
}

std::string CMsgId::cRelHeader() const
{
    return m_cGenerator.cRelHeaderForNamespaceMember(strings::genMsgIdEnumNameStr(), m_parent);
}

std::string CMsgId::cName() const
{
    return m_parent.cPrefixName() + '_' + strings::genMsgIdEnumNameStr();
}

bool CMsgId::cWrite() const
{
    auto filePath = m_cGenerator.cAbsHeaderForNamespaceMember(strings::genMsgIdEnumNameStr(), m_parent);

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
        "#^#CPP_GUARD_BEGIN#$#\n\n"
        "/// @brief Message ids enumeration.\n"
        "typedef enum\n"
        "{\n"
        "    #^#IDS#$#\n"
        "} #^#NAME#$#;\n\n"
        "#^#CPP_GUARD_END#$#\n";
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"IDS", cIdsInternal()},
        {"CPP_GUARD_BEGIN", CGenerator::cCppGuardBegin()},
        {"CPP_GUARD_END", CGenerator::cCppGuardEnd()},
        {"NAME", cName()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string CMsgId::cIdsInternal() const
{
    auto prefix = cName();
    auto allMsgIdFields = m_parent.genFindMessageIdFields();
    if (allMsgIdFields.empty() && m_parent.genName().empty()) {
        allMsgIdFields = m_cGenerator.genCurrentSchema().genGetAllMessageIdFields();
    }

    if (allMsgIdFields.size() == 1U) {
        auto* msgIdField = allMsgIdFields.front();
        assert(msgIdField->genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Enum);
        auto* castedMsgIdField = static_cast<const CEnumField*>(msgIdField);
        auto enumValues = castedMsgIdField->cEnumValues(prefix);
        return util::genStrListToString(enumValues, "\n", "");
    }

    auto allMessages = m_parent.genGetAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.genName().empty()) {
        allMessages = m_cGenerator.genCurrentSchema().genGetAllMessagesIdSorted();
    }

    util::GenStringsList ids;
    ids.reserve(allMessages.size());
    for (auto* m : allMessages) {
        ids.push_back(prefix + '_' + comms::genFullNameFor(*m) + " = " + util::genNumToString(m->genParseObj().parseId()));
    }
    return util::genStrListToString(ids, ",\n", "");
}

} // namespace commsdsl2c