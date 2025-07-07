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

#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <map>
#include <type_traits>

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::ParseNamesList& parseOptionalSupportedTypes()
{
    static const ParseXmlWrap::ParseNamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

ParseXmlWrap::ParseNamesList parseGetExtraNames()
{
    auto names = parseOptionalSupportedTypes();
    names.push_back(common::parseFieldStr());
    names.push_back(common::parseAndStr());
    names.push_back(common::parseOrStr());
    return names;
}

} // namespace

ParseOptionalFieldImpl::ParseOptionalFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}


ParseFieldImpl::ParseKind ParseOptionalFieldImpl::parseKindImpl() const
{
    return ParseKind::Optional;
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

ParseFieldImpl::ParsePtr ParseOptionalFieldImpl::parseCloneImpl() const
{
    return ParsePtr(new ParseOptionalFieldImpl(*this));
}

const ParseXmlWrap::ParseNamesList& ParseOptionalFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseDefaultModeStr(),
        common::parseCondStr(),
        common::parseDisplayExtModeCtrlStr(),
        common::parseMissingOnReadFailStr(),
        common::parseMissingOnInvalparseIdStr(),
    };

    return List;
}

const ParseXmlWrap::ParseNamesList&ParseOptionalFieldImpl::parseExtraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseFieldStr(),
    };

    return List;
}

const ParseXmlWrap::ParseNamesList& ParseOptionalFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = parseGetExtraNames();
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

bool ParseOptionalFieldImpl::parseVerifySiblingsImpl(const ParseFieldsList& fields) const
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

ParseOptionalFieldImpl::ParseFieldRefInfo ParseOptionalFieldImpl::parseProcessInnerRefImpl(const std::string& refStr) const
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
        return ParseFieldRefInfo();
    }

    return m_field->parseProcessInnerRef(restStr);
}

bool ParseOptionalFieldImpl::parseIsValidRefTypeImpl(ParseFieldRefType type) const
{
    return (type == FieldRefType_Exists);
}

bool ParseOptionalFieldImpl::parseUpdateMode()
{
    if (!parseValidateSinglePropInstance(common::parseDefaultModeStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseDefaultModeStr());
    if (iter == parseProps().end()) {
        return true;
    }

    static const std::map<std::string, ParseMode> Map = {
        std::make_pair("tent", ParseMode::Tentative),
        std::make_pair("tentative", ParseMode::Tentative),
        std::make_pair("t", ParseMode::Tentative),
        std::make_pair("miss", ParseMode::Missing),
        std::make_pair("missing", ParseMode::Missing),
        std::make_pair("m", ParseMode::Missing),
        std::make_pair("exists", ParseMode::Exists),
        std::make_pair("exist", ParseMode::Exists),
        std::make_pair("e", ParseMode::Exists),
    };

    auto modeStr = common::parseToLowerCopy(iter->second);
    auto mapIter = Map.find(modeStr);
    if (mapIter == Map.end()) {
        parseReportUnexpectedPropertyValue(common::parseDefaultModeStr(), iter->second);
        return false;
    }

    m_state.m_mode = mapIter->second;
    return true;
}

bool ParseOptionalFieldImpl::parseUpdateExternalModeCtrl()
{
    parseCheckAndReportDeprecatedPropertyValue(common::parseDisplayExtModeCtrlStr());
    return true;
}

bool ParseOptionalFieldImpl::parseUpdateMissingOnReadFail()
{
    return parseValidateAndUpdateBoolPropValue(common::parseMissingOnReadFailStr(), m_state.m_missingOnReadFail);
}

bool ParseOptionalFieldImpl::parseUpdateMissingOnInvalid()
{
    return parseValidateAndUpdateBoolPropValue(common::parseMissingOnInvalparseIdStr(), m_state.m_missingOnInvalid);
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
    if (!parseValidateSinglePropInstance(common::parseCondStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseCondStr());
    if (iter == parseProps().end()) {
        return true;
    }

    if ((!parseIsBundleMember()) && (!parseIsMessageMember())) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Condition for existing mode are applicable only to members of \"" <<
            common::parseBundleStr() << "\" and \"" << common::parseMessageStr() << "\".";
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
    static const ParseXmlWrap::ParseNamesList ElemNames = {
        common::parseAndStr(),
        common::parseOrStr()
    };

    auto multiChildren = ParseXmlWrap::parseGetChildren(parseGetNode(), ElemNames);
    if (multiChildren.empty()) {
        return true;
    }

    if (parseProps().find(common::parseCondStr()) != parseProps().end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(multiChildren.front()) <<
            "Cannot use \"" << multiChildren.front()->name << "\" condition bundling together with \"" <<
            common::parseCondStr() << "\" property.";
        return false;
    }

    if (1U < multiChildren.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(multiChildren.front()) <<
            "Cannot use more that one \"" << common::parseAndStr() << "\" or \"" <<
            common::parseOrStr() << "\" element.";
        return false;
    }

    auto iter = parseProps().find(common::parseCondStr());
    if (iter != parseProps().end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(multiChildren.front()) <<
            "Multiple definitions of existance conditions are not allowed";
        return false;
    }

    auto newCond = std::make_unique<ParseOptCondListImpl>();
    if (!newCond->parse(multiChildren.front(), parseProtocol())) {
        return false;
    }

    assert(newCond->parseKind() == ParseOptCondImpl::ParseKind::List);
    m_cond = std::move(newCond);
    return true;
}

bool ParseOptionalFieldImpl::parseCheckFieldFromRef()
{
    if (!parseValidateSinglePropInstance(common::parseFieldStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseFieldStr());
    if (iter == parseProps().end()) {
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << common::parseFieldStr() <<
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
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseFieldStr());
    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << common::parseFieldStr() << "\" child element.";
        return false;
    }

    auto fieldTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseOptionalSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The \"" << common::parseOptionalStr() << "\" element does not support "
                  "stand alone field as child element together with \"" <<
                  common::parseFieldStr() << "\" child element.";
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
                "The \"" << common::parseOptionalStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
        if (allChildren.size() != fieldTypes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The field type of \"" << common::parseOptionalStr() <<
                  "\" must be defined inside \"<" << common::parseFieldsStr() << ">\" child element "
                  "when there are other property describing children.";
            return false;
        }

        if (parseProps().find(common::parseFieldStr()) != parseProps().end()) {
            parseLogError() << "There must be only one occurance of \"" << common::parseFieldStr() << "\" definition.";
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
                "The \"" << common::parseFieldStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (parseProps().find(common::parseFieldStr()) == parseProps().end()) {
            fieldNode = fields.front();
            break;
        }

        auto attrs = ParseXmlWrap::parseNodeProps(parseGetNode());
        if (attrs.find(common::parseFieldsStr()) != attrs.end()) {
            parseLogError() << "There must be only one occurance of \"" << common::parseFieldStr() << "\" definition.";
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
    ParseStrToValueFieldConvertFunc&& forwardFunc) const
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
