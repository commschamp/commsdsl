#include "XmlWrap.h"

#include <cassert>
#include <algorithm>
#include <iostream>

namespace bbmp
{

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
    NodesList result;
    auto* cur = node->children;
    while (cur != nullptr) {
        do {
            if (cur->type != XML_ELEMENT_NODE) {
                break;
            }

            if (name.empty()) {
                result.push_back(cur);
                break;
            }

            std::string elemName(reinterpret_cast<const char*>(cur->name));
            if (elemName == name) {
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


bool XmlWrap::parseChildrenAsProps(
    ::xmlNodePtr node,
    const NamesList& names,
    Logger& logger,
    PropsMap& result)
{
    auto children = getChildren(node);
    for (auto* c : children) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = std::find(names.begin(), names.end(), cName);
        if (iter == names.end()) {
            continue;
        }

        auto resultIter = result.find(cName);
        if (resultIter != result.end()) {
            logError(logger) << node->doc->URL << ":" << node->line <<
                ": Multiple values of \"" << cName << "\" properties for \"" <<
                node->name << "\"";
            return false;
        }

        auto chProps = parseNodeProps(c);
        static const std::string ValueAttr("value");
        auto valIter = chProps.find(ValueAttr);
        std::string value;
        if (valIter != chProps.end()) {
            value = valIter->second;
        }

        auto text = getText(c);
        if (value.empty() && text.empty()) {
            logError(logger) << node->doc->URL << ":" << node->line <<
                ": No value for \"" << cName << "\" inside \"" << node->name << "\" element.";
            return false;
        }

        if ((!value.empty()) && (!text.empty())) {
            logError(logger) << node->doc->URL << ":" << node->line <<
                ": Incorrect value format for \"" << cName << "\" inside \"" << node->name << "\" element.";
            return false;
        }

        if (!value.empty()) {
            result.insert(std::make_pair(cName, value));
            continue;
        }

        if (!text.empty()) {
            result.insert(std::make_pair(cName, text));
            continue;
        }
    }

    return true;
}

XmlWrap::PropsMap XmlWrap::getUnknownProps(::xmlNodePtr node, const XmlWrap::NamesList& names)
{
    auto props = parseNodeProps(node);
    for (auto& n : names) {
        auto iter = props.find(n);
        if (iter == props.end()) {
            continue;
        }

        props.erase(iter);
    }
    return props;
}

XmlWrap::ContentsList XmlWrap::getUnknownChildren(::xmlNodePtr node, const XmlWrap::NamesList& names)
{
    ContentsList result;
    auto children = getChildren(node);
    for (auto* c : children) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto iter = std::find(names.begin(), names.end(), cName);
        if (iter != names.end()) {
            continue;
        }

        BufferPtr buf(::xmlBufferCreate());
        auto bufLen = ::xmlNodeDump(buf.get(), c->doc, c, 0, 0);
        if (bufLen == 0U) {
            continue;
        }

        result.emplace_back(reinterpret_cast<const char*>(::xmlBufferContent(buf.get())));
    }
    return result;
}



} // namespace bbmp
