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

class ProtocolImpl;
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

    static const NamesList& emptyNamesList();
    static PropsMap parseNodeProps(::xmlNodePtr node);
    static NodesList getChildren(::xmlNodePtr node, const std::string& name = common::emptyString());
    static NodesList getChildren(::xmlNodePtr node, const NamesList& names);
    static std::string getText(::xmlNodePtr node);
    static bool parseNodeValue(
        ::xmlNodePtr node,
        Logger& logger,
        std::string& value,
        bool mustHaveValue = true);

    static bool parseChildrenAsProps(
        ::xmlNodePtr node,
        const NamesList& names,
        Logger& logger,
        PropsMap& props,
        bool mustHaveValues = true);

    static PropsMap getUnknownProps(::xmlNodePtr node, const NamesList& names);
    static NodesList getUnknownChildren(::xmlNodePtr node, const NamesList& names);
    static std::string getElementContent(::xmlNodePtr node);
    static ContentsList getUnknownChildrenContents(::xmlNodePtr node, const NamesList& names);
    static std::string logPrefix(::xmlNodePtr node);
    static bool validateSinglePropInstance(
        ::xmlNodePtr node,
        const PropsMap& props,
        const std::string& str,
        Logger& logger,
        bool mustHave = false);

    static bool validateNoPropInstance(
        ::xmlNodePtr node,
        const PropsMap& props,
        const std::string& str,
        Logger& logger);

    static bool hasAnyChild(::xmlNodePtr node, const NamesList& names);

    static void reportUnexpectedPropertyValue(
        ::xmlNodePtr node,
        const std::string& elemName,
        const std::string& propName,
        const std::string& propValue,
        Logger& logger);

    static bool checkVersions(
        ::xmlNodePtr node,
        unsigned sinceVersion,
        unsigned deprecatedSince,
        ProtocolImpl& protocol,
        unsigned parentVersion,
        unsigned parentDeprecated);

    static bool getAndCheckVersions(
        ::xmlNodePtr node,
        const std::string& name,
        const PropsMap& props,
        unsigned& sinceVersion,
        unsigned& deprecatedSince,
        ProtocolImpl& protocol);

    static bool getAndCheckVersions(
        ::xmlNodePtr node,
        const std::string& name,
        unsigned& sinceVersion,
        unsigned& deprecatedSince,
        ProtocolImpl& protocol);
};

} // namespace bbmp
