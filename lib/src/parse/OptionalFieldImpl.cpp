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

#include "OptionalFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>
#include <map>

#include "common.h"
#include "ProtocolImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const XmlWrap::NamesList& optionalSupportedTypes()
{
    static const XmlWrap::NamesList Names = FieldImpl::supportedTypes();
    return Names;
}

XmlWrap::NamesList getExtraNames()
{
    auto names = optionalSupportedTypes();
    names.push_back(common::fieldStr());
    names.push_back(common::andStr());
    names.push_back(common::orStr());
    return names;
}

} // namespace

OptionalFieldImpl::OptionalFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind OptionalFieldImpl::kindImpl() const
{
    return Kind::Optional;
}

OptionalFieldImpl::OptionalFieldImpl(const OptionalFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_field) {
        assert(other.m_state.m_extField == nullptr);
        m_field = other.m_field->clone();
    }

    if (other.m_cond) {
        m_cond = other.m_cond->clone();
    }
}

FieldImpl::Ptr OptionalFieldImpl::cloneImpl() const
{
    return Ptr(new OptionalFieldImpl(*this));
}

const XmlWrap::NamesList& OptionalFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::defaultModeStr(),
        common::condStr(),
        common::displayExtModeCtrlStr(),
        common::missingOnReadFailStr(),
        common::missingOnInvalidStr(),
    };

    return List;
}

const XmlWrap::NamesList&OptionalFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::fieldStr(),
    };

    return List;
}

const XmlWrap::NamesList& OptionalFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = getExtraNames();
    return List;
}

bool OptionalFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const OptionalFieldImpl&>(other);
    m_state = castedOther.m_state;
    if (castedOther.m_field) {
        assert(m_state.m_extField == nullptr);
        m_field = castedOther.m_field->clone();
    }
    else {
        assert(!m_field);
    }
    return true;
}

bool OptionalFieldImpl::parseImpl()
{
    return
        updateMode() &&
        updateExternalModeCtrl() &&
        updateMissingOnReadFail() &&
        updateMissingOnInvalid() &&
        updateField() &&
        updateSingleCondition() &&
        updateMultiCondition();
}

bool OptionalFieldImpl::verifySiblingsImpl(const FieldsList& fields) const
{
    auto& c = cond();
    if (!c) {
        return true;
    }

    return c->verify(fields, getNode(), protocol());
}

std::size_t OptionalFieldImpl::minLengthImpl() const
{
    return 0U;
}

std::size_t OptionalFieldImpl::maxLengthImpl() const
{
    assert(hasField());
    return getField()->maxLength();
}

bool OptionalFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (ref.empty()) {
        return Base::strToNumericImpl(ref, val, isBigUnsigned);
    }

    return
        strToValue(
            ref,
            [&val, &isBigUnsigned](const FieldImpl& f, const std::string& str)
            {
                return f.strToNumeric(str, val, isBigUnsigned);
            });
}

bool OptionalFieldImpl::strToFpImpl(const std::string& ref, double& val) const
{
    if (ref.empty()) {
        return Base::strToFpImpl(ref, val);
    }

    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToFp(str, val);
            });
    }

bool OptionalFieldImpl::strToBoolImpl(const std::string& ref, bool& val) const
{
    if (ref.empty()) {
        return Base::strToBoolImpl(ref, val);
    }

    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToBool(str, val);
            });
}

bool OptionalFieldImpl::strToStringImpl(const std::string& ref, std::string& val) const
{
    if (ref.empty()) {
        return Base::strToStringImpl(ref, val);
    }

    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToString(str, val);
            });
}

bool OptionalFieldImpl::strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    if (ref.empty()) {
        return Base::strToDataImpl(ref, val);
    }

    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToData(str, val);
            });
}

OptionalFieldImpl::FieldRefInfo OptionalFieldImpl::processInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());

    auto fieldName = refStr;
    std::string restStr;
    auto sepPos = refStr.find('.');
    if (sepPos < refStr.size()) {
        fieldName = refStr.substr(0, sepPos);
        restStr = refStr.substr(sepPos + 1);
    }

    if (fieldName != m_field->name()) {
        return FieldRefInfo();
    }

    return m_field->processInnerRef(restStr);
}

bool OptionalFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_Exists);
}

bool OptionalFieldImpl::updateMode()
{
    if (!validateSinglePropInstance(common::defaultModeStr())) {
        return false;
    }

    auto iter = props().find(common::defaultModeStr());
    if (iter == props().end()) {
        return true;
    }

    static const std::map<std::string, Mode> Map = {
        std::make_pair("tent", Mode::Tentative),
        std::make_pair("tentative", Mode::Tentative),
        std::make_pair("t", Mode::Tentative),
        std::make_pair("miss", Mode::Missing),
        std::make_pair("missing", Mode::Missing),
        std::make_pair("m", Mode::Missing),
        std::make_pair("exists", Mode::Exists),
        std::make_pair("exist", Mode::Exists),
        std::make_pair("e", Mode::Exists),
    };

    auto modeStr = common::toLowerCopy(iter->second);
    auto mapIter = Map.find(modeStr);
    if (mapIter == Map.end()) {
        reportUnexpectedPropertyValue(common::defaultModeStr(), iter->second);
        return false;
    }

    m_state.m_mode = mapIter->second;
    return true;
}

bool OptionalFieldImpl::updateExternalModeCtrl()
{
    return validateAndUpdateBoolPropValue(common::displayExtModeCtrlStr(), m_state.m_externalModeCtrl);
}

bool OptionalFieldImpl::updateMissingOnReadFail()
{
    return validateAndUpdateBoolPropValue(common::missingOnReadFailStr(), m_state.m_missingOnReadFail);
}

bool OptionalFieldImpl::updateMissingOnInvalid()
{
    return validateAndUpdateBoolPropValue(common::missingOnInvalidStr(), m_state.m_missingOnInvalid);
}

bool OptionalFieldImpl::updateField()
{
    if ((!checkFieldFromRef()) ||
        (!checkFieldAsChild())) {
        return false;
    }

    if (!hasField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Field itself hasn't been provided.";
        return false;
    }

    return true;
}

bool OptionalFieldImpl::updateSingleCondition()
{
    if (!validateSinglePropInstance(common::condStr())) {
        return false;
    }

    auto iter = props().find(common::condStr());
    if (iter == props().end()) {
        return true;
    }

    if (m_cond) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Overriding non-empty condition(s) is not allowed";
        return false;
    }

    if ((!isBundleMember()) && (!isMessageMember())) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "Condition for existing mode are applicable only to members of \"" <<
            common::bundleStr() << "\" and \"" << common::messageStr() << "\".";
    }

    auto cond = std::make_unique<OptCondExprImpl>();
    if (!cond->parse(iter->second, getNode(), protocol())) {
        return false;
    }

    m_cond = std::move(cond);
    return true;
}

bool OptionalFieldImpl::updateMultiCondition()
{
    static const XmlWrap::NamesList ElemNames = {
        common::andStr(),
        common::orStr()
    };

    auto multiChildren = XmlWrap::getChildren(getNode(), ElemNames);
    if (multiChildren.empty()) {
        return true;
    }

    if (props().find(common::condStr()) != props().end()) {
        logError() << XmlWrap::logPrefix(multiChildren.front()) <<
            "Cannot use \"" << multiChildren.front()->name << "\" condition bundling together with \"" <<
            common::condStr() << "\" property.";
        return false;
    }

    if (1U < multiChildren.size()) {
        logError() << XmlWrap::logPrefix(multiChildren.front()) <<
            "Cannot use more that one \"" << common::andStr() << "\" or \"" <<
            common::orStr() << "\" element.";
        return false;
    }

    if (m_cond) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Overriding non-empty condition(s) is not allowed";
        return false;
    }

    auto newCond = std::make_unique<OptCondListImpl>();
    if (!newCond->parse(multiChildren.front(), protocol())) {
        return false;
    }

    assert(newCond->kind() == OptCondImpl::Kind::List);
    m_cond = std::move(newCond);
    return true;
}

bool OptionalFieldImpl::checkFieldFromRef()
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

    m_field.reset();
    m_state.m_extField = field;
    assert(hasField());
    return true;
}

bool OptionalFieldImpl::checkFieldAsChild()
{
    auto children = XmlWrap::getChildren(getNode(), common::fieldStr());
    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::fieldStr() << "\" child element.";
        return false;
    }

    auto fieldTypes = XmlWrap::getChildren(getNode(), optionalSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                  "The \"" << common::optionalStr() << "\" element does not support "
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
                "The \"" << common::optionalStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        auto allChildren = XmlWrap::getChildren(getNode());
        if (allChildren.size() != fieldTypes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                  "The field type of \"" << common::optionalStr() <<
                  "\" must be defined inside \"<" << common::fieldsStr() << ">\" child element "
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
            "Unknown field type \"" << fieldKind;
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_state.m_extField = nullptr;
    m_field = std::move(field);
    assert(m_field->externalRef(false).empty());
    return true;
}

const FieldImpl* OptionalFieldImpl::getField() const
{
    if (m_state.m_extField != nullptr) {
        assert(!m_field);
        return m_state.m_extField;
    }

    assert(m_field);
    return m_field.get();
}

bool OptionalFieldImpl::strToValue(
    const std::string& ref,
    StrToValueFieldConvertFunc&& forwardFunc) const
{
    assert(!ref.empty());
    if ((!protocol().isFieldValueReferenceSupported()) ||
        (!m_field)) {
        return false;
    }

    auto firstDotPos = ref.find_first_of('.');
    std::string firstName(ref, 0, firstDotPos);
    if (m_field->name() != firstName) {
        return false;
    }

    std::string restName;
    if (firstDotPos != std::string::npos) {
        restName.assign(ref, firstDotPos + 1, std::string::npos);
    }

    return forwardFunc(*m_field, restName);
}

} // namespace parse

} // namespace commsdsl
