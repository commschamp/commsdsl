#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "Logger.h"
#include "common.h"

namespace bbmp
{

struct XmlWrap
{
    using PropsMap = std::multimap<std::string, std::string>;
    struct CharFree
    {
        void operator()(::xmlChar* p) const
        {
            ::xmlFree(p);
        }
    };

    struct BufferFree
    {
        void operator()(::xmlBuffer* p) const
        {
            ::xmlBufferFree(p);
        }
    };

    using StringPtr = std::unique_ptr<::xmlChar, CharFree>;
    using BufferPtr = std::unique_ptr<::xmlBuffer, BufferFree>;
    using NamesList = std::vector<std::string>;
    using NodesList = std::vector<::xmlNodePtr>;
    using ContentsList = std::vector<std::string>;

    static PropsMap parseNodeProps(::xmlNodePtr node);
    static NodesList getChildren(::xmlNodePtr node, const std::string& name = common::emptyString());
    static std::string getText(::xmlNodePtr node);
    static bool parseNodeValue(
        ::xmlNodePtr node,
        Logger& logger,
        std::string& value);

    static bool parseChildrenAsProps(
        ::xmlNodePtr node,
        const NamesList& names,
        Logger& logger,
        PropsMap& props);

    static PropsMap getUnknownProps(::xmlNodePtr node, const NamesList& names);
    static ContentsList getUnknownChildren(::xmlNodePtr node, const NamesList& names);
    static std::string logPrefix(::xmlNodePtr node);
};

} // namespace bbmp
