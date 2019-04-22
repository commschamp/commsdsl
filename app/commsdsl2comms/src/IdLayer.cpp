//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "IdLayer.h"

#include <cassert>

#include "common.h"
#include "Generator.h"

namespace commsdsl2comms
{

void IdLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    static const common::StringsList List = {
        "comms/protocol/MsgIdLayer.h"
    };

    common::mergeIncludes(List, includes);
    common::mergeInclude(generator().headerfileForInput(common::allMessagesStr(), false), includes);
}

std::string IdLayer::getClassDefinitionImpl(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    static_cast<void>(hasInputMessages);
    assert(!hasInputMessages);
    assert(!prevLayer.empty());

    static const std::string Templ =
        "#^#FIELD_DEF#$#\n"
        "#^#PREFIX#$#\n"
        "template <typename TMessage, typename TAllMessages>\n"
        "using #^#CLASS_NAME#$# =\n"
        "    comms::protocol::MsgIdLayer<\n"
        "        #^#FIELD_TYPE#$#,\n"
        "        TMessage,\n"
        "        TAllMessages,\n"
        "        #^#PREV_LAYER#$#,\n"
        "        #^#EXTRA_OPT#$#\n"
        "    >;\n";
    
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("FIELD_DEF", getFieldDefinition(scope)));
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_OPT", getExtraOpt(scope)));
    replacements.insert(std::make_pair("PREV_LAYER", prevLayer));

    prevLayer = common::nameToClassCopy(name());
    hasInputMessages = true;
    return common::processTemplate(Templ, replacements);
}

const std::string& IdLayer::getBareMetalOptionStrImpl() const
{
    static const std::string Str("comms::option::InPlaceAllocation");
    return Str;
}

bool IdLayer::isCustomizableImpl() const
{
    return true;
}

} // namespace commsdsl2comms
