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

#include <cassert>
#include <algorithm>

#include "ParseProtocolImpl.h"

namespace commsdsl
{

namespace parse
{

const ParseXmlWrap::NamesList& ParseXmlWrap::emptyNamesList()
{
    static const NamesList List;
    return List;
}

ParseXmlWrap::PropsMap ParseXmlWrap::parseNodeProps(::xmlNodePtr node)
{
    assert(node != nullptr);
    PropsMap map;
    auto* prop = node->properties;
    while (prop != nullptr) {
        StringPtr valuePtr(::xmlNodeListGetString(node->doc, prop->children, 1));
        auto iter = map.insert(
            std::make_pair(
                reinterpret_cast<const char*>(prop->name),
                reinterpret_cast<const char*>(valuePtr.get())));
        common::removeHeadingTrailingWhitespaces(iter->second);
        prop = prop->next;
    }

    return map;
}

ParseXmlWrap::NodesList ParseXmlWrap::getChildren(::xmlNodePtr node, const std::string& name, bool skipValueAttr)
{
    NamesList names;
    if (!name.empty()) {
        names.push_back(name);
    }
    return getChildren(node, names, skipValueAttr);
}

ParseXmlWrap::NodesList ParseXmlWrap::getChildren(::xmlNodePtr node, const NamesList& names, bool skipValueAttr)
{
    NodesList result;
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

std::string ParseXmlWrap::getText(::xmlNodePtr node)
{
    assert(node != nullptr);
    auto* child = node->children;
    while (child != nullptr) {
        if (child->type == XML_ELEMENT_NODE) {
            return common::emptyString();
        }

        child = child->next;
    }

    child = node->children;
    while (child != nullptr) {
        if (child->type == XML_TEXT_NODE) {
            StringPtr valuePtr(::xmlNodeGetContent(child));
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
        common::removeHeadingTrailingWhitespaces(valueTmp);
    }

    auto text = getText(node);
    common::removeHeadingTrailingWhitespaces(text);
    if (valueTmp.empty() && text.empty()) {
        if (!mustHaveValue) {
            return true;
        }

        logError(logger) << logPrefix(node) <<
            "No value for \"" << node->name << "\" element.";
        return false;
    }

    if ((!valueTmp.empty()) && (!text.empty())) {
        logError(logger) << logPrefix(node) <<
            ": Incorrect value format for \"" << node->name << "\" element.";
        return false;
    }

    if (!valueTmp.empty()) {
        value = std::move(valueTmp);
        return true;
    }

    assert(!text.empty());
    common::normaliseString(text);
    value = std::move(text);
    return true;
}


bool ParseXmlWrap::parseChildrenAsProps(
    ::xmlNodePtr node,
    const NamesList& names,
    ParseLogger& logger,
    PropsMap& result,
    bool mustHaveValue)
{
    auto children = getChildren(node);
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

ParseXmlWrap::PropsMap ParseXmlWrap::getUnknownProps(::xmlNodePtr node, const ParseXmlWrap::NamesList& names)
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

ParseXmlWrap::NodesList ParseXmlWrap::getUnknownChildren(::xmlNodePtr node, const ParseXmlWrap::NamesList& names)
{
    NodesList result;
    auto children = getChildren(node);
    for (auto* c : children) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = std::find(names.begin(), names.end(), cName);
        if (iter == names.end()) {
            result.push_back(c);
        }
    }
    return result;
}

std::string ParseXmlWrap::getElementContent(::xmlNodePtr node)
{
    std::string result;
    BufferPtr buf(::xmlBufferCreate());
    auto bufLen = ::xmlNodeDump(buf.get(), node->doc, node, 0, 0);
    if (0 < bufLen) {
        result = reinterpret_cast<const char*>(::xmlBufferContent(buf.get()));
    }
    return result;
}

ParseXmlWrap::ContentsList ParseXmlWrap::getUnknownChildrenContents(::xmlNodePtr node, const ParseXmlWrap::NamesList& names)
{
    ContentsList result;
    auto children = getUnknownChildren(node, names);
    for (auto* c : children) {
        BufferPtr buf(::xmlBufferCreate());
        auto bufLen = ::xmlNodeDump(buf.get(), c->doc, c, 0, 0);
        if (bufLen == 0U) {
            continue;
        }

        result.emplace_back(reinterpret_cast<const char*>(::xmlBufferContent(buf.get())));
    }
    return result;
}

std::string ParseXmlWrap::logPrefix(::xmlNodePtr node)
{
    assert(node != nullptr);
    assert(node->doc != nullptr);
    assert(node->doc->URL != nullptr);
    return std::string(reinterpret_cast<const char*>(node->doc->URL)) + ":" + std::to_string(node->line) + ": ";
}

bool ParseXmlWrap::validateSinglePropInstance(
    ::xmlNodePtr node,
    const PropsMap& props,
    const std::string& str,
    ParseLogger& logger,
    bool mustHave)
{
    auto count = props.count(str);
    if (1U < count) {
        logError(logger) << ParseXmlWrap::logPrefix(node) <<
                      "Too many values of \"" << str << "\" property for \"" << node->name << "\" element.";
        return false;
    }

    if ((count == 0U) && mustHave) {
        logError(logger) << ParseXmlWrap::logPrefix(node) <<
                      "Missing value for mandatory property \"" << str << "\" for \"" << node->name << "\" element.";
        return false;
    }

    return true;
}

bool ParseXmlWrap::validateNoPropInstance(::xmlNodePtr node, const ParseXmlWrap::PropsMap& props, const std::string& str, ParseLogger& logger)
{
    auto iter = props.find(str);
    if (iter != props.end()) {
        logError(logger) << ParseXmlWrap::logPrefix(node) <<
                      "Preperty \"" << str << "\" defined when should not.";
        return false;
    }

    return true;
}

bool ParseXmlWrap::hasAnyChild(::xmlNodePtr node, const ParseXmlWrap::NamesList& names)
{
    ContentsList result;
    auto children = getChildren(node);
    for (auto* c : children) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = std::find(names.begin(), names.end(), cName);
        if (iter != names.end()) {
            return true;
        }
    }
    return false;
}

void ParseXmlWrap::reportUnexpectedPropertyValue(
    ::xmlNodePtr node,
    const std::string& elemName,
    const std::string& propName,
    const std::string& propValue,
    ParseLogger& logger)
{
    logError(logger) << ParseXmlWrap::logPrefix(node) <<
                  "Property \"" << propName << "\" of element \"" << elemName <<
                              "\" has unexpected value (" << propValue << ").";
}

bool ParseXmlWrap::checkVersions(
    ::xmlNodePtr node,
    unsigned sinceVersion,
    unsigned deprecatedSince,
    ParseProtocolImpl& protocol,
    unsigned parentVersion,
    unsigned parentDeprecated)
{
    assert(parentVersion < parentDeprecated);
    if (protocol.currSchema().version() < sinceVersion) {
        logError(protocol.logger()) << ParseXmlWrap::logPrefix(node) <<
            "The value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ") cannot "
            "be greater than value of \"" << common::versionStr() << "\" property of the schema (" << protocol.currSchema().version() << ").";
        return false;
    }

    if (sinceVersion < parentVersion) {
        logError(protocol.logger()) << ParseXmlWrap::logPrefix(node) <<
            "The value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ") cannot "
            "be less than " << parentVersion << ".";
        return false;
    }

    if (parentDeprecated <= sinceVersion) {
        logError(protocol.logger()) << ParseXmlWrap::logPrefix(node) <<
            "The value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ") must "
            "be less than " << parentDeprecated << ".";
        return false;
    }

    if (parentDeprecated < deprecatedSince) {
        logError(protocol.logger()) << ParseXmlWrap::logPrefix(node) <<
            "The value of \"" << common::deprecatedStr() << "\" property (" << deprecatedSince << ") cannot "
            "be greater than " << parentDeprecated << ".";
        return false;
    }


    if (deprecatedSince <= sinceVersion) {
        logError(protocol.logger()) << ParseXmlWrap::logPrefix(node) <<
            "The value of \"" << common::deprecatedStr() << "\" property (" << deprecatedSince << ") must "
            "be greater than value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ").";
        return false;
    }

    if ((deprecatedSince < commsdsl::parse::ParseProtocol::notYetDeprecated()) &&
        (protocol.currSchema().version() < deprecatedSince)) {
        logError(protocol.logger()) << ParseXmlWrap::logPrefix(node) <<
            "The value of \"" << common::deprecatedStr() << "\" property (" << deprecatedSince << ") cannot "
            "be greater than value of \"" << common::versionStr() << "\" property of the schema (" << protocol.currSchema().version() << ").";
        return false;
    }

    return true;
}

bool ParseXmlWrap::getAndCheckVersions(
    ::xmlNodePtr node,
    const std::string& name,
    const PropsMap& props,
    unsigned& sinceVersion,
    unsigned& deprecatedSince,
    ParseProtocolImpl& protocol)
{
    auto parentVersion = sinceVersion;
    auto parentDeprecated = deprecatedSince;
    auto sinceVerIter = props.find(common::sinceVersionStr());
    do {
        if (sinceVerIter == props.end()) {
            assert(sinceVersion <= protocol.currSchema().version());
            break;
        }

        auto& sinceVerStr = sinceVerIter->second;
        bool ok = false;
        sinceVersion = common::strToUnsigned(sinceVerStr, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(node, name, common::sinceVersionStr(), sinceVerStr, protocol.logger());
            return false;
        }

    } while (false);

    auto deprecatedIter = props.find(common::deprecatedStr());
    do {
        if (deprecatedIter == props.end()) {
            break;
        }

        auto& deprecatedStr = deprecatedIter->second;
        bool ok = false;
        deprecatedSince = common::strToUnsigned(deprecatedStr, &ok);
        if (!ok) {
            ParseXmlWrap::reportUnexpectedPropertyValue(node, name, common::deprecatedStr(), deprecatedStr, protocol.logger());
            return false;
        }

    } while (false);

    if (!checkVersions(node, sinceVersion, deprecatedSince, protocol, parentVersion, parentDeprecated)) {
        return false;
    }

    return true;
}

bool ParseXmlWrap::getAndCheckVersions(
    ::xmlNodePtr node,
    const std::string& name,
    unsigned& sinceVersion,
    unsigned& deprecatedSince,
    ParseProtocolImpl& protocol)
{
    auto props = parseNodeProps(node);
    static const NamesList Names = {
        common::sinceVersionStr(),
        common::deprecatedStr()
    };

    if (!parseChildrenAsProps(node, Names, protocol.logger(), props)) {
        return false;
    }

    return getAndCheckVersions(node, name, props, sinceVersion, deprecatedSince, protocol);
}

ParseXmlWrap::PropsMap ParseXmlWrap::getExtraAttributes(::xmlNodePtr node, const ParseXmlWrap::NamesList& names, ParseProtocolImpl& protocol)
{
    PropsMap attrs = ParseXmlWrap::getUnknownProps(node, names);
    auto& expectedPrefixes = protocol.extraElementPrefixes();
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
            logWarning(protocol.logger()) << logPrefix(node) <<
                "Unexpected attribute \"" << a.first << "\".";
        }
    }

    return attrs;
}

ParseXmlWrap::ContentsList ParseXmlWrap::getExtraChildren(::xmlNodePtr node, const ParseXmlWrap::NamesList& names, ParseProtocolImpl& protocol)
{
    ContentsList result;
    auto extraChildren = ParseXmlWrap::getUnknownChildren(node, names);
    auto& expectedPrefixes = protocol.extraElementPrefixes();
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
            logWarning(protocol.logger()) << logPrefix(c) <<
                "Unexpected element \"" << name << "\".";
        }
        result.push_back(ParseXmlWrap::getElementContent(c));
    }
    return result;
}

} // namespace parse

} // namespace commsdsl
