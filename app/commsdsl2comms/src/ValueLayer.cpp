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

#include "ValueLayer.h"

#include <cassert>

#include "common.h"
#include "Generator.h"

namespace commsdsl2comms
{

bool ValueLayer::prepareImpl()
{
    auto obj = valueLayerDslObj();
    if (obj.pseudo()) {
        setFieldForcedPseudo();
    }

    return true;
}

void ValueLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    static const common::StringsList List = {
        "comms/protocol/TransportValueLayer.h"
    };

    common::mergeIncludes(List, includes);
    common::mergeInclude(generator().headerfileForInput(common::allMessagesStr(), false), includes);
}

std::string ValueLayer::getClassDefinitionImpl(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    static const std::string Templ =
        "#^#FIELD_DEF#$#\n"
        "#^#PREFIX#$#\n"
        "#^#TEMPL_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    comms::protocol::TransportValueLayer<\n"
        "        #^#FIELD_TYPE#$#,\n"
        "        #^#INTERFACE_FIELD_IDX#$#,\n"
        "        #^#PREV_LAYER#$##^#COMMA#$#\n"
        "        #^#EXTRA_OPT#$#\n"
        "    >;\n";
    
    auto obj = valueLayerDslObj();
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("FIELD_DEF", getFieldDefinition(scope)));
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PREV_LAYER", prevLayer));
    replacements.insert(std::make_pair("INTERFACE_FIELD_IDX", common::numToString(obj.fieldIdx())));

    if (hasInputMessages) {
        static const std::string TemplParam =
            "template <typename TMessage, typename TAllMessages>";
        replacements.insert(std::make_pair("TEMPL_PARAM", TemplParam));
        replacements["PREV_LAYER"] += "<TMessage, TAllMessages>";
    }

    if (obj.pseudo()) {
        replacements.insert(std::make_pair("COMMA", ","));
        replacements.insert(std::make_pair("EXTRA_OPT", "comms::option::def::PseudoValue"));
    }

    prevLayer = common::nameToClassCopy(name());
    return common::processTemplate(Templ, replacements);
}

bool ValueLayer::isPseudoVersionLayerImpl(const std::vector<std::string>& interfaceVersionFields) const
{
    auto obj = valueLayerDslObj();
    if (!obj.pseudo()) {
        return false;
    }

    if (memberField()->semanticType() == commsdsl::Field::SemanticType::Version) {
        return true;
    }

    auto iter = std::find(interfaceVersionFields.begin(), interfaceVersionFields.end(), obj.fieldName());
    if (iter != interfaceVersionFields.end()) {
        return true;
    }

    return false;
}

} // namespace commsdsl2comms
