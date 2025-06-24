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

#include "ParseOptionalFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>
#include <map>

#include "parse_common.h"
#include "ParseProtocolImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::NamesList& parseOptionalSupportedTypes()
{
    static const ParseXmlWrap::NamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

ParseXmlWrap::NamesList parseGetExtraNames()
{
    auto names = parseOptionalSupportedTypes();
    names.push_back(common::fieldStr());
    names.push_back(common::andStr());
    names.push_back(common::orStr());
    return names;
}

} // namespace

ParseOptionalFieldImpl::ParseOptionalFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}


ParseFieldImpl::Kind ParseOptionalFieldImpl::parseKindImpl() const
{
    return Kind::Optional;
}

ParseOptionalFieldImpl::ParseOptionalFieldImpl(const ParseOptionalFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_field) {
        assert(other.m_state.m_extField == nullptr);
        m_field = other.m_field->parseClone();
    }

    if (other.m_cond) {
        m_cond = other.m_cond->parseClone();
    }
}

ParseFieldImpl::Ptr ParseOptionalFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseOptionalFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseOptionalFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::defaultModeStr(),
        common::condStr(),
        common::displayExtModeCtrlStr(),
        common::missingOnReadFailStr(),
        common::missingOnInvalidStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList&ParseOptionalFieldImpl::parseExtraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::fieldStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseOptionalFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = parseGetExtraNames();
    return List;
}

bool ParseOptionalFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseOptionalFieldImpl&>(other);
    m_state = castedOther.m_state;
    if (castedOther.m_field) {
        assert(m_state.m_extField == nullptr);
        m_field = castedOther.m_field->parseClone();
    }
    else {
        assert(!m_field);
    }

    if (castedOther.m_cond) {
        m_cond = castedOther.m_cond->parseClone();
    }

    return true;
}

bool ParseOptionalFieldImpl::parseImpl()
{
    return
        parseUpdateMode() &&
        parseUpdateExternalModeCtrl() &&
        parseUpdateMissingOnReadFail() &&
        parseUpdateMissingOnInvalid() &&
        parseUpdateField() &&
        parseUpdateSingleCondition() &&
        parseUpdateMultiCondition();
}

bool ParseOptionalFieldImpl::parseVerifySiblingsImpl(const FieldsList& fields) const
{
    auto& c = parseCond();
    if (!c) {
        return true;
    }

    return c->parseVerify(fields, parseGetNode(), parseProtocol());
}

std::size_t ParseOptionalFieldImpl::parseMinLengthImpl() const
{
    return 0U;
}

std::size_t ParseOptionalFieldImpl::parseMaxLengthImpl() const
{
    assert(parseHasField());
    return parseGetField()->parseMaxLength();
}

bool ParseOptionalFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    if (ref.empty()) {
        return Base::parseStrToNumericImpl(ref, val, isBigUnsigned);
    }

    return
        parseStrToValue(
            ref,
            [&val, &isBigUnsigned](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToNumeric(str, val, isBigUnsigned);
            });
}

bool ParseOptionalFieldImpl::parseStrToFpImpl(const std::string& ref, double& val) const
{
    if (ref.empty()) {
        return Base::parseStrToFpImpl(ref, val);
    }

    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToFp(str, val);
            });
    }

bool ParseOptionalFieldImpl::parseStrToBoolImpl(const std::string& ref, bool& val) const
{
    if (ref.empty()) {
        return Base::parseStrToBoolImpl(ref, val);
    }

    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToBool(str, val);
            });
}

bool ParseOptionalFieldImpl::parseStrToStringImpl(const std::string& ref, std::string& val) const
{
    if (ref.empty()) {
        return Base::parseStrToStringImpl(ref, val);
    }

    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToString(str, val);
            });
}

bool ParseOptionalFieldImpl::parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    if (ref.empty()) {
        return Base::parseStrToDataImpl(ref, val);
    }

    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToData(str, val);
            });
}

ParseOptionalFieldImpl::FieldRefInfo ParseOptionalFieldImpl::parseProcessInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());

    auto fieldName = refStr;
    std::string restStr;
    auto sepPos = refStr.find('.');
    if (sepPos < refStr.size()) {
        fieldName = refStr.substr(0, sepPos);
        restStr = refStr.substr(sepPos + 1);
    }

    if (fieldName != m_field->parseName()) {
        return FieldRefInfo();
    }

    return m_field->parseProcessInnerRef(restStr);
}

bool ParseOptionalFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_Exists);
}

bool ParseOptionalFieldImpl::parseUpdateMode()
{
    if (!parseValidateSinglePropInstance(common::defaultModeStr())) {
        return false;
    }

    auto iter = parseProps().find(common::defaultModeStr());
    if (iter == parseProps().end()) {
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
        parseReportUnexpectedPropertyValue(common::defaultModeStr(), iter->second);
        return false;
    }

    m_state.m_mode = mapIter->second;
    return true;
}

bool ParseOptionalFieldImpl::parseUpdateExternalModeCtrl()
{
    parseCheckAndReportDeprecatedPropertyValue(common::displayExtModeCtrlStr());
    return true;
}

bool ParseOptionalFieldImpl::parseUpdateMissingOnReadFail()
{
    return parseValidateAndUpdateBoolPropValue(common::missingOnReadFailStr(), m_state.m_missingOnReadFail);
}

bool ParseOptionalFieldImpl::parseUpdateMissingOnInvalid()
{
    return parseValidateAndUpdateBoolPropValue(common::missingOnInvalidStr(), m_state.m_missingOnInvalid);
}

bool ParseOptionalFieldImpl::parseUpdateField()
{
    if ((!parseCheckFieldFromRef()) ||
        (!parseCheckFieldAsChild())) {
        return false;
    }

    if (!parseHasField()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Field itself hasn't been provided.";
        return false;
    }

    return true;
}

bool ParseOptionalFieldImpl::parseUpdateSingleCondition()
{
    if (!parseValidateSinglePropInstance(common::condStr())) {
        return false;
    }

    auto iter = parseProps().find(common::condStr());
    if (iter == parseProps().end()) {
        return true;
    }

    if ((!parseIsBundleMember()) && (!parseIsMessageMember())) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Condition for existing mode are applicable only to members of \"" <<
            common::bundleStr() << "\" and \"" << common::messageStr() << "\".";
    }

    auto cond = std::make_unique<ParseOptCondExprImpl>();
    if (!cond->parse(iter->second, parseGetNode(), parseProtocol())) {
        return false;
    }

    m_cond = std::move(cond);
    return true;
}

bool ParseOptionalFieldImpl::parseUpdateMultiCondition()
{
    static const ParseXmlWrap::NamesList ElemNames = {
        common::andStr(),
        common::orStr()
    };

    auto multiChildren = ParseXmlWrap::parseGetChildren(parseGetNode(), ElemNames);
    if (multiChildren.empty()) {
        return true;
    }

    if (parseProps().find(common::condStr()) != parseProps().end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(multiChildren.front()) <<
            "Cannot use \"" << multiChildren.front()->name << "\" condition bundling together with \"" <<
            common::condStr() << "\" property.";
        return false;
    }

    if (1U < multiChildren.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(multiChildren.front()) <<
            "Cannot use more that one \"" << common::andStr() << "\" or \"" <<
            common::orStr() << "\" element.";
        return false;
    }

    auto iter = parseProps().find(common::condStr());
    if (iter != parseProps().end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(multiChildren.front()) <<
            "Multiple definitions of existance conditions are not allowed";
        return false;
    }

    auto newCond = std::make_unique<ParseOptCondListImpl>();
    if (!newCond->parse(multiChildren.front(), parseProtocol())) {
        return false;
    }

    assert(newCond->parseKind() == ParseOptCondImpl::Kind::List);
    m_cond = std::move(newCond);
    return true;
}

bool ParseOptionalFieldImpl::parseCheckFieldFromRef()
{
    if (!parseValidateSinglePropInstance(common::fieldStr())) {
        return false;
    }

    auto iter = parseProps().find(common::fieldStr());
    if (iter == parseProps().end()) {
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << common::fieldStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_field.reset();
    m_state.m_extField = field;
    assert(parseHasField());
    return true;
}

bool ParseOptionalFieldImpl::parseCheckFieldAsChild()
{
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::fieldStr());
    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << common::fieldStr() << "\" child element.";
        return false;
    }

    auto fieldTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseOptionalSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
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
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "The \"" << common::optionalStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
        if (allChildren.size() != fieldTypes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The field type of \"" << common::optionalStr() <<
                  "\" must be defined inside \"<" << common::fieldsStr() << ">\" child element "
                  "when there are other property describing children.";
            return false;
        }

        if (parseProps().find(common::fieldStr()) != parseProps().end()) {
            parseLogError() << "There must be only one occurance of \"" << common::fieldStr() << "\" definition.";
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
        auto fields = ParseXmlWrap::parseGetChildren(child);
        if (1U < fields.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
                "The \"" << common::fieldStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (parseProps().find(common::fieldStr()) == parseProps().end()) {
            fieldNode = fields.front();
            break;
        }

        auto attrs = ParseXmlWrap::parseNodeProps(parseGetNode());
        if (attrs.find(common::fieldsStr()) != attrs.end()) {
            parseLogError() << "There must be only one occurance of \"" << common::fieldStr() << "\" definition.";
            return false;
        }

        // The field element is parsed as property
        return true;
    } while (false);

    assert (fieldNode != nullptr);

    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = ParseFieldImpl::parseCreate(fieldKind, fieldNode, parseProtocol());
    if (!field) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Unknown field type \"" << fieldKind;
        return false;
    }

    field->parseSetParent(this);
    if (!field->parse()) {
        return false;
    }

    m_state.m_extField = nullptr;
    m_field = std::move(field);
    assert(m_field->parseExternalRef(false).empty());
    return true;
}

const ParseFieldImpl* ParseOptionalFieldImpl::parseGetField() const
{
    if (m_state.m_extField != nullptr) {
        assert(!m_field);
        return m_state.m_extField;
    }

    assert(m_field);
    return m_field.get();
}

bool ParseOptionalFieldImpl::parseStrToValue(
    const std::string& ref,
    StrToValueFieldConvertFunc&& forwardFunc) const
{
    assert(!ref.empty());
    if ((!parseProtocol().parseIsFieldValueReferenceSupported()) ||
        (!m_field)) {
        return false;
    }

    auto firstDotPos = ref.find_first_of('.');
    std::string firstName(ref, 0, firstDotPos);
    if (m_field->parseName() != firstName) {
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
