//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "MsgId.h"

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "EnumField.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string Template(
    "#^#GEN_COMMENT#$#\n"
    "/// @file\n"
    "/// @brief Contains definition of message ids enumeration.\n\n"
    "#pragma once\n\n"
    "#include <cstdint>\n"
    "#include \"#^#PROT_NAMESPACE#$#/Version.h\"\n\n"
    "#^#BEG_NAMESPACE#$#\n"
    "/// @brief Message ids enumeration.\n"
    "enum #^#ENUM_NAME#$# #^#TYPE#$#\n"
    "{\n"
    "    #^#IDS#$#\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
);

} // namespace

bool MsgId::write(Generator& generator)
{
    MsgId obj(generator);
    return obj.writeDefinition();
}

bool MsgId::writeDefinition() const
{
    auto startInfo = m_generator.startGenericProtocolWrite(common::msgIdEnumNameStr());
    auto& filePath = startInfo.first;
    auto& enumName = startInfo.second;

    if (filePath.empty()) {
        return true;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }


    common::ReplacementMap replacements;
    auto namespaces = m_generator.namespacesForRoot();
    replacements.insert(std::make_pair("GEN_COMMENT", m_generator.fileGeneratedComment()));
    replacements.insert(std::make_pair("BEG_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("ENUM_NAME", std::move(enumName)));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));

    std::string idsStr;
    std::string typeStr;
    auto* msgIdField = m_generator.getMessageIdField();
    if (msgIdField != nullptr) {
        assert(msgIdField->kind() == commsdsl::Field::Kind::Enum);
        auto* castedMsgIdField = static_cast<const EnumField*>(msgIdField);
        auto values = castedMsgIdField->getValuesList();
        auto& prefix = common::msgIdPrefixStr();
        for (auto& v : values) {
            if (!ba::contains(v, "// ---")) {
                v.insert(v.begin(), prefix.begin(), prefix.end());
            }
        }

        idsStr = common::listToString(values, common::emptyString(), common::emptyString());
        typeStr = ": " + castedMsgIdField->underlyingType();
    }
    else {
        auto allMessages = m_generator.getAllMessageIds();
        common::StringsList ids;
        ids.reserve(allMessages.size());
        for (auto& m : allMessages) {
            ids.push_back(m.second + " = " + common::numToString(m.first));
        }

        idsStr = common::listToString(ids, ",\n", common::emptyString());
    }
    replacements.insert(std::make_pair("IDS", std::move(idsStr)));
    replacements.insert(std::make_pair("TYPE", std::move(typeStr)));

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

} // namespace commsdsl2comms
