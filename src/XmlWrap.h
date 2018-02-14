#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "Logger.h"

namespace bbmp
{

struct XmlWrap
{
    using PropsMap = std::map<std::string, std::string>;
    struct CharFree
    {
        void operator()(::xmlChar* p) const
        {
            ::xmlFree(p);
        }
    };

    using StringPtr = std::unique_ptr<::xmlChar, CharFree>;
    using NamesList = std::vector<std::string>;
    using NodesList = std::vector<::xmlNodePtr>;

    static PropsMap parseNodeProps(::xmlNodePtr node);
    static NodesList getChildren(::xmlNodePtr node, const std::string& name = std::string());
    static std::string getText(::xmlNodePtr node);
    static bool parseChildrenAsProps(
        ::xmlNodePtr node,
        const NamesList& names,
        Logger& logger,
        PropsMap& props);
};

} // namespace bbmp
