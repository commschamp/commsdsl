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

#include "ParseRefFieldImpl.h"

#include "ParseBitfieldFieldImpl.h"
#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

namespace
{
const std::size_t BitsInByte =
        std::numeric_limits<std::uint8_t>::digits;
static_assert(BitsInByte == 8U, "Invalid assumption");  
} // namespace  

ParseRefFieldImpl::ParseRefFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

ParseRefFieldImpl::ParseRefFieldImpl(const ParseRefFieldImpl&) = default;

ParseFieldImpl::Kind ParseRefFieldImpl::parseKindImpl() const
{
    return Kind::Ref;
}

ParseFieldImpl::Ptr ParseRefFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseRefFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseRefFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::parseFieldStr(),
        common::parseBitLengthStr(),
    };

    return List;
}

bool ParseRefFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseRefFieldImpl&>(other);
    m_state = castedOther.m_state;
    m_field = castedOther.m_field;
    assert(m_field != nullptr);
    return true;
}

bool ParseRefFieldImpl::parseImpl()
{
    bool mustHaveField = (m_field == nullptr);
    if (!parseValidateSinglePropInstance(common::parseFieldStr(), mustHaveField)) {
        return false;
    }

    auto propsIter = parseProps().find(common::parseFieldStr());
    if (propsIter == parseProps().end()) {
        assert(m_field != nullptr);
        assert(!parseName().empty());
        return true;
    }

    m_field = parseProtocol().parseFindField(propsIter->second);
    if (m_field == nullptr) {
        parseReportUnexpectedPropertyValue(common::parseFieldStr(), propsIter->second);
        return false;
    }

    bool result = parseUpdateBitLength();
    if (!result) {
        return false;
    }

    if (parseName().empty()) {
        parseSetName(m_field->parseName());
    }

    if (parseDisplayName().empty() && (!m_field->parseDisplayName().empty())) {
        parseSetDisplayName(m_field->parseDisplayName());
    }

    if ((parseProtocol().parseIsSemanticTypeRefInheritanceSupported()) &&
        (parseSemanticType() == SemanticType::None) &&
        (m_field->parseSemanticType() != SemanticType::MessageId)) {
        parseSetSemanticType(m_field->parseSemanticType());
    }

    return true;
}

std::size_t ParseRefFieldImpl::parseMinLengthImpl() const
{
    assert(m_field != nullptr);
    return m_field->parseMinLength();
}

std::size_t ParseRefFieldImpl::parseMaxLengthImpl() const
{
    assert(m_field != nullptr);
    return m_field->parseMaxLength();
}

std::size_t ParseRefFieldImpl::parseBitLengthImpl() const
{
    if (parseIsBitfieldMember()) {
        return m_state.m_bitLength;
    }

    return Base::parseBitLengthImpl();
}

bool ParseRefFieldImpl::parseIsComparableToValueImpl(const std::string& val) const
{
    assert(m_field != nullptr);
    return m_field->parseIsComparableToValue(val);
}

bool ParseRefFieldImpl::parseIsComparableToFieldImpl(const ParseFieldImpl& field) const
{
    assert(m_field != nullptr);
    return m_field->parseIsComparableToField(field);
}

bool ParseRefFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return
        parseStrToValue(
            ref,
            [&val, &isBigUnsigned](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToNumeric(str, val, isBigUnsigned);
            });
}

bool ParseRefFieldImpl::parseStrToFpImpl(const std::string& ref, double& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToFp(str, val);
            });
    }

bool ParseRefFieldImpl::parseStrToBoolImpl(const std::string& ref, bool& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToBool(str, val);
            });
}

bool ParseRefFieldImpl::parseStrToStringImpl(const std::string& ref, std::string& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToString(str, val);
            });
}

bool ParseRefFieldImpl::parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToData(str, val);
            });
}

bool ParseRefFieldImpl::parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const
{
    assert(m_field != nullptr);
    return m_field->parseValidateBitLengthValue(node, bitLength);
}

bool ParseRefFieldImpl::parseVerifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const
{
    assert(m_field != nullptr);
    return m_field->parseVerifySemanticType(node, type);
}

ParseRefFieldImpl::FieldRefInfo ParseRefFieldImpl::parseProcessInnerRefImpl(const std::string& refStr) const
{
    assert(m_field != nullptr);
    return m_field->parseProcessInnerRef(refStr);
}

bool ParseRefFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    assert(m_field != nullptr);
    return m_field->parseIsValidRefType(type);
}

bool ParseRefFieldImpl::parseUpdateBitLength()
{
    if (!parseValidateSinglePropInstance(common::parseBitLengthStr())) {
        return false;
    }

    auto& valStr = common::parseGetStringProp(parseProps(), common::parseBitLengthStr());
    assert(0 < parseMaxLength());
    auto maxBitLength = parseMaxLength() * BitsInByte;
    if (valStr.empty()) {
        if (m_state.m_bitLength == 0) {
            m_state.m_bitLength = maxBitLength;
            return true;
        }

        assert(m_state.m_bitLength <= maxBitLength);
        return true;
    }    

    if (!parseIsBitfieldMember()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix((parseGetNode())) <<
                        "The property \"" << common::parseBitLengthStr() << "\" is "
                        "applicable only to the members of \"" << common::parseBitparseFieldStr() << "\"";
        return true;
    }    

    bool ok = false;
    m_state.m_bitLength = common::parseStrToUnsigned(valStr, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseBitLengthStr(), valStr);
        return false;
    }

    assert(m_field != nullptr);
    if (!m_field->parseValidateBitLengthValue(parseGetNode(), m_state.m_bitLength)) {
        return false;
    }

    return true;
}

bool ParseRefFieldImpl::parseStrToValue(
    const std::string& ref,
    StrToValueFieldConvertFunc&& forwardFunc) const
{
    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
        return false;
    }

    assert(m_field != nullptr);
    return forwardFunc(*m_field, ref);
}


} // namespace parse

} // namespace commsdsl
