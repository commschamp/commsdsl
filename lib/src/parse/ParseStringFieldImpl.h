//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseStringField.h"
#include "ParseFieldImpl.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseStringFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using ParseValidValueInfo = ParseStringField::ParseValidValueInfo;
    using ParseValidValuesList = ParseStringField::ParseValidValuesList;

    ParseStringFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseStringFieldImpl(const ParseStringFieldImpl& other);

    [[nodiscard]] const std::string& parseDefaultValue() const
    {
        return m_state.m_defaultValue;
    }

    [[nodiscard]] const std::string& parseEncodingStr() const
    {
        return m_state.m_encoding;
    }

    [[nodiscard]] std::size_t parseLength() const
    {
        return m_state.m_length;
    }

    [[nodiscard]] bool parseHasPrefixField() const
    {
        return (m_state.m_extPrefixField != nullptr) || static_cast<bool>(m_prefixField);
    }

    [[nodiscard]] ParseField parsePrefixField() const
    {
        if (m_state.m_extPrefixField != nullptr) {
            return ParseField(m_state.m_extPrefixField);
        }

        return ParseField(m_prefixField.get());
    }

    [[nodiscard]] const std::string& parseDetachedPrefixFieldName() const
    {
        return m_state.m_detachedPrefixField;
    }

    [[nodiscard]] bool parseHasZeroTermSuffix() const
    {
        return m_state.m_haxZeroSuffix;
    }

    [[nodiscard]] const ParseValidValuesList& parseValidValues() const
    {
        return m_state.m_validValues;
    }

protected:
    virtual ParseKind parseKindImpl() const override;
    virtual ParsePtr parseCloneImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNamesImpl() const override;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraChildrenNamesImpl() const override;
    virtual bool parseReuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual bool parseVerifySiblingsImpl(const ParseFieldsList& fields) const override;
    virtual std::size_t parseMinLengthImpl() const override;
    virtual std::size_t parseMaxLengthImpl() const override;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const override;
    virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool parseIsValidRefTypeImpl(ParseFieldRefType type) const override;

private:
    [[nodiscard]] bool parseUpdateDefaultValue();
    [[nodiscard]] bool parseUpdateDefaultValidValue();
    [[nodiscard]] bool parseUpdateEncoding();
    [[nodiscard]] bool parseUpdateLength();
    [[nodiscard]] bool parseUpdatePrefix();
    [[nodiscard]] bool parseUpdateZeroTerm();
    [[nodiscard]] bool parseUpdateValidValues();
    [[nodiscard]] bool parseCheckPrefixFromRef();
    [[nodiscard]] bool parseCheckPrefixAsChild();
    [[nodiscard]] bool parseCheckValidValueAsAttr(const ParsePropsMap& xmlAttrs);
    [[nodiscard]] bool parseCheckValidValueAsChild(::xmlNodePtr child);
    [[nodiscard]] const ParseFieldImpl* parseGetPrefixField() const;
    [[nodiscard]] bool parseStrToValue(const std::string& str, std::string& val) const;

    struct ParseState
    {
        std::string m_defaultValue;
        std::string m_encoding;
        std::size_t m_length = 0U;
        const ParseFieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
        ParseValidValuesList m_validValues;
        bool m_haxZeroSuffix = false;
    };

    ParseState m_state;
    ParseFieldImplPtr m_prefixField;
};

} // namespace parse

} // namespace commsdsl
