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
