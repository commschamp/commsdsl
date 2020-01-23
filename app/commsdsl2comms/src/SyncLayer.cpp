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

#include "SyncLayer.h"

#include <cassert>

#include "common.h"
#include "Generator.h"

namespace commsdsl2comms
{

bool SyncLayer::prepareImpl()
{
    setFieldForcedFailOnInvalid();
    return true;
}

void SyncLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    static const common::StringsList List = {
        "comms/protocol/SyncPrefixLayer.h"
    };

    common::mergeIncludes(List, includes);
}

std::string SyncLayer::getClassDefinitionImpl(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    static const std::string Templ =
        "#^#FIELD_DEF#$#\n"
        "#^#PREFIX#$#\n"
        "#^#TEMPL_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    comms::protocol::SyncPrefixLayer<\n"
        "        #^#FIELD_TYPE#$#,\n"
        "        #^#PREV_LAYER#$#\n"
        "    >;\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("FIELD_DEF", getFieldDefinition(scope)));
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PREV_LAYER", prevLayer));
    if (hasInputMessages) {
        static const std::string TemplParam =
            "template <typename TMessage, typename TAllMessages>";
        replacements.insert(std::make_pair("TEMPL_PARAM", TemplParam));
        replacements["PREV_LAYER"] += "<TMessage, TAllMessages>";
    }

    prevLayer = common::nameToClassCopy(name());
    return common::processTemplate(Templ, replacements);
}

} // namespace commsdsl2comms
