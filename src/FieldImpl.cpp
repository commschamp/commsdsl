#include "FieldImpl.h"

#include <functional>
#include <map>
#include <string>
#include <cassert>
#include <limits>

#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "FloatFieldImpl.h"
#include "BitfieldFieldImpl.h"
#include "common.h"

namespace bbmp
{

FieldImpl::Ptr FieldImpl::create(
    const std::string& kind,
    ::xmlNodePtr node,
    ProtocolImpl& protocol)
{
    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ProtocolImpl& p)>;
    static const std::map<std::string, CreateFunc> CreateMap = {
        std::make_pair(
            common::intStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new IntFieldImpl(n, p));
            }),
        std::make_pair(
            common::floatStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new FloatFieldImpl(n, p));
            }),
        std::make_pair(
            common::bitfieldStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new BitfieldFieldImpl(n, p));
            })
    };

    auto iter = CreateMap.find(kind);
    if (iter == CreateMap.end()) {
        return Ptr();
    }

    return iter->second(node, protocol);
}

bool FieldImpl::parse()
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

    bool result =
        updateName() &&
        updateDisplayName() &&
        updateDescription();

    if (!result) {
        return false;
    }

    if (!parseImpl()) {
        return false;
    }

    XmlWrap::NamesList expectedProps = commonProps();
    expectedProps.insert(expectedProps.end(), extraPropsNames.begin(), extraPropsNames.end());
    m_unknownAttrs = XmlWrap::getUnknownProps(m_node, expectedProps);

    auto& commonCh = commonChildren();
    auto& extraChildren = extraChildrenNamesImpl();
    XmlWrap::NamesList expectedChildren = commonProps();
    expectedChildren.insert(expectedChildren.end(), commonCh.begin(), commonCh.end());
    expectedChildren.insert(expectedChildren.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraChildren.begin(), extraChildren.end());
    m_unknownChildren = XmlWrap::getUnknownChildrenContents(m_node, expectedChildren);
    return true;
}

const std::string& FieldImpl::name() const
{
    assert(m_name != nullptr);
    return *m_name;
}

const std::string& FieldImpl::displayName() const
{
    assert(m_displayName != nullptr);
    return *m_displayName;
}

const std::string& FieldImpl::description() const
{
    assert(m_description != nullptr);
    return *m_description;
}

FieldImpl::FieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol),
    m_name(&common::emptyString()),
    m_displayName(&common::emptyString()),
    m_description(&common::emptyString())
{
}

LogWrapper FieldImpl::logError() const
{
    return bbmp::logError(m_protocol.logger());
}

LogWrapper FieldImpl::logWarning() const
{
    return bbmp::logWarning(m_protocol.logger());
}

Object::ObjKind FieldImpl::objKindImpl() const
{
    return ObjKind::SimpleField;
}

const XmlWrap::NamesList& FieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList Names;
    return Names;
}

const XmlWrap::NamesList&FieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList Names;
    return Names;
}

bool FieldImpl::parseImpl()
{
    return true;
}

bool FieldImpl::validateImpl()
{
    return true;
}

std::size_t FieldImpl::bitLengthImpl() const
{
    return length() * std::numeric_limits<std::uint8_t>::digits;
}

bool FieldImpl::validateSinglePropInstance(const std::string& str, bool mustHave)
{
    return XmlWrap::validateSinglePropInstance(m_node, m_props, str, protocol().logger(), mustHave);
}

bool FieldImpl::validateAndUpdateStringPropValue(
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

void FieldImpl::reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    logError() << XmlWrap::logPrefix(m_node) <<
                  "Property \"" << propName << "\" of element \"" << name() <<
                  "\" has unexpected value (" << propValue << ").";
}

const XmlWrap::NamesList& FieldImpl::commonProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::displayNameStr(),
        common::descriptionStr()
    };

    return CommonNames;
}

const XmlWrap::NamesList& FieldImpl::commonChildren()
{
    static const XmlWrap::NamesList CommonChildren = {
        common::metaStr()
    };

    return CommonChildren;
}

bool FieldImpl::updateName()
{
    return validateAndUpdateStringPropValue(common::nameStr(), m_name, true);
}

bool FieldImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_description);
}

bool FieldImpl::updateDisplayName()
{
    return validateAndUpdateStringPropValue(common::displayNameStr(), m_displayName);
}

} // namespace bbmp
