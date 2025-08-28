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

#include "ParseXmlWrap.h"

#include "ParseProtocolImpl.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace parse
{

const ParseXmlWrap::ParseNamesList& ParseXmlWrap::parseEmptyNamesList()
{
    static const ParseNamesList List;
    return List;
}

ParseXmlWrap::ParsePropsMap ParseXmlWrap::parseNodeProps(::xmlNodePtr node)
{
    assert(node != nullptr);
    ParsePropsMap map;
    auto* prop = node->properties;
    while (prop != nullptr) {
        ParseStringPtr valuePtr(::xmlNodeListGetString(node->doc, prop->children, 1));
        auto iter = map.insert(
            std::make_pair(
                reinterpret_cast<const char*>(prop->name),
                reinterpret_cast<const char*>(valuePtr.get())));
        common::parseRemoveHeadingTrailingWhitespaces(iter->second);
        prop = prop->next;
    }

    return map;
}

ParseXmlWrap::ParseNodesList ParseXmlWrap::parseGetChildren(::xmlNodePtr node, const std::string& name, bool skipValueAttr)
{
    ParseNamesList names;
    if (!name.empty()) {
        names.push_back(name);
    }
    return parseGetChildren(node, names, skipValueAttr);
}

ParseXmlWrap::ParseNodesList ParseXmlWrap::parseGetChildren(::xmlNodePtr node, const ParseNamesList& names, bool skipValueAttr)
{
    ParseNodesList result;
    auto* cur = node->children;
    while (cur != nullptr) {
        do {
            if (cur->type != XML_ELEMENT_NODE) {
                break;
            }

            if (names.empty()) {
                result.push_back(cur);
                break;
            }

            if (skipValueAttr) {
                auto props = parseNodeProps(cur);
                static const std::string ValueAttr("value");
                if (props.find(ValueAttr) != props.end()) {
                    // Skip one with the value attribute
                    break;
                }            
            }

            std::string elemName(reinterpret_cast<const char*>(cur->name));
            auto iter = std::find(names.begin(), names.end(), elemName);
            if (iter == names.end()) {
                break;
            }

            result.push_back(cur);
        } while (false);

        cur = cur->next;
    }
    return result;
}

std::string ParseXmlWrap::parseGetText(::xmlNodePtr node)
{
    assert(node != nullptr);
    auto* child = node->children;
    while (child != nullptr) {
        if (child->type == XML_ELEMENT_NODE) {
            return common::parseEmptyString();
        }

        child = child->next;
    }

    child = node->children;
    while (child != nullptr) {
        if (child->type == XML_TEXT_NODE) {
            ParseStringPtr valuePtr(::xmlNodeGetContent(child));
            return std::string(reinterpret_cast<const char*>(valuePtr.get()));
        }
        child = child->next;
    }
    return std::string();
}

bool ParseXmlWrap::parseNodeValue(
    ::xmlNodePtr node,
    ParseLogger& logger,
    std::string& value,
    bool mustHaveValue)
{
    auto props = parseNodeProps(node);
    static const std::string ValueAttr("value");
    auto valIter = props.find(ValueAttr);
    std::string valueTmp;
    if (valIter != props.end()) {
        valueTmp = valIter->second;
        common::parseRemoveHeadingTrailingWhitespaces(valueTmp);
    }

    auto text = parseGetText(node);
    common::parseRemoveHeadingTrailingWhitespaces(text);
    if (valueTmp.empty() && text.empty()) {
        if (!mustHaveValue) {
            return true;
        }

        parseLogError(logger) << parseLogPrefix(node) <<
            "No value for \"" << node->name << "\" element.";
        return false;
    }

    if ((!valueTmp.empty()) && (!text.empty())) {
        parseLogError(logger) << parseLogPrefix(node) <<
            ": Incorrect value format for \"" << node->name << "\" element.";
        return false;
    }

    if (!valueTmp.empty()) {
        value = std::move(valueTmp);
        return true;
    }

    assert(!text.empty());
    common::parseNormaliseString(text);
    value = std::move(text);
    return true;
}


bool ParseXmlWrap::parseChildrenAsProps(
    ::xmlNodePtr node,
    const ParseNamesList& names,
    ParseLogger& logger,
    ParsePropsMap& result,
    bool mustHaveValue)
{
    auto children = parseGetChildren(node);
    for (auto* c : children) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = std::find(names.begin(), names.end(), cName);
        if (iter == names.end()) {
            continue;
        }

        std::string value;
        if ((!parseNodeValue(c, logger, value, mustHaveValue)) && (mustHaveValue)) {
            return false;
        }

        if (value.empty()) {
            continue;
        }

        result.insert(std::make_pair(cName, std::move(value)));
    }

    return true;
}

ParseXmlWrap::ParsePropsMap ParseXmlWrap::parseGetUnknownProps(::xmlNodePtr node, const ParseXmlWrap::ParseNamesList& names)
{
    auto props = parseNodeProps(node);
    for (auto& n : names) {
        auto iters = props.equal_range(n);
        if (iters.first == iters.second) {
            continue;
        }

        props.erase(iters.first, iters.second);
    }
    return props;
}

ParseXmlWrap::ParseNodesList ParseXmlWrap::parseGetUnknownChildren(::xmlNodePtr node, const ParseXmlWrap::ParseNamesList& names)
{
    ParseNodesList result;
    auto children = parseGetChildren(node);
    for (auto* c : children) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = std::find(names.begin(), names.end(), cName);
        if (iter == names.end()) {
            result.push_back(c);
        }
    }
    return result;
}

std::string ParseXmlWrap::parseGetElementContent(::xmlNodePtr node)
{
    std::string result;
    ParseBufferPtr buf(::xmlBufferCreate());
    auto bufLen = ::xmlNodeDump(buf.get(), node->doc, node, 0, 0);
    if (0 < bufLen) {
        result = reinterpret_cast<const char*>(::xmlBufferContent(buf.get()));
    }
    return result;
}

ParseXmlWrap::ParseContentsList ParseXmlWrap::parseGetUnknownChildrenContents(::xmlNodePtr node, const ParseXmlWrap::ParseNamesList& names)
{
    ParseContentsList result;
    auto children = parseGetUnknownChildren(node, names);
    for (auto* c : children) {
        ParseBufferPtr buf(::xmlBufferCreate());
        auto bufLen = ::xmlNodeDump(buf.get(), c->doc, c, 0, 0);
        if (bufLen == 0U) {
            continue;
        }

        result.emplace_back(reinterpret_cast<const char*>(::xmlBufferContent(buf.get())));
    }
    return result;
}

std::string ParseXmlWrap::parseLogPrefix(::xmlNodePtr node)
{
    assert(node != nullptr);
    assert(node->doc != nullptr);
    assert(node->doc->URL != nullptr);
    return std::string(reinterpret_cast<const char*>(node->doc->URL)) + ":" + std::to_string(node->line) + ": ";
}

bool ParseXmlWrap::parseValidateSinglePropInstance(
    ::xmlNodePtr node,
    const ParsePropsMap& props,
    const std::string& str,
    ParseLogger& logger,
    bool mustHave)
{
    auto count = props.count(str);
    if (1U < count) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                      "Too many values of \"" << str << "\" property for \"" << node->name << "\" element.";
        return false;
    }

    if ((count == 0U) && mustHave) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                      "Missing value for mandatory property \"" << str << "\" for \"" << node->name << "\" element.";
        return false;
    }

    return true;
}

bool ParseXmlWrap::parseValidateNoPropInstance(::xmlNodePtr node, const ParseXmlWrap::ParsePropsMap& props, const std::string& str, ParseLogger& logger)
{
    auto iter = props.find(str);
    if (iter != props.end()) {
        parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                      "Preperty \"" << str << "\" defined when should not.";
        return false;
    }

    return true;
}

bool ParseXmlWrap::parseHasAnyChild(::xmlNodePtr node, const ParseXmlWrap::ParseNamesList& names)
{
    ParseContentsList result;
    auto children = parseGetChildren(node);
    for (auto* c : children) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = std::find(names.begin(), names.end(), cName);
        if (iter != names.end()) {
            return true;
        }
    }
    return false;
}

void ParseXmlWrap::parseReportUnexpectedPropertyValue(
    ::xmlNodePtr node,
    const std::string& elemName,
    const std::string& propName,
    const std::string& propValue,
    ParseLogger& logger)
{
    parseLogError(logger) << ParseXmlWrap::parseLogPrefix(node) <<
                  "Property \"" << propName << "\" of element \"" << elemName <<
                              "\" has unexpected value (" << propValue << ").";
}

bool ParseXmlWrap::parseCheckVersions(
    ::xmlNodePtr node,
    unsigned sinceVersion,
    unsigned deprecatedSince,
    ParseProtocolImpl& protocol,
    unsigned parentVersion,
    unsigned parentDeprecated)
{
    assert(parentVersion < parentDeprecated);
    if (protocol.parseCurrSchema().parseVersion() < sinceVersion) {
        parseLogError(protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(node) <<
            "The value of \"" << common::parseSinceVersionStr() << "\" property (" << sinceVersion << ") cannot "
            "be greater than value of \"" << common::parseVersionStr() << "\" property of the schema (" << protocol.parseCurrSchema().parseVersion() << ").";
        return false;
    }

    if (sinceVersion < parentVersion) {
        parseLogError(protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(node) <<
            "The value of \"" << common::parseSinceVersionStr() << "\" property (" << sinceVersion << ") cannot "
            "be less than " << parentVersion << ".";
        return false;
    }

    if (parentDeprecated <= sinceVersion) {
        parseLogError(protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(node) <<
            "The value of \"" << common::parseSinceVersionStr() << "\" property (" << sinceVersion << ") must "
            "be less than " << parentDeprecated << ".";
        return false;
    }

    if (parentDeprecated < deprecatedSince) {
        parseLogError(protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(node) <<
            "The value of \"" << common::parseDeprecatedStr() << "\" property (" << deprecatedSince << ") cannot "
            "be greater than " << parentDeprecated << ".";
        return false;
    }


    if (deprecatedSince <= sinceVersion) {
        parseLogError(protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(node) <<
            "The value of \"" << common::parseDeprecatedStr() << "\" property (" << deprecatedSince << ") must "
            "be greater than value of \"" << common::parseSinceVersionStr() << "\" property (" << sinceVersion << ").";
        return false;
    }

    if ((deprecatedSince < commsdsl::parse::ParseProtocol::parseNotYetDeprecated()) &&
        (protocol.parseCurrSchema().parseVersion() < deprecatedSince)) {
        parseLogError(protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(node) <<
            "The value of \"" << common::parseDeprecatedStr() << "\" property (" << deprecatedSince << ") cannot "
            "be greater than value of \"" << common::parseVersionStr() << "\" property of the schema (" << protocol.parseCurrSchema().parseVersion() << ").";
        return false;
    }

    return true;
}

bool ParseXmlWrap::parseGetAndCheckVersions(
    ::xmlNodePtr node,
    const std::string& name,
    const ParsePropsMap& props,
    unsigned& sinceVersion,
    unsigned& deprecatedSince,
    ParseProtocolImpl& protocol)
{
    auto parentVersion = sinceVersion;
    auto parentDeprecated = deprecatedSince;
    auto sinceVerIter = props.find(common::parseSinceVersionStr());
    do {
        if (sinceVerIter == props.end()) {
            assert(sinceVersion <= protocol.parseCurrSchema().parseVersion());
            break;
        }

        auto& sinceVerStr = sinceVerIter->second;
        bool ok = false;
        sinceVersion = common::parseStrToUnsigned(sinceVerStr, &ok);
        if (!ok) {
            parseReportUnexpectedPropertyValue(node, name, common::parseSinceVersionStr(), sinceVerStr, protocol.parseLogger());
            return false;
        }

    } while (false);

    auto deprecatedIter = props.find(common::parseDeprecatedStr());
    do {
        if (deprecatedIter == props.end()) {
            break;
        }

        auto& deprecatedStr = deprecatedIter->second;
        bool ok = false;
        deprecatedSince = common::parseStrToUnsigned(deprecatedStr, &ok);
        if (!ok) {
            ParseXmlWrap::parseReportUnexpectedPropertyValue(node, name, common::parseDeprecatedStr(), deprecatedStr, protocol.parseLogger());
            return false;
        }

    } while (false);

    if (!parseCheckVersions(node, sinceVersion, deprecatedSince, protocol, parentVersion, parentDeprecated)) {
        return false;
    }

    return true;
}

bool ParseXmlWrap::parseGetAndCheckVersions(
    ::xmlNodePtr node,
    const std::string& name,
    unsigned& sinceVersion,
    unsigned& deprecatedSince,
    ParseProtocolImpl& protocol)
{
    auto props = parseNodeProps(node);
    static const ParseNamesList Names = {
        common::parseSinceVersionStr(),
        common::parseDeprecatedStr()
    };

    if (!parseChildrenAsProps(node, Names, protocol.parseLogger(), props)) {
        return false;
    }

    return parseGetAndCheckVersions(node, name, props, sinceVersion, deprecatedSince, protocol);
}

ParseXmlWrap::ParsePropsMap ParseXmlWrap::parseGetExtraAttributes(::xmlNodePtr node, const ParseXmlWrap::ParseNamesList& names, ParseProtocolImpl& protocol)
{
    ParsePropsMap attrs = ParseXmlWrap::parseGetUnknownProps(node, names);
    auto& expectedPrefixes = protocol.parseExtraElementPrefixes();
    for (auto& a : attrs) {
        bool expected =
            std::any_of(
                expectedPrefixes.begin(), expectedPrefixes.end(),
                [&a](const std::string& prefix)
                {
                    if (a.first.size() < prefix.size()) {
                        return false;
                    }

                    return (a.first.compare(0, prefix.size(), prefix) == 0);
                });

        if (!expected) {
            parseLogWarning(protocol.parseLogger()) << parseLogPrefix(node) <<
                "Unexpected attribute \"" << a.first << "\".";
        }
    }

    return attrs;
}

ParseXmlWrap::ParseContentsList ParseXmlWrap::parseGetExtraChildren(::xmlNodePtr node, const ParseXmlWrap::ParseNamesList& names, ParseProtocolImpl& protocol)
{
    ParseContentsList result;
    auto extraChildren = ParseXmlWrap::parseGetUnknownChildren(node, names);
    auto& expectedPrefixes = protocol.parseExtraElementPrefixes();
    for (auto c : extraChildren) {
        std::string name(reinterpret_cast<const char*>(c->name));
        bool expected =
            std::any_of(
                expectedPrefixes.begin(), expectedPrefixes.end(),
                [&name](const std::string& prefix)
                {
                    if (name.size() < prefix.size()) {
                        return false;
                    }

                    return (name.compare(0, prefix.size(), prefix) == 0);
                });

        if (!expected) {
            parseLogWarning(protocol.parseLogger()) << parseLogPrefix(c) <<
                "Unexpected element \"" << name << "\".";
        }
        result.push_back(ParseXmlWrap::parseGetElementContent(c));
    }
    return result;
}

} // namespace parse

} // namespace commsdsl
