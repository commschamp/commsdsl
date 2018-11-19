//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "LayerImpl.h"

#include <cassert>
#include <limits>
#include <algorithm>
#include <set>
#include <iterator>

#include "ProtocolImpl.h"
#include "NamespaceImpl.h"
#include "common.h"
#include "PayloadLayerImpl.h"
#include "IdLayerImpl.h"
#include "SizeLayerImpl.h"
#include "SyncLayerImpl.h"
#include "ChecksumLayerImpl.h"
#include "ValueLayerImpl.h"
#include "CustomLayerImpl.h"

namespace commsdsl
{

LayerImpl::Ptr LayerImpl::create(
    const std::string& kind,
    ::xmlNodePtr node,
    ProtocolImpl& protocol)
{
    auto& map = createMap();

    auto iter = map.find(kind);
    if (iter == map.end()) {
        return Ptr();
    }

    return iter->second(node, protocol);
}

bool LayerImpl::parse()
{
    m_props = XmlWrap::parseNodeProps(m_node);

    if (!XmlWrap::parseChildrenAsProps(m_node, commonProps(), m_protocol.logger(), m_props)) {
        return false;
    }

    auto& extraPropsNames = extraPropsNamesImpl();
    do {
        if (extraPropsNames.empty()) {
            break;
        }

        if (!XmlWrap::parseChildrenAsProps(m_node, extraPropsNames, m_protocol.logger(), m_props)) {
            return false;
        }

    } while (false);

    if (!XmlWrap::parseChildrenAsProps(m_node, commonPossibleProps(), m_protocol.logger(), m_props, false)) {
        return false;
    }

    bool result =
        updateName() &&
        updateDescription() &&
        updateField();

    if (!result) {
        return false;
    }

    if (!parseImpl()) {
        return false;
    }

    XmlWrap::NamesList expectedProps = commonProps();
    expectedProps.insert(expectedProps.end(), commonPossibleProps().begin(), commonPossibleProps().end());
    expectedProps.insert(expectedProps.end(), extraPropsNames.begin(), extraPropsNames.end());
    if (!updateExtraAttrs(expectedProps)) {
        return false;
    }

    XmlWrap::NamesList expectedChildren = commonProps();
    expectedChildren.insert(expectedChildren.end(), commonPossibleProps().begin(), commonPossibleProps().end());
    expectedChildren.insert(expectedChildren.end(), extraPropsNames.begin(), extraPropsNames.end());

    auto supportedFields = FieldImpl::supportedTypes();
    expectedChildren.insert(expectedChildren.end(), supportedFields.begin(), supportedFields.end());
    if (!updateExtraChildren(expectedChildren)) {
        return false;
    }
    return true;
}

const std::string& LayerImpl::name() const
{
    assert(m_name != nullptr);
    return *m_name;
}

const std::string& LayerImpl::description() const
{
    assert(m_description != nullptr);
    return *m_description;
}

XmlWrap::NamesList LayerImpl::supportedTypes()
{
    XmlWrap::NamesList result;
    auto& map = createMap();
    result.reserve(map.size());
    std::transform(
        map.begin(), map.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return elem.first;
        });
    return result;
}

LayerImpl::LayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol),
    m_name(&common::emptyString()),
    m_description(&common::emptyString())
{
}

LogWrapper LayerImpl::logError() const
{
    return commsdsl::logError(m_protocol.logger());
}

LogWrapper LayerImpl::logWarning() const
{
    return commsdsl::logWarning(m_protocol.logger());
}

LogWrapper LayerImpl::logInfo() const
{
    return commsdsl::logInfo(m_protocol.logger());
}

Object::ObjKind LayerImpl::objKindImpl() const
{
    return ObjKind::Layer;
}

const XmlWrap::NamesList& LayerImpl::extraPropsNamesImpl() const
{
    return XmlWrap::emptyNamesList();
}

bool LayerImpl::parseImpl()
{
    return true;
}

bool LayerImpl::verifyImpl(const LayerImpl::LayersList& layers)
{
    static_cast<void>(layers);
    return true;
}

bool LayerImpl::mustHaveFieldImpl() const
{
    return true;
}

bool LayerImpl::validateSinglePropInstance(const std::string& str, bool mustHave)
{
    return XmlWrap::validateSinglePropInstance(m_node, m_props, str, protocol().logger(), mustHave);
}

bool LayerImpl::validateAndUpdateStringPropValue(
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

void LayerImpl::reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    XmlWrap::reportUnexpectedPropertyValue(m_node, name(), propName, propValue, protocol().logger());
}

bool LayerImpl::verifySingleLayer(const LayerImpl::LayersList& layers, const std::string& kindStr)
{
    auto k = kind();
    for (auto& l : layers) {
        if (l.get() == this) {
            continue;
        }

        if (l->kind() == k) {
            logError() << XmlWrap::logPrefix(l->getNode()) <<
                "Only single \"" << kindStr << "\" layer can exist in the frame.";
            return false;
        }
    }
    return true;
}

bool LayerImpl::verifyBeforePayload(const LayerImpl::LayersList& layers)
{
    auto thisIdx = findThisLayerIndex(layers);
    auto payloadIdx = findLayerIndex(layers, Kind::Payload);
    assert(thisIdx < layers.size());
    assert(payloadIdx < layers.size());

    if (payloadIdx <= thisIdx) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "This layer is expected to be before the \"" << common::payloadStr() <<
            "\" one.";
        return false;
    }

    return true;
}

std::size_t LayerImpl::findThisLayerIndex(const LayerImpl::LayersList& layers) const
{
    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [this](auto& l)
            {
                return this == l.get();
            });

    if (iter == layers.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(layers.begin(), iter));
}

std::size_t LayerImpl::findLayerIndex(
    const LayerImpl::LayersList& layers,
    LayerImpl::Kind lKind)
{
    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [lKind](auto& l)
            {
                return l->kind() == lKind;
            });

    if (iter == layers.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(layers.begin(), iter));
}

std::size_t LayerImpl::findLayerIndex(
    const LayerImpl::LayersList& layers,
    const std::string& name)
{
    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [&name](auto& l)
            {
                return l->name() == name;
            });

    if (iter == layers.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(layers.begin(), iter));
}

const XmlWrap::NamesList& LayerImpl::commonProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::descriptionStr()
    };

    return CommonNames;
}

const XmlWrap::NamesList&LayerImpl::commonPossibleProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::fieldStr()
    };

    return CommonNames;
}

bool LayerImpl::updateName()
{
    bool mustHave = m_name->empty();
    if (!validateAndUpdateStringPropValue(common::nameStr(), m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(*m_name)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid value for name property \"" << m_name << "\".";
        return false;
    }

    return true;
}

bool LayerImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_description);
}

bool LayerImpl::updateField()
{
    if ((!checkFieldFromRef()) ||
        (!checkFieldAsChild())) {
        return false;
    }

    if (hasField() == mustHaveFieldImpl()) {
        return true;
    }

    if (hasField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "This layer mustn't specify field.";
        return false;
    }

    logError() << XmlWrap::logPrefix(getNode()) <<
        "This layer must specify field.";

    return false;
}

bool LayerImpl::updateExtraAttrs(const XmlWrap::NamesList& names)
{
    auto extraAttrs = XmlWrap::getExtraAttributes(m_node, names, m_protocol);
    if (extraAttrs.empty()) {
        return true;
    }

    if (m_extraAttrs.empty()) {
        m_extraAttrs = std::move(extraAttrs);
        return true;
    }

    std::move(extraAttrs.begin(), extraAttrs.end(), std::inserter(m_extraAttrs, m_extraAttrs.end()));
    return true;
}

bool LayerImpl::updateExtraChildren(const XmlWrap::NamesList& names)
{
    auto extraChildren = XmlWrap::getExtraChildren(m_node, names, m_protocol);
    if (extraChildren.empty()) {
        return true;
    }

    if (m_extraChildren.empty()) {
        m_extraChildren = std::move(extraChildren);
        return true;
    }

    m_extraChildren.reserve(m_extraChildren.size() + extraChildren.size());
    std::move(extraChildren.begin(), extraChildren.end(), std::back_inserter(m_extraChildren));
    return true;
}

bool LayerImpl::checkFieldFromRef()
{
    if (!validateSinglePropInstance(common::fieldStr())) {
        return false;
    }

    auto iter = props().find(common::fieldStr());
    if (iter == props().end()) {
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::fieldStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_extField = field;
    assert(!m_field);
    return true;
}

bool LayerImpl::checkFieldAsChild()
{
    auto children = XmlWrap::getChildren(getNode(), common::fieldStr());
    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::fieldStr() << "\" child element.";
        return false;
    }

    auto fieldTypes = XmlWrap::getChildren(getNode(), FieldImpl::supportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                  "The frame layer element does not support "
                  "stand alone field as child element together with \"" <<
                  common::fieldStr() << "\" child element.";
        return false;
    }

    if (children.empty() && fieldTypes.empty()) {
        return true;
    }

    ::xmlNodePtr fieldNode = nullptr;
    do {
        if (fieldTypes.empty()) {
            break;
        }

        if (1U < fieldTypes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "The frame layer element is expected to define only "
                "single field";
            return false;
        }

        auto allChildren = XmlWrap::getChildren(getNode());
        if (allChildren.size() != fieldTypes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                  "The field type of frame layer "
                  " must be defined inside \"<" << common::fieldsStr() << ">\" child element "
                  "when there are other property describing children.";
            return false;
        }

        if (props().find(common::fieldStr()) != props().end()) {
            logError() << "There must be only one occurance of \"" << common::fieldStr() << "\" definition.";
            return false;
        }

        fieldNode = fieldTypes.front();
    } while (false);


    do {
        if (fieldNode != nullptr) {
            assert(children.empty());
            break;
        }

        assert(!children.empty());

        auto child = children.front();
        auto fields = XmlWrap::getChildren(child);
        if (1U < fields.size()) {
            logError() << XmlWrap::logPrefix(child) <<
                "The \"" << common::fieldStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (props().find(common::fieldStr()) == props().end()) {
            fieldNode = fields.front();
            break;
        }

        auto attrs = XmlWrap::parseNodeProps(getNode());
        if (attrs.find(common::fieldsStr()) != attrs.end()) {
            logError() << "There must be only one occurance of \"" << common::fieldStr() << "\" definition.";
            return false;
        }

        // The field element is parsed as property
        return true;
    } while (false);

    assert (fieldNode != nullptr);

    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Unknown field type \"" << fieldKind << "\"";
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_extField = nullptr;
    m_field = std::move(field);
    assert(m_field->externalRef().empty());
    return true;
}

const LayerImpl::CreateMap& LayerImpl::createMap()
{
    static const CreateMap Map = {
        std::make_pair(
            common::payloadStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new PayloadLayerImpl(n, p));
            }),
        std::make_pair(
            common::idStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new IdLayerImpl(n, p));
            }),
        std::make_pair(
            common::sizeStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new SizeLayerImpl(n, p));
            }),
        std::make_pair(
            common::syncStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new SyncLayerImpl(n, p));
            }),
        std::make_pair(
            common::checksumStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new ChecksumLayerImpl(n, p));
            }),
        std::make_pair(
            common::valueStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new ValueLayerImpl(n, p));
            }),
        std::make_pair(
            common::customStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new CustomLayerImpl(n, p));
            }),
    };

    return Map;
}

} // namespace commsdsl
