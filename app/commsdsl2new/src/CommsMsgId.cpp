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

#include "CommsMsgId.h"

#include "CommsGenerator.h"
#include "CommsEnumField.h"

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

namespace commsdsl2new
{

namespace 
{

using ReplacementMap = commsdsl::gen::util::ReplacementMap;

} // namespace 
    

bool CommsMsgId::write(CommsGenerator& generator)
{
    CommsMsgId obj(generator);
    return obj.commsWriteInternal();
}

bool CommsMsgId::commsWriteInternal() const
{
    static_cast<void>(m_generator);
    auto filePath = comms::headerPathRoot(strings::msgIdEnumNameStr(), m_generator);

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of message ids enumeration.\n\n"
        "#pragma once\n\n"
        "#include <cstdint>\n"
        "#include \"#^#PROT_NAMESPACE#$#/Version.h\"\n\n"
        "namespace #^#PROT_NAMESPACE#$#\n"
        "{\n"
        "/// @brief Message ids enumeration.\n"
        "enum MsgId : #^#TYPE#$#\n"
        "{\n"
        "    #^#IDS#$#\n"
        "};\n\n"
        "} // namespace #^#PROT_NAMESPACE#$#\n\n"        
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"PROT_NAMESPACE", m_generator.mainNamespace()},
        {"TYPE", commsTypeInternal()},
        {"IDS", commsIdsInternal()}
    };

    stream << util::processTemplate(Templ, repl);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string CommsMsgId::commsTypeInternal() const
{
    auto* msgIdField = m_generator.getMessageIdField();
    if (msgIdField != nullptr) {
        assert(msgIdField->dslObj().kind() == commsdsl::parse::Field::Kind::Enum);
        auto* castedMsgIdField = static_cast<const CommsEnumField*>(msgIdField);
        auto dslObj = castedMsgIdField->enumDslObj();
        return comms::cppIntTypeFor(dslObj.type(), dslObj.maxLength());
    }

    auto allMessages = m_generator.getAllMessages();
    assert(!allMessages.empty());
    auto iter = 
        std::max_element(
            allMessages.begin(), allMessages.end(),
            [](auto* first, auto* second)
            {
                return first->dslObj().id() < second->dslObj().id();
            });

    auto maxId = (*iter)->dslObj().id();
    bool fitsUnsigned = maxId <= std::numeric_limits<unsigned>::max();
    if (fitsUnsigned) {
        return "unsigned";
    }

    return "unsigned long long";
}

std::string CommsMsgId::commsIdsInternal() const
{
    auto* msgIdField = m_generator.getMessageIdField();
    auto& prefix = strings::msgIdPrefixStr();
    if (msgIdField != nullptr) {
        assert(msgIdField->dslObj().kind() == commsdsl::parse::Field::Kind::Enum);
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

        return util::strListToString(enumValues, ",\n", "");
    }

    auto allMessages = m_generator.getAllMessages();
    util::StringsList ids;
    ids.reserve(allMessages.size());
    for (auto* m : allMessages) {
        ids.push_back(prefix + comms::fullNameFor(*m) + " = " + util::numToString(m->dslObj().id()));
    }
    return util::strListToString(ids, ",\n", "");
}

} // namespace commsdsl2new