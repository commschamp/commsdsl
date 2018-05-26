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

    auto& extraPossiblePropsNames = extraPossiblePropsNamesImpl();
    do {
        if (extraPossiblePropsNames.empty()) {
            break;
        }

        if (!XmlWrap::parseChildrenAsProps(m_node, extraPossiblePropsNames, m_protocol.logger(), m_props, false)) {
            return false;
        }

    } while (false);

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
    expectedProps.insert(expectedProps.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    if (!updateExtraAttrs(expectedProps)) {
        return false;
    }

    auto& extraChildren = extraChildrenNamesImpl();
    XmlWrap::NamesList expectedChildren = commonProps();
    expectedChildren.insert(expectedChildren.end(), commonPossibleProps().begin(), commonPossibleProps().end());
    expectedChildren.insert(expectedChildren.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraChildren.begin(), extraChildren.end());
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

const XmlWrap::NamesList&LayerImpl::extraPossiblePropsNamesImpl() const
{
    return XmlWrap::emptyNamesList();
}

const XmlWrap::NamesList&LayerImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList Names;
    return Names;
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
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::fieldStr() << "\".";
        return false;
    }

    auto child = children.front();
    auto fields = XmlWrap::getChildren(child);
    if (1U < fields.size()) {
        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << common::fieldStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = props().find(common::fieldStr());
    bool hasInProps = iter != props().end();
    if (fields.empty()) {
        assert(hasInProps);
        return true;
    }

    if (hasInProps) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << common::fieldStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = fields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        logError() << XmlWrap::logPrefix(fieldNode) <<
            "Unknown field type \"" << fieldKind << "\".";
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_extField = nullptr;
    m_field = std::move(field);
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
    };

    return Map;
}

} // namespace commsdsl
