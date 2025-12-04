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

#include "commsdsl/parse/ParseEndian.h"
#include "ParseFieldImpl.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseListFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:

    ParseListFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseListFieldImpl(const ParseListFieldImpl& other);

    std::size_t parseCount() const
    {
        return m_state.m_count;
    }

    bool parseHasElementField() const
    {
        return (m_state.m_extElementField != nullptr) ||
               static_cast<bool>(m_elementField);
    }

    ParseField parseElementField() const
    {
        if (m_state.m_extElementField != nullptr) {
            return ParseField(m_state.m_extElementField);
        }

        return ParseField(m_elementField.get());
    }

    bool parseHasCountPrefixField() const
    {
        return (m_state.m_extCountPrefixField != nullptr) ||
               static_cast<bool>(m_countPrefixField);
    }

    ParseField parseCountPrefixField() const
    {
        if (m_state.m_extCountPrefixField != nullptr) {
            return ParseField(m_state.m_extCountPrefixField);
        }

        return ParseField(m_countPrefixField.get());
    }

    const std::string& parseDetachedCountPrefixFieldName() const
    {
        return m_state.m_detachedCountPrefixField;
    }

    bool parseHasLengthPrefixField() const
    {
        return (m_state.m_extLengthPrefixField != nullptr) ||
               static_cast<bool>(m_lengthPrefixField);
    }

    ParseField parseLengthPrefixField() const
    {
        if (m_state.m_extLengthPrefixField != nullptr) {
            return ParseField(m_state.m_extLengthPrefixField);
        }

        return ParseField(m_lengthPrefixField.get());
    }

    const std::string& parseDetachedLengthPrefixFieldName() const
    {
        return m_state.m_detachedLengthPrefixField;
    }

    bool parseHasElemLengthPrefixField() const
    {
        return (m_state.m_extElemLengthPrefixField != nullptr) ||
               static_cast<bool>(m_elemLengthPrefixField);
    }

    ParseField parseElemLengthPrefixField() const
    {
        if (m_state.m_extElemLengthPrefixField != nullptr) {
            return ParseField(m_state.m_extElemLengthPrefixField);
        }

        return ParseField(m_elemLengthPrefixField.get());
    }

    const std::string& parseDetachedElemLengthPrefixFieldName() const
    {
        return m_state.m_detachedElemLengthPrefixField;
    }

    bool parseHasTermSuffixField() const
    {
        return (m_state.m_extTermSuffixField != nullptr) ||
               static_cast<bool>(m_termSuffixField);
    }

    ParseField parseTermSuffixField() const
    {
        if (m_state.m_extTermSuffixField != nullptr) {
            return ParseField(m_state.m_extTermSuffixField);
        }

        return ParseField(m_termSuffixField.get());
    }

    const std::string& parseDetachedTermSuffixFieldName() const
    {
        return m_state.m_detachedTermSuffixField;
    }

    bool parseElemFixedLength() const
    {
        return m_state.m_elemFixedLength;
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
    virtual bool parseIsValidRefTypeImpl(ParseFieldRefType type) const override;

private:
    void parseCloneFields(const ParseListFieldImpl& other);
    bool parseUpdateElement();
    bool parseUpdateCount();
    bool parseUpdateCountPrefix();
    bool parseUpdateLengthPrefix();
    bool parseUpdateElemLengthPrefix();
    bool parseUpdateElemFixedLength();
    bool parseUpdateTermSuffix();
    bool parseCheckElementFromRef();
    bool parseCheckElementAsChild();
    bool parseCheckPrefixFromRef(
        const std::string& type,
        const ParseFieldImpl*& extField,
        ParseFieldImplPtr& locField,
        std::string& detachedPrefix);
    bool parseCheckPrefixAsChild(
        const std::string& type,
        const ParseFieldImpl*& extField,
        ParseFieldImplPtr& locField,
        std::string& detachedPrefix);
    const ParseFieldImpl* parseGetCountPrefixField() const;
    const ParseFieldImpl* parseGetLengthPrefixField() const;
    bool parseVerifySiblingsForPrefix(const ParseFieldsList& fields, const std::string& detachedName) const;

    struct ParseState
    {
        std::size_t m_count = 0U;
        const ParseFieldImpl* m_extElementField = nullptr;
        const ParseFieldImpl* m_extCountPrefixField = nullptr;
        const ParseFieldImpl* m_extLengthPrefixField = nullptr;
        const ParseFieldImpl* m_extElemLengthPrefixField = nullptr;
        const ParseFieldImpl* m_extTermSuffixField = nullptr;
        std::string m_detachedCountPrefixField;
        std::string m_detachedLengthPrefixField;
        std::string m_detachedElemLengthPrefixField;
        std::string m_detachedTermSuffixField;
        bool m_elemFixedLength = false;
    };

    ParseState m_state;
    ParseFieldImplPtr m_elementField;
    ParseFieldImplPtr m_countPrefixField;
    ParseFieldImplPtr m_lengthPrefixField;
    ParseFieldImplPtr m_elemLengthPrefixField;
    ParseFieldImplPtr m_termSuffixField;
};

} // namespace parse

} // namespace commsdsl
