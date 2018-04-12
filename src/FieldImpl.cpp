#include "FieldImpl.h"

#include <cassert>
#include <limits>
#include <algorithm>
#include <set>

#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "FloatFieldImpl.h"
#include "EnumFieldImpl.h"
#include "SetFieldImpl.h"
#include "BitfieldFieldImpl.h"
#include "BundleFieldImpl.h"
#include "RefFieldImpl.h"
#include "common.h"

namespace bbmp
{

FieldImpl::Ptr FieldImpl::create(
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
        checkReuse() &&
        updateName() &&
        updateDisplayName() &&
        updateDescription() &&
        updateVersions();

    if (!result) {
        return false;
    }

    if (!parseImpl()) {
        return false;
    }

    XmlWrap::NamesList expectedProps = commonProps();
    expectedProps.insert(expectedProps.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedProps.insert(expectedProps.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    m_state.m_unknownAttrs = XmlWrap::getUnknownProps(m_node, expectedProps);

    auto& commonCh = commonChildren();
    auto& extraChildren = extraChildrenNamesImpl();
    XmlWrap::NamesList expectedChildren = commonProps();
    expectedChildren.insert(expectedChildren.end(), commonCh.begin(), commonCh.end());
    expectedChildren.insert(expectedChildren.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraChildren.begin(), extraChildren.end());
    m_state.m_unknownChildren = XmlWrap::getUnknownChildrenContents(m_node, expectedChildren);
    return true;
}

const std::string& FieldImpl::name() const
{
    assert(m_state.m_name != nullptr);
    return *m_state.m_name;
}

const std::string& FieldImpl::displayName() const
{
    assert(m_state.m_displayName != nullptr);
    return *m_state.m_displayName;
}

const std::string& FieldImpl::description() const
{
    assert(m_state.m_description != nullptr);
    return *m_state.m_description;
}

XmlWrap::NamesList FieldImpl::supportedTypes()
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

//bool FieldImpl::validateMembersVersions(
//    const Object& obj,
//    const FieldImpl::FieldsList& fields,
//    Logger& logger)
//{
//    if (fields.size() < 2U) {
//        return true;
//    }

//    for (auto idx = 0U; idx < (fields.size() - 1); ++idx) {
//        auto& thisMem = fields[idx];
//        auto& nextMem = fields[idx + 1];

//        assert(obj.getSinceVersion() <= thisMem->getMinSinceVersion());

//        if (nextMem->getMinSinceVersion() < thisMem->getMaxSinceVersion()) {
//            bbmp::logError(logger) << XmlWrap::logPrefix(nextMem->getNode()) <<
//                "Version of the member \"" << nextMem->name() << "\" (" <<
//                nextMem->getMinSinceVersion() << ") must't be less than previous "
//                " member's one (" << thisMem->getMaxSinceVersion() << ").";
//            return false;
//        }

//        assert(thisMem->getMaxSinceVersion() <= obj.getMaxSinceVersion());
//    }
//    return true;
//}

//bool FieldImpl::validateMembersVersions(const FieldImpl::FieldsList& fields)
//{
//    return validateMembersVersions(*this, fields, protocol().logger());
//}

bool FieldImpl::validateMembersNames(
    const FieldImpl::FieldsList& fields,
    Logger& logger)
{
    std::set<std::string> usedNames;
    for (auto& f : fields) {
        if (usedNames.find(f->name()) != usedNames.end()) {
            bbmp::logError(logger) << XmlWrap::logPrefix(f->getNode()) <<
                "Member field with name \"" << f->name() << "\" has already been defined.";
            return false;
        }
        usedNames.insert(f->name());
    }
    return true;
}

bool FieldImpl::validateMembersNames(const FieldImpl::FieldsList& fields)
{
    return validateMembersNames(fields, protocol().logger());
}

bool FieldImpl::isBitfieldMember() const
{
    return (getParent() != nullptr) &&
           (getParent()->objKind() == ObjKind::Field) &&
           (static_cast<const FieldImpl*>(getParent())->kind() == Kind::Bitfield);
}

FieldImpl::FieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
    m_state.m_name = &common::emptyString();
    m_state.m_displayName = &common::emptyString();
    m_state.m_description = &common::emptyString();
}

FieldImpl::FieldImpl(const FieldImpl&) = default;

LogWrapper FieldImpl::logError() const
{
    return bbmp::logError(m_protocol.logger());
}

LogWrapper FieldImpl::logWarning() const
{
    return bbmp::logWarning(m_protocol.logger());
}

LogWrapper FieldImpl::logInfo() const
{
    return bbmp::logInfo(m_protocol.logger());
}

Object::ObjKind FieldImpl::objKindImpl() const
{
    return ObjKind::Field;
}

const XmlWrap::NamesList& FieldImpl::extraPropsNamesImpl() const
{
    return XmlWrap::emptyNamesList();
}

const XmlWrap::NamesList&FieldImpl::extraPossiblePropsNamesImpl() const
{
    return XmlWrap::emptyNamesList();
}

const XmlWrap::NamesList&FieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList Names;
    return Names;
}

bool FieldImpl::reuseImpl(const FieldImpl& other)
{
    static_cast<void>(other);
    assert(!"NYI, this function should not be called");
    return false;
}

bool FieldImpl::parseImpl()
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

bool FieldImpl::validateNoPropInstance(const std::string& str)
{
    return XmlWrap::validateNoPropInstance(m_node, m_props, str, protocol().logger());
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
    XmlWrap::reportUnexpectedPropertyValue(m_node, name(), propName, propValue, protocol().logger());
}

const XmlWrap::NamesList& FieldImpl::commonProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::displayNameStr(),
        common::descriptionStr(),
        common::sinceVersionStr(),
        common::deprecatedStr(),
        common::removedStr()
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

bool FieldImpl::checkReuse()
{
    if (!validateSinglePropInstance(common::reuseStr())) {
        return false;
    }

    auto iter = m_props.find(common::reuseStr());
    if (iter == m_props.end()) {
        return true;
    }

    auto& valueStr = iter->second;
    auto* field = m_protocol.findField(valueStr);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The field \"" << valueStr << "\" hasn't been recorded yet.";
        return false;
    }

    if (field->kind() != kind()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Cannot reuse field of different kind (\"" << valueStr << "\").";
        return false;
    }

    assert(field != this);
    Base::reuseState(*field);
    m_state = field->m_state;

    assert(getSinceVersion() == 0U);
    assert(getDeprecated() == bbmp::Protocol::notYetDeprecated());
    assert(!isDeprecatedRemoved());
    return reuseImpl(*field);
}

bool FieldImpl::updateName()
{
    bool mustHave = m_state.m_name->empty();
    if (!validateAndUpdateStringPropValue(common::nameStr(), m_state.m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(*m_state.m_name)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid value for name property \"" << m_state.m_name << "\".";
        return false;
    }

    return true;
}

bool FieldImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_state.m_description);
}

bool FieldImpl::updateDisplayName()
{
    return validateAndUpdateStringPropValue(common::displayNameStr(), m_state.m_displayName);
}

bool FieldImpl::updateVersions()
{
    if (!validateSinglePropInstance(common::sinceVersionStr())) {
        return false;
    }

    if (!validateSinglePropInstance(common::deprecatedStr())) {
        return false;
    }

    if (!validateSinglePropInstance(common::removedStr())) {
        return false;
    }

    unsigned sinceVersion = 0U;
    if ((getParent() != nullptr) && (getParent()->objKind() != ObjKind::Namespace)) {
        sinceVersion = getParent()->getSinceVersion();
    }

    unsigned deprecated = bbmp::Protocol::notYetDeprecated();
    if ((getParent() != nullptr) && (getParent()->objKind() != ObjKind::Namespace)) {
        deprecated = getParent()->getDeprecated();
    }

    if (!XmlWrap::getAndCheckVersions(m_node, name(), m_props, sinceVersion, deprecated, protocol())) {
        return false;
    }

    do {
        if ((getParent() != nullptr) && (getParent()->objKind() != ObjKind::Namespace)) {
            break;
        }

        if (sinceVersion != 0U) {
            logWarning() << XmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::sinceVersionStr() << "\" is not applicable to "
                "stand alone fields, ignoring provided value";
            sinceVersion = 0U;
        }

        if (deprecated != bbmp::Protocol::notYetDeprecated()) {
            logWarning() << XmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::deprecatedStr() << "\" is not applicable to "
                "stand alone fields, ignoring provided value";
            deprecated = bbmp::Protocol::notYetDeprecated();
        }

    } while (false);

    bool deprecatedRemoved = false;
    do {
        auto deprecatedRemovedIter = m_props.find(common::removedStr());
        if (deprecatedRemovedIter == m_props.end()) {
            break;
        }

        bool ok = false;
        deprecatedRemoved = common::strToBool(deprecatedRemovedIter->second, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(common::removedStr(), deprecatedRemovedIter->second);
            return false;
        }

        if (!deprecatedRemoved) {
            break;
        }

        if (deprecated == bbmp::Protocol::notYetDeprecated()) {
            logWarning() << XmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::removedStr() << "\" is not applicable to "
                "non deprecated fields";
        }
    } while (false);

    setSinceVersion(sinceVersion);
    setDeprecated(deprecated);
    setDeprecatedRemoved(deprecatedRemoved);
    return true;
}

const FieldImpl::CreateMap& FieldImpl::createMap()
{
    static const CreateMap Map = {
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
            common::enumStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new EnumFieldImpl(n, p));
            }),
        std::make_pair(
            common::setStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new SetFieldImpl(n, p));
            }),
        std::make_pair(
            common::bitfieldStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new BitfieldFieldImpl(n, p));
            }),
        std::make_pair(
            common::bundleStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new BundleFieldImpl(n, p));
            }),
        std::make_pair(
            common::refStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new RefFieldImpl(n, p));
            })
    };

    return Map;
}

} // namespace bbmp
