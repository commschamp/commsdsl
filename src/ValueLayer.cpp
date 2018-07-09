#include "ValueLayer.h"

#include <cassert>

#include "common.h"
#include "Generator.h"

namespace commsdsl2comms
{

void ValueLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    static const common::StringsList List = {
        "comms/protocol/TransportValueLayer.h"
    };

    common::mergeIncludes(List, includes);
    common::mergeInclude(generator().mainNamespace() + '/' + common::allMessagesStr() + common::headerSuffix(), includes);
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
        replacements.insert(std::make_pair("COMMA", prevLayer));
        replacements.insert(std::make_pair("EXTRA_OPT", "comms::option::PseudoValue"));
    }

    prevLayer = common::nameToClassCopy(name());
    return common::processTemplate(Templ, replacements);
}

} // namespace commsdsl2comms
