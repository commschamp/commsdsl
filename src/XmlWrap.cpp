#include "XmlWrap.h"

#include <cassert>
#include <algorithm>
#include <iostream>

#include "ProtocolImpl.h"

namespace bbmp
{

const XmlWrap::NamesList& XmlWrap::emptyNamesList()
{
    static const NamesList List;
    return List;
}

XmlWrap::PropsMap XmlWrap::parseNodeProps(::xmlNodePtr node)
{
    assert(node != nullptr);
    PropsMap map;
    auto* prop = node->properties;
    while (prop != nullptr) {
        StringPtr valuePtr(::xmlNodeListGetString(node->doc, prop->children, 1));
        map.insert(
            std::make_pair(
                reinterpret_cast<const char*>(prop->name),
                reinterpret_cast<const char*>(valuePtr.get())));
        prop = prop->next;
    }

    return map;
}

XmlWrap::NodesList XmlWrap::getChildren(::xmlNodePtr node, const std::string& name)
{
    NamesList names;
    if (!name.empty()) {
        names.push_back(name);
    }
    return getChildren(node, names);
}

XmlWrap::NodesList XmlWrap::getChildren(::xmlNodePtr node, const NamesList& names)
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

            std::string elemName(reinterpret_cast<const char*>(cur->name));
            auto iter = std::find(names.begin(), names.end(), elemName);
            if (iter != names.end()) {
                result.push_back(cur);
                break;
            }
        } while (false);

        cur = cur->next;
    }
    return result;
}

std::string XmlWrap::getText(::xmlNodePtr node)
{
    assert(node != nullptr);
    auto* child = node->children;
    while (child != nullptr) {
        if (child->type == XML_TEXT_NODE) {
            StringPtr valuePtr(::xmlNodeGetContent(child));
            return std::string(reinterpret_cast<const char*>(valuePtr.get()));
        }
        child = child->next;
    }
    return std::string();
}

bool XmlWrap::parseNodeValue(
    ::xmlNodePtr node,
    Logger& logger,
    std::string& value,
    bool mustHaveValue)
{
    auto props = parseNodeProps(node);
    static const std::string ValueAttr("value");
    auto valIter = props.find(ValueAttr);
    std::string valueTmp;
    if (valIter != props.end()) {
        valueTmp = valIter->second;
    }

    auto text = getText(node);
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
    value = std::move(text);
    return true;
}


bool XmlWrap::parseChildrenAsProps(
    ::xmlNodePtr node,
    const NamesList& names,
    Logger& logger,
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

XmlWrap::PropsMap XmlWrap::getUnknownProps(::xmlNodePtr node, const XmlWrap::NamesList& names)
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

XmlWrap::NodesList XmlWrap::getUnknownChildren(::xmlNodePtr node, const XmlWrap::NamesList& names)
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

XmlWrap::ContentsList XmlWrap::getUnknownChildrenContents(::xmlNodePtr node, const XmlWrap::NamesList& names)
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

std::string XmlWrap::logPrefix(::xmlNodePtr node)
{
    assert(node != nullptr);
    assert(node->doc != nullptr);
    assert(node->doc->URL != nullptr);
    return std::string(reinterpret_cast<const char*>(node->doc->URL)) + ":" + std::to_string(node->line) + ": ";
}

bool XmlWrap::validateSinglePropInstance(
    ::xmlNodePtr node,
    const PropsMap& props,
    const std::string& str,
    Logger& logger,
    bool mustHave)
{
    auto count = props.count(str);
    if (1U < count) {
        bbmp::logError(logger) << XmlWrap::logPrefix(node) <<
                      "Too many values of \"" << str << "\" property for \"" << node->name << "\" element.";
        return false;
    }

    if ((count == 0U) && mustHave) {
        bbmp::logError(logger) << XmlWrap::logPrefix(node) <<
                      "Missing value for mandatory property \"" << str << "\" for \"" << node->name << "\" element.";
        return false;
    }

    return true;
}

bool XmlWrap::validateNoPropInstance(::xmlNodePtr node, const XmlWrap::PropsMap& props, const std::__cxx11::string& str, Logger& logger)
{
    auto iter = props.find(str);
    if (iter != props.end()) {
        bbmp::logError(logger) << XmlWrap::logPrefix(node) <<
                      "Preperty \"" << str << "\" defined when should not.";
        return false;
    }

    return true;
}

bool XmlWrap::hasAnyChild(::xmlNodePtr node, const XmlWrap::NamesList& names)
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

void XmlWrap::reportUnexpectedPropertyValue(
    ::xmlNodePtr node,
    const std::string& elemName,
    const std::string& propName,
    const std::string& propValue,
    Logger& logger)
{
    bbmp::logError(logger) << XmlWrap::logPrefix(node) <<
                  "Property \"" << propName << "\" of element \"" << elemName <<
                              "\" has unexpected value (" << propValue << ").";
}

bool XmlWrap::checkVersions(
    ::xmlNodePtr node,
    unsigned sinceVersion,
    unsigned deprecatedSince,
    ProtocolImpl& protocol,
    unsigned parentVersion,
    unsigned parentDeprecated)
{
    assert(parentVersion < parentDeprecated);
    if (protocol.schemaImpl().version() < sinceVersion) {
        bbmp::logError(protocol.logger()) << XmlWrap::logPrefix(node) <<
            "The value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ") cannot "
            "be greater than value of \"" << common::versionStr() << "\" property of the schema (" << protocol.schemaImpl().version() << ").";
        return false;
    }

    if (sinceVersion < parentVersion) {
        bbmp::logError(protocol.logger()) << XmlWrap::logPrefix(node) <<
            "The value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ") cannot "
            "be less than " << parentVersion << ".";
        return false;
    }

    if (parentDeprecated <= sinceVersion) {
        bbmp::logError(protocol.logger()) << XmlWrap::logPrefix(node) <<
            "The value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ") must "
            "be less than " << parentDeprecated << ".";
        return false;
    }

    if (parentDeprecated < deprecatedSince) {
        bbmp::logError(protocol.logger()) << XmlWrap::logPrefix(node) <<
            "The value of \"" << common::deprecatedStr() << "\" property (" << deprecatedSince << ") cannot "
            "be greater than " << parentDeprecated << ".";
        return false;
    }


    if (deprecatedSince <= sinceVersion) {
        bbmp::logError(protocol.logger()) << XmlWrap::logPrefix(node) <<
            "The value of \"" << common::deprecatedStr() << "\" property (" << deprecatedSince << ") must "
            "be greater than value of \"" << common::sinceVersionStr() << "\" property (" << sinceVersion << ").";
        return false;
    }

    if ((deprecatedSince < bbmp::Protocol::notYetDeprecated()) &&
        (protocol.schemaImpl().version() < deprecatedSince)) {
        bbmp::logError(protocol.logger()) << XmlWrap::logPrefix(node) <<
            "The value of \"" << common::deprecatedStr() << "\" property (" << deprecatedSince << ") cannot "
            "be greater than value of \"" << common::versionStr() << "\" property of the schema (" << protocol.schemaImpl().version() << ").";
        return false;
    }

    return true;
}

bool XmlWrap::getAndCheckVersions(
    ::xmlNodePtr node,
    const std::string& name,
    const PropsMap& props,
    unsigned& sinceVersion,
    unsigned& deprecatedSince,
    ProtocolImpl& protocol)
{
    auto parentVersion = sinceVersion;
    auto parentDeprecated = deprecatedSince;
    auto sinceVerIter = props.find(common::sinceVersionStr());
    do {
        if (sinceVerIter == props.end()) {
            assert(sinceVersion <= protocol.schemaImpl().version());
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
            XmlWrap::reportUnexpectedPropertyValue(node, name, common::deprecatedStr(), deprecatedStr, protocol.logger());
            return false;
        }

    } while (false);

    if (!checkVersions(node, sinceVersion, deprecatedSince, protocol, parentVersion, parentDeprecated)) {
        return false;
    }

    return true;
}

bool XmlWrap::getAndCheckVersions(
    ::xmlNodePtr node,
    const std::string& name,
    unsigned& sinceVersion,
    unsigned& deprecatedSince,
    ProtocolImpl& protocol)
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

} // namespace bbmp
