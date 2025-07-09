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

#include "CommsMsgId.h"

#include "CommsGenerator.h"
#include "CommsEnumField.h"
#include "CommsNamespace.h"
#include "CommsSchema.h"

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

namespace commsdsl2comms
{

namespace 
{

using GenReplacementMap = commsdsl::gen::util::GenReplacementMap;

} // namespace 
    
CommsMsgId::CommsMsgId(CommsGenerator& generator, const CommsNamespace& parent) :
    m_commsGenerator(generator),
    m_parent(parent)
{
}

bool CommsMsgId::commsWrite() const
{
    auto filePath = comms::genHeaderPathForMsgId(strings::genMsgIdEnumNameStr(), m_commsGenerator, m_parent);

    m_commsGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_commsGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_commsGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of message ids enumeration.\n\n"
        "#pragma once\n\n"
        "#include <cstdint>\n"
        "#include \"#^#PROT_NAMESPACE#$#/Version.h\"\n\n"
        "#^#NS_BEGIN#$#\n\n"
        "/// @brief Message ids enumeration.\n"
        "enum MsgId : #^#TYPE#$#\n"
        "{\n"
        "    #^#IDS#$#\n"
        "};\n\n"
        "#^#NS_END#$#\n";
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"PROT_NAMESPACE", m_commsGenerator.genCurrentSchema().genMainNamespace()},
        {"TYPE", commsTypeInternal()},
        {"IDS", commsIdsInternal()},
        {"NS_BEGIN", comms::genNamespaceBeginFor(m_parent, m_commsGenerator)},
        {"NS_END", comms::genNamespaceEndFor(m_parent, m_commsGenerator)},           
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_commsGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string CommsMsgId::commsTypeInternal() const
{
    auto allMsgIdFields = m_parent.genFindMessageIdFields();
    if (allMsgIdFields.size() == 1U) {
        auto* msgIdField = allMsgIdFields.front();
        assert(msgIdField->genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Enum);
        auto* castedMsgIdField = static_cast<const CommsEnumField*>(msgIdField);
        auto parseObj = castedMsgIdField->genEnumFieldParseObj();
        return comms::genCppIntTypeFor(parseObj.parseType(), parseObj.parseMaxLength());
    }

    auto allMessages = m_parent.genGetAllMessages();
    auto iter = 
        std::max_element(
            allMessages.begin(), allMessages.end(),
            [](auto* first, auto* second)
            {
                return first->genParseObj().parseId() < second->genParseObj().parseId();
            });

    std::string result = "unsigned";
    do {
        if (iter == allMessages.end()) {
            break;
        }

        auto maxId = (*iter)->genParseObj().parseId();
        bool fitsUnsigned = maxId <= std::numeric_limits<unsigned>::max();
        if (fitsUnsigned) {
            break;
        }

        result = "unsigned long long";
    } while (false);

    return result;
}

std::string CommsMsgId::commsIdsInternal() const
{
    auto& prefix = strings::genMsgIdPrefixStr();
    auto allMsgIdFields = m_parent.genFindMessageIdFields();
    if (allMsgIdFields.empty() && m_parent.genName().empty()) {
        allMsgIdFields = m_commsGenerator.genCurrentSchema().genGetAllMessageIdFields();
    }

    if (allMsgIdFields.size() == 1U) {    
        auto* msgIdField = allMsgIdFields.front();
        assert(msgIdField->genParseObj().parseKind() == commsdsl::parse::ParseField::ParseKind::Enum);
        auto* castedMsgIdField = static_cast<const CommsEnumField*>(msgIdField);
        auto enumValues = castedMsgIdField->commsEnumValues();
        static const std::string CommentPrefix("// ---");
        for (auto& v : enumValues) {
            auto commentPos = v.find(CommentPrefix);
            if (commentPos != std::string::npos) {
                continue;
            }

            v.insert(v.begin(), prefix.begin(), prefix.end());
        }

        return util::genStrListToString(enumValues, "\n", "");
    }

    auto allMessages = m_parent.genGetAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.genName().empty()) {
        allMessages = m_commsGenerator.genCurrentSchema().genGetAllMessagesIdSorted();
    }

    util::GenStringsList ids;
    ids.reserve(allMessages.size());
    for (auto* m : allMessages) {
        ids.push_back(prefix + comms::genFullNameFor(*m) + " = " + util::genNumToString(m->genParseObj().parseId()));
    }
    return util::genStrListToString(ids, ",\n", "");
}

} // namespace commsdsl2comms