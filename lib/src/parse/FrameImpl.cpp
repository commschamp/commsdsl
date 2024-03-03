//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "FrameImpl.h"

#include <cassert>
#include <limits>
#include <algorithm>
#include <set>
#include <numeric>
#include <iterator>

#include "ProtocolImpl.h"
#include "NamespaceImpl.h"
#include "common.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const XmlWrap::NamesList& frameSupportedTypes()
{
    static const XmlWrap::NamesList Names = LayerImpl::supportedTypes();
    return Names;
}


} // namespace

FrameImpl::FrameImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol),
    m_name(&common::emptyString()),
    m_description(&common::emptyString())
{
}

bool FrameImpl::parse()
{
    m_props = XmlWrap::parseNodeProps(m_node);

    if (!XmlWrap::parseChildrenAsProps(m_node, commonProps(), m_protocol.logger(), m_props)) {
        return false;
    }

    return
        updateName() &&
        updateDescription() &&
        updateLayers() &&
        updateExtraAttrs() &&
        updateExtraChildren();
}

const std::string& FrameImpl::name() const
{
    assert(m_name != nullptr);
    return *m_name;
}

const std::string& FrameImpl::description() const
{
    assert(m_description != nullptr);
    return *m_description;
}

FrameImpl::LayersList FrameImpl::layersList() const
{
    LayersList result;
    result.reserve(m_layers.size());
    std::transform(
        m_layers.begin(), m_layers.end(), std::back_inserter(result),
        [](auto& f)
        {
            return Layer(f.get());
        });
    return result;
}

std::string FrameImpl::externalRef(bool schemaRef) const
{
    assert(getParent() != nullptr);
    assert(getParent()->objKind() == ObjKind::Namespace);

    auto& ns = static_cast<const NamespaceImpl&>(*getParent());
    auto nsRef = ns.externalRef(schemaRef);
    if (nsRef.empty()) {
        return name();
    }

    return nsRef + '.' + name();
}

Object::ObjKind FrameImpl::objKindImpl() const
{
    return ObjKind::Frame;
}

LogWrapper FrameImpl::logError() const
{
    return commsdsl::parse::logError(m_protocol.logger());
}

LogWrapper FrameImpl::logWarning() const
{
    return commsdsl::parse::logWarning(m_protocol.logger());
}

LogWrapper FrameImpl::logInfo() const
{
    return commsdsl::parse::logInfo(m_protocol.logger());
}

bool FrameImpl::validateSinglePropInstance(const std::string& str, bool mustHave)
{
    return XmlWrap::validateSinglePropInstance(m_node, m_props, str, m_protocol.logger(), mustHave);
}

bool FrameImpl::validateAndUpdateStringPropValue(
    const std::string& str,
    const std::string*& valuePtr,
    bool mustHave)
{
    if (!validateSinglePropInstance(str, mustHave)) {
        return false;
    }

    auto iter = m_props.find(str);
    if (iter != m_props.end()) {
        valuePtr = &iter->second;
    }

    assert(iter != m_props.end() || (!mustHave));
    return true;
}

void FrameImpl::reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    XmlWrap::reportUnexpectedPropertyValue(m_node, name(), propName, propValue, m_protocol.logger());
}

const XmlWrap::NamesList& FrameImpl::commonProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::descriptionStr(),
    };

    return CommonNames;
}

XmlWrap::NamesList FrameImpl::allNames()
{
    auto names = commonProps();
    auto& layerTypes = frameSupportedTypes();
    names.insert(names.end(), layerTypes.begin(), layerTypes.end());
    names.push_back(common::layersStr());
    return names;
}

bool FrameImpl::updateName()
{
    assert(m_name != nullptr);
    bool mustHave = m_name->empty();
    if (!validateAndUpdateStringPropValue(common::nameStr(), m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(*m_name)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid value for name property \"" << *m_name << "\".";
        return false;
    }

    return true;
}

bool FrameImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_description);
}

bool FrameImpl::updateLayers()
{
    auto layersNodes = XmlWrap::getChildren(getNode(), common::layersStr());
    if (1U < layersNodes.size()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Only single \"" << common::layersStr() << "\" child element is "
                      "supported for \"" << common::frameStr() << "\".";
        return false;
    }

    auto layersTypes = XmlWrap::getChildren(getNode(), frameSupportedTypes());
    if ((!layersNodes.empty()) && (!layersTypes.empty())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The \"" << common::frameStr() << "\" element does not support "
                      "list of stand alone layers as child elements together with \"" <<
                      common::layersStr() << "\" child element.";
        return false;
    }

    if ((layersNodes.empty()) && (layersTypes.empty())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << common::framesStr() << "\" element must contain at least one layer";
        return false;
    }

    if ((0U < layersTypes.size())) {
        assert(0U == layersNodes.size());
        auto allChildren = XmlWrap::getChildren(getNode());
        if (allChildren.size() != layersTypes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The layer types of \"" << common::frameStr() <<
                          "\" must be defined inside \"<" << common::layersStr() << ">\" child element "
                          "when there are other property describing children.";
            return false;
        }
    }

    if (0U < layersNodes.size()) {
        assert(0U == layersTypes.size());
        layersTypes = XmlWrap::getChildren(layersNodes.front());
        auto cleanMemberFieldsTypes = XmlWrap::getChildren(layersNodes.front(), frameSupportedTypes());
        if (cleanMemberFieldsTypes.size() != layersTypes.size()) {
            logError() << XmlWrap::logPrefix(layersNodes.front()) <<
                "The \"" << common::layersStr() << "\" child node of \"" <<
                common::frameStr() << "\" element must contain only supported layer types.";
            return false;
        }

        // layersTypes is updated with the list from <layers>
    }

    assert(m_layers.empty());
    m_layers.reserve(layersTypes.size());
    bool hasPayloadLayer = false;
    for (auto* lNode : layersTypes) {
        std::string lKind(reinterpret_cast<const char*>(lNode->name));
        auto layer = LayerImpl::create(lKind, lNode, m_protocol);
        if (!layer) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            logError() << XmlWrap::logPrefix(getNode()) <<
                  "Internal error, failed to create objects for layers.";
            return false;
        }

        layer->setParent(this);
        if (!layer->parse()) {
            return false;
        }

        auto lIter =
            std::find_if(
                m_layers.begin(), m_layers.end(),
                [&layer](auto& l)
                {
                    return layer->name() == l->name();
                });

        if (lIter != m_layers.end()) {
            logError() << XmlWrap::logPrefix(lNode) <<
                "Layer with name \"" << layer->name() << "\" has already been "
                "defined within the same frame.";
        }

        if (layer->kind() == LayerImpl::Kind::Payload) {
            hasPayloadLayer = true;
        }

        m_layers.push_back(std::move(layer));
    }

    if (!hasPayloadLayer) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The frame \"" << name() << "\" must contain a \"" <<
            common::payloadStr() << "\" layer.";
        return false;
    }

    bool verified =
        std::all_of(
            m_layers.begin(), m_layers.end(),
            [this](auto& l)
            {
                assert(l);
                return l->verify(m_layers);
            });

    return verified;
}

bool FrameImpl::updateExtraAttrs()
{
    m_extraAttrs = XmlWrap::getExtraAttributes(m_node, commonProps(), m_protocol);
    return true;
}

bool FrameImpl::updateExtraChildren()
{
    static const XmlWrap::NamesList ChildrenNames = allNames();
    m_extraChildren = XmlWrap::getExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}


} // namespace parse

} // namespace commsdsl
