#include "PayloadLayer.h"

#include <cassert>

#include "common.h"

namespace commsdsl2comms
{

void PayloadLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    static const common::StringsList List = {
        "comms/protocol/MsgDataLayer.h"
    };

    common::mergeIncludes(List, includes);
}

std::string PayloadLayer::getClassDefinitionImpl(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    static_cast<void>(hasInputMessages);
    assert(!hasInputMessages);
    assert(prevLayer.empty());
    prevLayer = common::nameToClassCopy(name());

    static const std::string Templ =
        "#^#PREFIX#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "    comms::protocol::MsgDataLayer<\n"
        "        #^#EXTRA_OPT#$#\n"
        "    >;\n";
    
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_OPT", getExtraOpt(scope)));
    return common::processTemplate(Templ, replacements);
}

std::string PayloadLayer::getExtraOpt(const std::string& scope) const
{
    return "typename " + scope + common::nameToClassCopy(name());
}

} // namespace commsdsl2comms
