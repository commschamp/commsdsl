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
    common::mergeInclude(generator().mainNamespace() + '/' + common::allMessagesStr() + common::headerSuffix(), includes);
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

bool IdLayer::isCustomizableImpl() const
{
    return true;
}

} // namespace commsdsl2comms
