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

#include "RefFieldImpl.h"

#include <cassert>
#include "common.h"
#include "ProtocolImpl.h"

namespace commsdsl
{

namespace
{
const std::size_t BitsInByte =
        std::numeric_limits<std::uint8_t>::digits;
static_assert(BitsInByte == 8U, "Invalid assumption");  
} // namespace  

RefFieldImpl::RefFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

RefFieldImpl::RefFieldImpl(const RefFieldImpl&) = default;

FieldImpl::Kind RefFieldImpl::kindImpl() const
{
    return Kind::Ref;
}

FieldImpl::Ptr RefFieldImpl::cloneImpl() const
{
    return Ptr(new RefFieldImpl(*this));
}

const XmlWrap::NamesList& RefFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::fieldStr(),
        common::bitLengthStr(),
    };

    return List;
}

bool RefFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const RefFieldImpl&>(other);
    m_state = castedOther.m_state;
    m_field = castedOther.m_field;
    assert(m_field != nullptr);
    return true;
}

bool RefFieldImpl::parseImpl()
{
    bool mustHaveField = (m_field == nullptr);
    if (!validateSinglePropInstance(common::fieldStr(), mustHaveField)) {
        return false;
    }

    auto propsIter = props().find(common::fieldStr());
    if (propsIter == props().end()) {
        assert(m_field != nullptr);
        assert(!name().empty());
        return true;
    }

    m_field = protocol().findField(propsIter->second);
    if (m_field == nullptr) {
        reportUnexpectedPropertyValue(common::fieldStr(), propsIter->second);
        return false;
    }

    if (name().empty()) {
        setName(m_field->name());
    }

    if (displayName().empty() && (!m_field->displayName().empty())) {
        setDisplayName(m_field->displayName());
    }

    return updateBitLength();
}

std::size_t RefFieldImpl::minLengthImpl() const
{
    assert(m_field != nullptr);
    return m_field->minLength();
}

std::size_t RefFieldImpl::maxLengthImpl() const
{
    assert(m_field != nullptr);
    return m_field->maxLength();
}

std::size_t RefFieldImpl::bitLengthImpl() const
{
    if (isBitfieldMember()) {
        return m_state.m_bitLength;
    }

    return Base::bitLengthImpl();
}

bool RefFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    assert(m_field != nullptr);
    return m_field->isComparableToValue(val);
}

bool RefFieldImpl::isComparableToFieldImpl(const FieldImpl& field) const
{
    assert(m_field != nullptr);
    return m_field->isComparableToField(field);
}

bool RefFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return
        strToValue(
            ref,
            [&val, &isBigUnsigned](const FieldImpl& f, const std::string& str)
            {
                return f.strToNumeric(str, val, isBigUnsigned);
            });
}

bool RefFieldImpl::strToFpImpl(const std::string& ref, double& val) const
{
    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToFp(str, val);
            });
    }

bool RefFieldImpl::strToBoolImpl(const std::string& ref, bool& val) const
{
    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToBool(str, val);
            });
}

bool RefFieldImpl::strToStringImpl(const std::string& ref, std::string& val) const
{
    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToString(str, val);
            });
}

bool RefFieldImpl::strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return
        strToValue(
            ref,
            [&val](const FieldImpl& f, const std::string& str)
            {
                return f.strToData(str, val);
            });
}

bool RefFieldImpl::updateBitLength()
{
    if (!validateSinglePropInstance(common::bitLengthStr())) {
        return false;
    }

    if (!isBitfieldMember()) {
        logWarning() << XmlWrap::logPrefix((getNode())) <<
                        "The property \"" << common::bitLengthStr() << "\" is "
                        "applicable only to the members of \"" << common::bitfieldStr() << "\"";
        return true;
    }

    auto& valStr = common::getStringProp(props(), common::bitLengthStr());
    assert(0 < maxLength());
    auto maxBitLength = maxLength() * BitsInByte;
    if (valStr.empty()) {
        if (m_state.m_bitLength == 0) {
            m_state.m_bitLength = maxBitLength;
            return true;
        }

        assert(m_state.m_bitLength <= maxBitLength);
        return true;
    }    

    bool ok = false;
    m_state.m_bitLength = common::strToUnsigned(valStr, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::bitLengthStr(), valStr);
        return false;
    }

    assert(m_field != nullptr);
    if (!m_field->validateBitLengthValue(getNode(), m_state.m_bitLength)) {
        return false;
    }

    return true;
}

bool RefFieldImpl::strToValue(
    const std::string& ref,
    StrToValueFieldConvertFunc&& forwardFunc) const
{
    if (!protocol().isFieldValueReferenceSupported()) {
        return false;
    }

    assert(m_field != nullptr);
    return forwardFunc(*m_field, ref);
}


} // namespace commsdsl
