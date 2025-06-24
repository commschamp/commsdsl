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

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseStringField.h"
#include "ParseFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseStringFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using ValidValueInfo = ParseStringField::ValidValueInfo;
    using ValidValuesList = ParseStringField::ValidValuesList;

    ParseStringFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseStringFieldImpl(const ParseStringFieldImpl& other);

    const std::string& parseDefaultValue() const
    {
        return m_state.m_defaultValue;
    }

    const std::string& parseEncodingStr() const
    {
        return m_state.m_encoding;
    }

    std::size_t parseLength() const
    {
        return m_state.m_length;
    }

    bool parseHasPrefixField() const
    {
        return (m_state.m_extPrefixField != nullptr) || static_cast<bool>(m_prefixField);
    }

    ParseField parsePrefixField() const
    {
        if (m_state.m_extPrefixField != nullptr) {
            return ParseField(m_state.m_extPrefixField);
        }

        return ParseField(m_prefixField.get());
    }

    const std::string& parseDetachedPrefixFieldName() const
    {
        return m_state.m_detachedPrefixField;
    }

    bool parseHasZeroTermSuffix() const
    {
        return m_state.m_haxZeroSuffix;
    }

    const ValidValuesList& parseValidValues() const
    {
        return m_state.m_validValues;
    }

protected:
    virtual Kind parseKindImpl() const override;
    virtual Ptr parseCloneImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraPossiblePropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual bool parseVerifySiblingsImpl(const FieldsList& fields) const override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseMaxLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool parseIsValidRefTypeImpl(FieldRefType type) const override;

private:
    bool parseUpdateDefaultValue();
    bool parseUpdateDefaultValidValue();
    bool parseUpdateEncoding();
    bool parseUpdateLength();
    bool parseUpdatePrefix();
    bool parseUpdateZeroTerm();
    bool parseUpdateValidValues();
    bool parseCheckPrefixFromRef();
    bool parseCheckPrefixAsChild();
    bool parseCheckValidValueAsAttr(const PropsMap& xmlAttrs);
    bool parseCheckValidValueAsChild(::xmlNodePtr child);    
    const ParseFieldImpl* parseGetPrefixField() const;
    bool parseStrToValue(const std::string& str, std::string& val) const;

    struct State
    {
        std::string m_defaultValue;
        std::string m_encoding;
        std::size_t m_length = 0U;
        const ParseFieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
        ValidValuesList m_validValues;
        bool m_haxZeroSuffix = false;
    };

    State m_state;
    ParseFieldImplPtr m_prefixField;
};

} // namespace parse

} // namespace commsdsl
