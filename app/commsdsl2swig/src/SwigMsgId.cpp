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

#include "SwigMsgId.h"

#include "SwigGenerator.h"
#include "SwigEnumField.h"
#include "SwigNamespace.h"
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

SwigMsgId::SwigMsgId(SwigGenerator& generator, const SwigNamespace& parent) :
    m_generator(generator),
    m_parent(parent)
{
}

bool SwigMsgId::swigWrite() const
{
    auto filePath = comms::headerPathForMsgId(strings::msgIdEnumNameStr(), m_generator, m_parent);
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
        "enum #^#CLASS_NAME#$# : #^#TYPE#$#\n"
        "{\n"
        "    #^#IDS#$#\n"
        "};\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"CLASS_NAME", swigClassName()},
        {"TYPE", swigTypeInternal()},
        {"IDS", swigIdsInternal()}
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true; 
}

void SwigMsgId::swigAddDef(StringsList& list) const
{
    list.push_back(SwigGenerator::swigDefInclude(comms::relHeaderForMsgId(strings::msgIdEnumNameStr(), m_generator, m_parent)));
}

void SwigMsgId::swigAddCode(StringsList& list) const
{
    const std::string Templ = 
        "using #^#SWIG_TYPE#$# = #^#COMMS_TYPE#$#;\n";

    auto commsType = comms::scopeForMsgId(strings::msgIdEnumNameStr(), m_generator, m_parent);
    util::ReplacementMap repl = {
        {"SWIG_TYPE", swigClassName()},
        {"COMMS_TYPE", commsType}
    };

    list.push_back(util::processTemplate(Templ, repl));

    list.push_back(swigCodeInternal());
}

std::string SwigMsgId::swigClassName() const
{
    return m_generator.swigScopeNameForMsgId(strings::msgIdEnumNameStr(), m_parent);
}

void SwigMsgId::swigAddCodeIncludes(StringsList& list) const
{
    list.push_back(comms::relHeaderForNamespaceMember(strings::msgIdEnumNameStr(), m_generator, m_parent));
}

std::string SwigMsgId::swigTypeInternal() const
{
    auto allMsgIds = m_generator.currentSchema().getAllMessageIdFields();
    if (allMsgIds.size() == 1U) {
        auto* msgIdField = allMsgIds.front();
        assert(msgIdField->dslObj().kind() == commsdsl::parse::ParseField::Kind::Enum);
        auto* castedMsgIdField = static_cast<const SwigEnumField*>(msgIdField);
        auto dslObj = castedMsgIdField->enumDslObj();
        return comms::cppIntTypeFor(dslObj.type(), dslObj.maxLength());
    }

    auto allMessages = m_generator.currentSchema().getAllMessages();
    auto iter = 
        std::max_element(
            allMessages.begin(), allMessages.end(),
            [](auto* first, auto* second)
            {
                return first->dslObj().id() < second->dslObj().id();
            });

    std::string result = "unsigned";
    do {
        if (iter == allMessages.end()) {
            break;
        }

        auto maxId = (*iter)->dslObj().id();
        bool fitsUnsigned = maxId <= std::numeric_limits<unsigned>::max();
        if (fitsUnsigned) {
            break;
        }

        result = "unsigned long long";
    } while (false);

    return result;
}

std::string SwigMsgId::swigIdsInternal() const
{
    auto prefix = swigClassName() + '_';
    auto allMsgIds = m_parent.findMessageIdFields();
    if (allMsgIds.empty() && m_parent.name().empty()) {
        allMsgIds = m_generator.currentSchema().getAllMessageIdFields();
    }

    if (allMsgIds.size() == 1U) {
        auto* msgIdField = allMsgIds.front();    
        assert(msgIdField->dslObj().kind() == commsdsl::parse::ParseField::Kind::Enum);
        auto* castedMsgIdField = static_cast<const SwigEnumField*>(msgIdField);
        auto enumValues = castedMsgIdField->swigEnumValues();
        static const std::string CommentPrefix("// ---");
        for (auto& v : enumValues) {
            auto commentPos = v.find(CommentPrefix);
            if (commentPos != std::string::npos) {
                continue;
            }

            v.insert(v.begin(), prefix.begin(), prefix.end());
        }

        return util::strListToString(enumValues, "\n", "");
    }

    auto allMessages = m_parent.getAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.name().empty()) {
        allMessages = m_generator.currentSchema().getAllMessagesIdSorted();
    }

    util::StringsList ids;
    ids.reserve(allMessages.size());
    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        ids.push_back(prefix + comms::fullNameFor(*m) + " = " + util::numToString(m->dslObj().id()));
    }
    return util::strListToString(ids, ",\n", "");
}

std::string SwigMsgId::swigCodeInternal() const
{
    static const std::string Templ = 
        "const auto #^#CLASS_NAME#$#_#^#NAME#$# = #^#SCOPE#$#_#^#NAME#$#;\n";

    auto scope = comms::scopeForMsgId(strings::msgIdEnumNameStr(), m_generator, m_parent);

    auto allMsgIds = m_parent.findMessageIdFields();
    if (allMsgIds.empty() && m_parent.name().empty()) {
        allMsgIds = m_generator.currentSchema().getAllMessageIdFields();
    }

    if (allMsgIds.size() == 1U) {
        auto* msgIdField = allMsgIds.front();      
        assert(msgIdField->dslObj().kind() == commsdsl::parse::ParseField::Kind::Enum);
        auto* castedMsgIdField = static_cast<const SwigEnumField*>(msgIdField);
        auto enumValues = castedMsgIdField->swigEnumValues();


        static const std::string CommentPrefix("// ---");
        static const std::string EqStr(" = ");
        util::StringsList result;
        for (auto& v : enumValues) {
            auto commentPos = v.find(CommentPrefix);
            if (commentPos != std::string::npos) {
                continue;
            }

            auto eqPos = v.find(EqStr);
            assert(eqPos < v.size());

            util::ReplacementMap repl = {
                {"CLASS_NAME", swigClassName()},
                {"SCOPE", scope},
                {"NAME", v.substr(0, eqPos)}
            };

            result.push_back(util::processTemplate(Templ, repl));
        }

        return util::strListToString(result, "", "");
    }

    auto allMessages = m_parent.getAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.name().empty()) {
        allMessages = m_generator.currentSchema().getAllMessagesIdSorted();
    }

    util::StringsList result;
    result.reserve(allMessages.size());
    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }
                
        util::ReplacementMap repl = {
            {"CLASS_NAME", swigClassName()},
            {"SCOPE", scope},
            {"NAME", comms::fullNameFor(*m)}
        };

        result.push_back(util::processTemplate(Templ, repl));
    }
    return util::strListToString(result, "", "");
}

} // namespace commsdsl2swig
