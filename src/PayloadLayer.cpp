#include "PayloadLayer.h"

#include "common.h"

namespace commsdsl2comms
{

void PayloadLayer::updateIncludesImpl(Layer::IncludesList& includes) const
{
    static const common::StringsList List = {
        "comms::protocol::MsgDataLayer.h"
    };

    common::mergeIncludes(List, includes);
}

std::string PayloadLayer::getClassDefinitionImpl(const std::string& scope) const
{
    static_cast<void>(scope);
    static const std::string Templ =
        "#^#PREFIX#$#\n"
        "using #^#CLASS_NAME#$# =\n"
        "   comms::protocol::MsgDataLayer<\n"
        "       #^#EXTRA_OPT#$#\n"
        "   >;";
    
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getPrefix()));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_OPT", getExtraOpt(scope)));
    return common::processTemplate(Templ, replacements);
}

std::string PayloadLayer::getExtraOpt(const std::string& scope) const
{
    return scope + common::nameToClassCopy(name());
}

} // namespace commsdsl2comms
