//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "ParseLogger.h"
#include "parse_common.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
struct ParseXmlWrap
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

    static const NamesList& parseEmptyNamesList();
    static PropsMap parseNodeProps(::xmlNodePtr node);
    static NodesList parseGetChildren(::xmlNodePtr node, const std::string& name = common::parseEmptyString(), bool skipValueAttr = false);
    static NodesList parseGetChildren(::xmlNodePtr node, const NamesList& names, bool skipValueAttr = false);
    static std::string parseGetText(::xmlNodePtr node);
    static bool parseNodeValue(
        ::xmlNodePtr node,
        ParseLogger& logger,
        std::string& value,
        bool mustHaveValue = true);

    static bool parseChildrenAsProps(
        ::xmlNodePtr node,
        const NamesList& names,
        ParseLogger& logger,
        PropsMap& props,
        bool mustHaveValues = true);

    static PropsMap parseGetUnknownProps(::xmlNodePtr node, const NamesList& names);
    static NodesList parseGetUnknownChildren(::xmlNodePtr node, const NamesList& names);
    static std::string parseGetElementContent(::xmlNodePtr node);
    static ContentsList parseGetUnknownChildrenContents(::xmlNodePtr node, const NamesList& names);
    static std::string parseLogPrefix(::xmlNodePtr node);
    static bool parseValidateSinglePropInstance(
        ::xmlNodePtr node,
        const PropsMap& props,
        const std::string& str,
        ParseLogger& logger,
        bool mustHave = false);

    static bool parseValidateNoPropInstance(
        ::xmlNodePtr node,
        const PropsMap& props,
        const std::string& str,
        ParseLogger& logger);

    static bool parseHasAnyChild(::xmlNodePtr node, const NamesList& names);

    static void parseReportUnexpectedPropertyValue(
        ::xmlNodePtr node,
        const std::string& elemName,
        const std::string& propName,
        const std::string& propValue,
        ParseLogger& logger);

    static bool parseCheckVersions(
        ::xmlNodePtr node,
        unsigned sinceVersion,
        unsigned deprecatedSince,
        ParseProtocolImpl& protocol,
        unsigned parentVersion,
        unsigned parentDeprecated);

    static bool parseGetAndCheckVersions(
        ::xmlNodePtr node,
        const std::string& name,
        const PropsMap& props,
        unsigned& sinceVersion,
        unsigned& deprecatedSince,
        ParseProtocolImpl& protocol);

    static bool parseGetAndCheckVersions(
        ::xmlNodePtr node,
        const std::string& name,
        unsigned& sinceVersion,
        unsigned& deprecatedSince,
        ParseProtocolImpl& protocol);

    static PropsMap parseGetExtraAttributes(
        ::xmlNodePtr node,
        const ParseXmlWrap::NamesList& names,
        ParseProtocolImpl& protocol);

    static ContentsList parseGetExtraChildren(
        ::xmlNodePtr node,
        const ParseXmlWrap::NamesList& names,
        ParseProtocolImpl& protocol);
};

} // namespace parse

} // namespace commsdsl
