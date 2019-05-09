//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "FieldImpl.h"

#include <cassert>
#include <limits>
#include <algorithm>
#include <set>
#include <iterator>

#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "FloatFieldImpl.h"
#include "EnumFieldImpl.h"
#include "SetFieldImpl.h"
#include "BitfieldFieldImpl.h"
#include "BundleFieldImpl.h"
#include "StringFieldImpl.h"
#include "DataFieldImpl.h"
#include "ListFieldImpl.h"
#include "RefFieldImpl.h"
#include "OptionalFieldImpl.h"
#include "VariantFieldImpl.h"
#include "NamespaceImpl.h"
#include "common.h"

namespace commsdsl
{

namespace {

const unsigned MinDslVersionForLengthSemanticType  = 2U;

} // namespace

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
        updateVersions() &&
        updateSemanticType() &&
        updatePseudo() &&
        updateDisplayReadOnly() &&
        updateDisplayHidden() &&
        updateCustomizable() &&
        updateFailOnInvalid();

    if (!result) {
        return false;
    }

    if (!parseImpl()) {
        return false;
    }

    if ((!verifySemanticType()) ||
        (!verifyName())) {
        return false;
    }

    XmlWrap::NamesList expectedProps = commonProps();
    expectedProps.insert(expectedProps.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedProps.insert(expectedProps.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    if (!updateExtraAttrs(expectedProps)) {
        return false;
    }

    auto& commonCh = commonChildren();
    auto& extraChildren = extraChildrenNamesImpl();
    XmlWrap::NamesList expectedChildren = commonProps();
    expectedChildren.insert(expectedChildren.end(), commonCh.begin(), commonCh.end());
    expectedChildren.insert(expectedChildren.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraChildren.begin(), extraChildren.end());
    if (!updateExtraChildren(expectedChildren)) {
        return false;
    }
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

const std::string& FieldImpl::kindStr() const
{
    static const std::string* const Map[] = {
        /* Int */ &common::intStr(),
        /* Enum */ &common::enumStr(),
        /* Set */ &common::setStr(),
        /* Float */ &common::floatStr(),
        /* Bitfield */ &common::bitfieldStr(),
        /* Bundle */ &common::bundleStr(),
        /* String */ &common::stringStr(),
        /* Data */ &common::dataStr(),
        /* List */ &common::listStr(),
        /* Ref */ &common::refStr(),
        /* Optional */ &common::optionalStr(),
        /* Variant */ &common::variantStr()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == (unsigned)Kind::NumOfValues, "Invalid map");

    auto idx = static_cast<unsigned>(kind());
    assert(idx < MapSize);
    return *Map[idx];
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

bool FieldImpl::validateMembersNames(
    const FieldImpl::FieldsList& fields,
    Logger& logger)
{
    std::set<std::string> usedNames;
    for (auto& f : fields) {
        if (usedNames.find(f->name()) != usedNames.end()) {
            commsdsl::logError(logger) << XmlWrap::logPrefix(f->getNode()) <<
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

bool FieldImpl::isBundleMember() const
{
    return (getParent() != nullptr) &&
           (getParent()->objKind() == ObjKind::Field) &&
            (static_cast<const FieldImpl*>(getParent())->kind() == Kind::Bundle);
}

bool FieldImpl::isMessageMember() const
{
    return (getParent() != nullptr) &&
           (getParent()->objKind() == ObjKind::Message);
}

std::string FieldImpl::externalRef() const
{
    if ((getParent() == nullptr) || (getParent()->objKind() != ObjKind::Namespace)) {
        return common::emptyString();
    }

    auto& ns = static_cast<const commsdsl::NamespaceImpl&>(*getParent());
    auto nsRef = ns.externalRef();
    if (nsRef.empty()) {
        return name();
    }

    return nsRef + '.' + name();
}

bool FieldImpl::isComparableToField(const FieldImpl& field) const
{
    if (field.kind() == Kind::Ref) {
        auto& refField = static_cast<const RefFieldImpl&>(field);
        auto* referee = refField.fieldImpl();
        if (referee == nullptr) {
            assert(!"BUG");
            return false;
        }

        return isComparableToField(*referee);
    }

    if (kind() == field.kind()) {
        return true;
    }

    return isComparableToFieldImpl(field);
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
    return commsdsl::logError(m_protocol.logger());
}

LogWrapper FieldImpl::logWarning() const
{
    return commsdsl::logWarning(m_protocol.logger());
}

LogWrapper FieldImpl::logInfo() const
{
    return commsdsl::logInfo(m_protocol.logger());
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

bool FieldImpl::verifySiblingsImpl(const FieldImpl::FieldsList& fields) const
{
    static_cast<void>(fields);
    return true;
}

std::size_t FieldImpl::maxLengthImpl() const
{
    return this->minLengthImpl();
}

std::size_t FieldImpl::bitLengthImpl() const
{
    return 0U;
}

bool FieldImpl::isComparableToValueImpl(const std::string& val) const
{
    static_cast<void>(val);
    return false;
}

bool FieldImpl::isComparableToFieldImpl(const FieldImpl& field) const
{
    static_cast<void>(field);
    return false;
}

bool FieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    static_cast<void>(ref);
    static_cast<void>(val);
    static_cast<void>(isBigUnsigned);

    if (protocol().isFieldValueReferenceSupported()) {
        logWarning() <<
            "Extracting integral numeric value from \"" << kindStr() <<
            "\" field is not supported.";
    }

    return false;
}

bool FieldImpl::strToFpImpl(const std::string& ref, double& val) const
{
    std::intmax_t intVal = 0;
    bool isBigUnsigned = false;
    if (!strToNumeric(ref, intVal, isBigUnsigned)) {
        return false;
    }

    if (!isBigUnsigned) {
        val = static_cast<double>(intVal);
        return true;
    }

    val = static_cast<double>(static_cast<std::uintmax_t>(intVal));
    return true;
}

bool FieldImpl::strToBoolImpl(const std::string& ref, bool& val) const
{
    static_cast<void>(ref);
    static_cast<void>(val);

    if (protocol().isFieldValueReferenceSupported()) {
        logWarning() <<
            "Extracting boolean value from \"" << kindStr() <<
            "\" field is not supported.";
    }

    return false;
}

bool FieldImpl::strToStringImpl(const std::string& ref, std::string& val) const
{
    static_cast<void>(ref);
    static_cast<void>(val);

    if (protocol().isFieldValueReferenceSupported()) {
        logWarning() <<
            "Extracting string value from \"" << kindStr() <<
            "\" field is not supported.";
    }

    return false;
}

bool FieldImpl::strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    static_cast<void>(ref);
    static_cast<void>(val);

    if (protocol().isFieldValueReferenceSupported()) {
        logWarning() <<
            "Extracting data value from \"" << kindStr() <<
            "\" field is not supported.";
    }

    return false;
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

bool FieldImpl::validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave)
{
    if (!validateSinglePropInstance(propName, mustHave)) {
        return false;
    }

    auto iter = m_props.find(propName);
    if (iter == m_props.end()) {
        return true;
    }

    bool ok = false;
    value = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

const XmlWrap::NamesList& FieldImpl::commonProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::displayNameStr(),
        common::descriptionStr(),
        common::sinceVersionStr(),
        common::deprecatedStr(),
        common::removedStr(),
        common::reuseStr(),
        common::semanticTypeStr(),
        common::pseudoStr(),
        common::displayReadOnlyStr(),
        common::displayHiddenStr(),
        common::customizableStr(),
        common::failOnInvalidStr()
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

const FieldImpl* FieldImpl::findSibling(const FieldsList& fields, const std::string& sibName) const
{
    auto iter =
        std::find_if(
            fields.begin(), fields.end(),
            [&sibName](auto& f)
            {
                return f->name() == sibName;
            });

    if (iter == fields.end()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The holding bundle/message does not contain field named \"" <<
            sibName << "\".";

        return nullptr;
    }

    return iter->get();
}

FieldImpl::Kind FieldImpl::getNonRefFieldKind(const FieldImpl& field)
{
    const FieldImpl* referee = &field;
    while (referee->kind() == Kind::Ref) {
        auto* castedField = static_cast<const RefFieldImpl*>(referee);
        referee = castedField->fieldImpl();
        assert(referee != nullptr);
    }

    return referee->kind();
}

bool FieldImpl::checkDetachedPrefixAllowed() const
{
    auto* parent = getParent();
    bool result = false;
    do {
        if (parent == nullptr) {
            break;
        }

        auto objK = parent->objKind();
        if (objK == ObjKind::Message) {
            result = true;
            break;
        }

        if (objK != ObjKind::Field) {
            break;
        }

        auto* parentField = static_cast<const FieldImpl*>(parent);
        result = (parentField->kind() == Kind::Bundle);
    } while (false);

    if (!result) {
        logError() << XmlWrap::logPrefix(m_node) <<
            "Detached prefixes are allowed only for members of \"" << common::bundleStr() << "\" field "
            "or \"" << common::messageStr() << "\" object.";
    }
    return result;
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
    assert(getDeprecated() == commsdsl::Protocol::notYetDeprecated());
    assert(!isDeprecatedRemoved());
    return reuseImpl(*field);
}

bool FieldImpl::updateName()
{
    return validateAndUpdateStringPropValue(common::nameStr(), m_state.m_name);
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

    unsigned deprecated = commsdsl::Protocol::notYetDeprecated();
    if ((getParent() != nullptr) && (getParent()->objKind() != ObjKind::Namespace)) {
        deprecated = getParent()->getDeprecated();
    }

    if (!XmlWrap::getAndCheckVersions(m_node, name(), m_props, sinceVersion, deprecated, protocol())) {
        return false;
    }

    do {
        if ((getParent() != nullptr) &&
            ((getParent()->objKind() == ObjKind::Field) || (getParent()->objKind() == ObjKind::Message))) {
            break;
        }

        if (sinceVersion != 0U) {
            logWarning() << XmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::sinceVersionStr() << "\" is not applicable to "
                "this field, ignoring provided value";
            sinceVersion = 0U;
        }

        if (deprecated != commsdsl::Protocol::notYetDeprecated()) {
            logWarning() << XmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::deprecatedStr() << "\" is not applicable to "
                "this field, ignoring provided value";
            deprecated = commsdsl::Protocol::notYetDeprecated();
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

        if (deprecated == commsdsl::Protocol::notYetDeprecated()) {
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

bool FieldImpl::updateSemanticType()
{
    if (!validateSinglePropInstance(common::semanticTypeStr())) {
        return false;
    }

    auto iter = m_props.find(common::semanticTypeStr());
    if (iter == m_props.end()) {
        return true;
    }

    static const std::string Map[] = {
        /* None */ common::noneStr(),
        /* Version */ common::versionStr(),
        /* MessageId */ common::toLowerCopy(common::messageIdStr()),
        /* Length */ common::lengthStr(),
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<std::size_t>(SemanticType::NumOfValues),
        "Invalid map");

    if (iter->second.empty()) {
        m_state.m_semanticType = SemanticType::None;
        return true;
    }

    auto typeVal = common::toLowerCopy(iter->second);
    auto valIter = std::find(std::begin(Map), std::end(Map), typeVal);
    if (valIter == std::end(Map)) {
        reportUnexpectedPropertyValue(common::semanticTypeStr(), iter->second);
        return false;
    }

    m_state.m_semanticType =
        static_cast<SemanticType>(std::distance(std::begin(Map), valIter));

    return true;
}

bool FieldImpl::updatePseudo()
{
    return validateAndUpdateBoolPropValue(common::pseudoStr(), m_state.m_pseudo);
}

bool FieldImpl::updateDisplayReadOnly()
{
    return validateAndUpdateBoolPropValue(common::displayReadOnlyStr(), m_state.m_displayReadOnly);
}

bool FieldImpl::updateDisplayHidden()
{
    return validateAndUpdateBoolPropValue(common::displayHiddenStr(), m_state.m_displayHidden);
}

bool FieldImpl::updateCustomizable()
{
    return validateAndUpdateBoolPropValue(common::customizableStr(), m_state.m_customizable);
}

bool FieldImpl::updateFailOnInvalid()
{
    return validateAndUpdateBoolPropValue(common::failOnInvalidStr(), m_state.m_failOnInvalid);
}

bool FieldImpl::updateExtraAttrs(const XmlWrap::NamesList& names)
{
    auto extraAttrs = XmlWrap::getExtraAttributes(m_node, names, m_protocol);
    if (extraAttrs.empty()) {
        return true;
    }

    if (m_state.m_extraAttrs.empty()) {
        m_state.m_extraAttrs = std::move(extraAttrs);
        return true;
    }

    std::move(extraAttrs.begin(), extraAttrs.end(), std::inserter(m_state.m_extraAttrs, m_state.m_extraAttrs.end()));
    return true;
}

bool FieldImpl::updateExtraChildren(const XmlWrap::NamesList& names)
{
    auto extraChildren = XmlWrap::getExtraChildren(m_node, names, m_protocol);
    if (extraChildren.empty()) {
        return true;
    }

    if (m_state.m_extraChildren.empty()) {
        m_state.m_extraChildren = std::move(extraChildren);
        return true;
    }

    m_state.m_extraChildren.reserve(m_state.m_extraChildren.size() + extraChildren.size());
    std::move(extraChildren.begin(), extraChildren.end(), std::back_inserter(m_state.m_extraChildren));
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
            common::stringStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new StringFieldImpl(n, p));
            }),
        std::make_pair(
            common::dataStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new DataFieldImpl(n, p));
            }),
        std::make_pair(
            common::listStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new ListFieldImpl(n, p));
            }),
        std::make_pair(
            common::refStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new RefFieldImpl(n, p));
            }),
        std::make_pair(
            common::optionalStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new OptionalFieldImpl(n, p));
            }),
        std::make_pair(
            common::variantStr(),
            [](::xmlNodePtr n, ProtocolImpl& p)
            {
                return Ptr(new VariantFieldImpl(n, p));
            })
    };

    return Map;
}

bool FieldImpl::verifySemanticType() const
{
    if (semanticType() == SemanticType::None) {
        return true;
    }

    if (semanticType() == SemanticType::Version) {
        if (kind() != Kind::Int) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Semantic type \"" << common::versionStr() << "\" is applicable only to " <<
                "integral value fields.";
            return false;
        }

        return true;
    }

    if (semanticType() == SemanticType::MessageId) {
        if (kind() != Kind::Enum) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Semantic type \"" << common::messageIdStr() << "\" is applicable only to \"" <<
                common::enumStr() << "\" fields.";
            return false;
        }
        return true;
    }

    if (semanticType() == SemanticType::Length) {
        if (!m_protocol.isFeatureSupported(MinDslVersionForLengthSemanticType)) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Semantic type \"" << common::lengthStr() << "\" supported only since "
                "DSL v" << MinDslVersionForLengthSemanticType << ", please update \"" <<
                common::dslVersionStr() << "\" property of your schema.";
            return false;
        }

        if (kind() != Kind::Int) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Semantic type \"" << common::lengthStr() << "\" is applicable only to \"" <<
                common::intStr() << "\" fields.";
            return false;
        }


        auto parent = getParent();
        if ((parent == nullptr) ||
            (parent->objKind() != ObjKind::Field) ||
            (static_cast<const FieldImpl*>(parent)->kind() != Kind::Bundle)) {

            logError() << XmlWrap::logPrefix(getNode()) <<
                "Semantic type \"" << common::lengthStr() << "\" is applicable only to "
                "members of \"" << common::bundleStr() << "\" field.";

            return false;
        }

        return true;
    }

    assert(!"Unhandled semantic type, please fix");
    return true;
}

bool FieldImpl::verifyName() const
{
    assert(m_state.m_name != nullptr);
    if (m_state.m_name->empty()) {
        logError() << XmlWrap::logPrefix(m_node) <<
            "Missing value for mandatory property \"" << common::nameStr() << "\" for \"" << m_node->name << "\" element.";
        return false;
    }

    if (!common::isValidName(*m_state.m_name)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                "Invalid value for name property \"" << *m_state.m_name << "\".";
        return false;
    }

    return true;
}

} // namespace commsdsl
