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

#include "commsdsl/parse/ParseDataField.h"
#include "commsdsl/parse/ParseEndian.h"
#include "ParseFieldImpl.h"

#include <cstdint>
#include <vector>
#include <utility>

namespace commsdsl
{

namespace parse
{

class ParseDataFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using ValueType = ParseDataField::ValueType;
    using ValidValueInfo = ParseDataField::ValidValueInfo;
    using ValidValuesList = ParseDataField::ValidValuesList;    

    ParseDataFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseDataFieldImpl(const ParseDataFieldImpl& other);

    const ValueType& parseDefaultValue() const
    {
        return m_state.m_defaultValue;
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
    virtual bool parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual bool parseIsValidRefTypeImpl(FieldRefType type) const override;

private:
    bool parseUpdateDefaultValue();
    bool parseUpdateDefaultValidValue();
    bool parseUpdateLength();
    bool parseUpdatePrefix();
    bool parseUpdateValidValues();    
    bool parseCheckPrefixFromRef();
    bool parseCheckPrefixAsChild();
    bool parseCheckValidValueAsAttr(const PropsMap& xmlAttrs);
    bool parseCheckValidValueAsChild(::xmlNodePtr child);        
    const ParseFieldImpl* parseGetPrefixField() const;

    bool parseStrToValue(const std::string& str, ValueType& val) const;

    struct State
    {
        ValueType m_defaultValue;
        std::size_t m_length = 0U;
        const ParseFieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
        ValidValuesList m_validValues;
    };

    State m_state;
    ParseFieldImplPtr m_prefixField;
};

} // namespace parse

} // namespace commsdsl
