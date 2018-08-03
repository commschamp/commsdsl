#include "CustomLayer.h"

#include <cassert>

#include "common.h"
#include "Generator.h"

namespace commsdsl2comms
{

void CustomLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    common::mergeInclude(generator().headerfileForCustomLayer(name(), false), includes);
}

std::string CustomLayer::getClassDefinitionImpl(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    static const std::string Templ =
        "#^#FIELD_DEF#$#\n"
        "#^#PREFIX#$#\n"
        "#^#TEMPL_PARAM#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    #^#CUSTOM_LAYER_TYPE#$#<\n"
        "        #^#FIELD_TYPE#$#,\n"
        "        #^#ID_TEMPLATE_PARAMS#$#\n"
        "        #^#PREV_LAYER#$#,\n"
        "        #^#EXTRA_OPT#$#\n"
        "    >;\n";
    
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("FIELD_DEF", getFieldDefinition(scope)));
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PREV_LAYER", prevLayer));
    replacements.insert(std::make_pair("EXTRA_OPT", getExtraOpt(scope)));
    replacements.insert(std::make_pair("CUSTOM_LAYER_TYPE", generator().scopeForCustomLayer(name(), true, true)));

    auto obj = customLayerDslObj();

    static const std::string TemplParam =
        "template <typename TMessage, typename TAllMessages>";

    if (obj.isIdReplacement()) {
        hasInputMessages = true;
        replacements.insert(std::make_pair("ID_TEMPLATE_PARAMS", "TMessage,\nTAllMessages,"));
        replacements.insert(std::make_pair("TEMPL_PARAM", TemplParam));
    }
    else if (hasInputMessages) {
        replacements.insert(std::make_pair("TEMPL_PARAM", TemplParam));
        replacements["PREV_LAYER"] += "<TMessage, TAllMessages>";
    }

    prevLayer = common::nameToClassCopy(name());
    return common::processTemplate(Templ, replacements);
}

} // namespace commsdsl2comms
