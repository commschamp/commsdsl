//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "ParseLogger.h"
#include "parse_common.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
struct ParseXmlWrap
{
    using ParsePropsMap = std::multimap<std::string, std::string>;
    struct ParseCharFree
    {
        void operator()(::xmlChar* p) const
        {
            ::xmlFree(p);
        }
    };

    struct ParseBufferFree
    {
        void operator()(::xmlBuffer* p) const
        {
            ::xmlBufferFree(p);
        }
    };

    using ParseStringPtr = std::unique_ptr<::xmlChar, ParseCharFree>;
    using ParseBufferPtr = std::unique_ptr<::xmlBuffer, ParseBufferFree>;
    using ParseNamesList = std::vector<std::string>;
    using ParseNodesList = std::vector<::xmlNodePtr>;
    using ParseContentsList = std::vector<std::string>;

    [[nodiscard]] static const ParseNamesList& parseEmptyNamesList();
    [[nodiscard]] static ParsePropsMap parseNodeProps(::xmlNodePtr node);
    [[nodiscard]] static ParseNodesList parseGetChildren(::xmlNodePtr node, const std::string& name = common::parseEmptyString(), bool skipValueAttr = false);
    [[nodiscard]] static ParseNodesList parseGetChildren(::xmlNodePtr node, const ParseNamesList& names, bool skipValueAttr = false);
    [[nodiscard]] static std::string parseGetText(::xmlNodePtr node);
    [[nodiscard]] static bool parseNodeValue(
        ::xmlNodePtr node,
        ParseLogger& logger,
        std::string& value,
        bool mustHaveValue = true);

    [[nodiscard]] static bool parseChildrenAsProps(
        ::xmlNodePtr node,
        const ParseNamesList& names,
        ParseLogger& logger,
        ParsePropsMap& props,
        bool mustHaveValues = true);

    [[nodiscard]] static ParsePropsMap parseGetUnknownProps(::xmlNodePtr node, const ParseNamesList& names);
    [[nodiscard]] static ParseNodesList parseGetUnknownChildren(::xmlNodePtr node, const ParseNamesList& names);
    [[nodiscard]] static std::string parseGetElementContent(::xmlNodePtr node);
    [[nodiscard]] static ParseContentsList parseGetUnknownChildrenContents(::xmlNodePtr node, const ParseNamesList& names);
    [[nodiscard]] static std::string parseLogPrefix(::xmlNodePtr node);
    [[nodiscard]] static bool parseValidateSinglePropInstance(
        ::xmlNodePtr node,
        const ParsePropsMap& props,
        const std::string& str,
        ParseLogger& logger,
        bool mustHave = false);

    [[nodiscard]] static bool parseValidateNoPropInstance(
        ::xmlNodePtr node,
        const ParsePropsMap& props,
        const std::string& str,
        ParseLogger& logger);

    [[nodiscard]] static bool parseHasAnyChild(::xmlNodePtr node, const ParseNamesList& names);

    static void parseReportUnexpectedPropertyValue(
        ::xmlNodePtr node,
        const std::string& elemName,
        const std::string& propName,
        const std::string& propValue,
        ParseLogger& logger);

    [[nodiscard]] static bool parseCheckVersions(
        ::xmlNodePtr node,
        unsigned sinceVersion,
        unsigned deprecatedSince,
        ParseProtocolImpl& protocol,
        unsigned parentVersion,
        unsigned parentDeprecated);

    [[nodiscard]] static bool parseGetAndCheckVersions(
        ::xmlNodePtr node,
        const std::string& name,
        const ParsePropsMap& props,
        unsigned& sinceVersion,
        unsigned& deprecatedSince,
        ParseProtocolImpl& protocol);

    [[nodiscard]] static bool parseGetAndCheckVersions(
        ::xmlNodePtr node,
        const std::string& name,
        unsigned& sinceVersion,
        unsigned& deprecatedSince,
        ParseProtocolImpl& protocol);

    [[nodiscard]] static ParsePropsMap parseGetExtraAttributes(
        ::xmlNodePtr node,
        const ParseXmlWrap::ParseNamesList& names,
        ParseProtocolImpl& protocol);

    [[nodiscard]] static ParseContentsList parseGetExtraChildren(
        ::xmlNodePtr node,
        const ParseXmlWrap::ParseNamesList& names,
        ParseProtocolImpl& protocol);
};

} // namespace parse

} // namespace commsdsl
