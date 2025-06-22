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
#include "ParseFieldImpl.h"

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

    std::size_t count() const
    {
        return m_state.m_count;
    }

    bool hasElementField() const
    {
        return (m_state.m_extElementField != nullptr) ||
               static_cast<bool>(m_elementField);
    }

    ParseField elementField() const
    {
        if (m_state.m_extElementField != nullptr) {
            return ParseField(m_state.m_extElementField);
        }

        return ParseField(m_elementField.get());
    }


    bool hasCountPrefixField() const
    {
        return (m_state.m_extCountPrefixField != nullptr) ||
               static_cast<bool>(m_countPrefixField);
    }

    ParseField countPrefixField() const
    {
        if (m_state.m_extCountPrefixField != nullptr) {
            return ParseField(m_state.m_extCountPrefixField);
        }

        return ParseField(m_countPrefixField.get());
    }

    const std::string& detachedCountPrefixFieldName() const
    {
        return m_state.m_detachedCountPrefixField;
    }

    bool hasLengthPrefixField() const
    {
        return (m_state.m_extLengthPrefixField != nullptr) ||
               static_cast<bool>(m_lengthPrefixField);
    }

    ParseField lengthPrefixField() const
    {
        if (m_state.m_extLengthPrefixField != nullptr) {
            return ParseField(m_state.m_extLengthPrefixField);
        }

        return ParseField(m_lengthPrefixField.get());
    }

    const std::string& detachedLengthPrefixFieldName() const
    {
        return m_state.m_detachedLengthPrefixField;
    }

    bool hasElemLengthPrefixField() const
    {
        return (m_state.m_extElemLengthPrefixField != nullptr) ||
               static_cast<bool>(m_elemLengthPrefixField);
    }

    ParseField elemLengthPrefixField() const
    {
        if (m_state.m_extElemLengthPrefixField != nullptr) {
            return ParseField(m_state.m_extElemLengthPrefixField);
        }

        return ParseField(m_elemLengthPrefixField.get());
    }

    const std::string& detachedElemLengthPrefixFieldName() const
    {
        return m_state.m_detachedElemLengthPrefixField;
    }

    bool hasTermSuffixField() const
    {
        return (m_state.m_extTermSuffixField != nullptr) ||
               static_cast<bool>(m_termSuffixField);
    }

    ParseField termSuffixField() const
    {
        if (m_state.m_extTermSuffixField != nullptr) {
            return ParseField(m_state.m_extTermSuffixField);
        }

        return ParseField(m_termSuffixField.get());
    }

    const std::string& detachedTermSuffixFieldName() const
    {
        return m_state.m_detachedTermSuffixField;
    }    

    bool elemFixedLength() const
    {
        return m_state.m_elemFixedLength;
    }


protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const ParseXmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& extraPossiblePropsNamesImpl() const override;
    virtual const ParseXmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const ParseFieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual bool verifySiblingsImpl(const FieldsList& fields) const override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual bool isValidRefTypeImpl(FieldRefType type) const override;

private:
    void cloneFields(const ParseListFieldImpl& other);
    bool updateElement();
    bool updateCount();
    bool updateCountPrefix();
    bool updateLengthPrefix();
    bool updateElemLengthPrefix();
    bool updateElemFixedLength();
    bool updateTermSuffix();
    bool checkElementFromRef();
    bool checkElementAsChild();
    bool checkPrefixFromRef(
        const std::string& type,
        const ParseFieldImpl*& extField,
        ParseFieldImplPtr& locField,
        std::string& detachedPrefix);
    bool checkPrefixAsChild(
        const std::string& type,
        const ParseFieldImpl*& extField,
        ParseFieldImplPtr& locField,
        std::string& detachedPrefix);
    const ParseFieldImpl* getCountPrefixField() const;
    const ParseFieldImpl* getLengthPrefixField() const;
    bool verifySiblingsForPrefix(const FieldsList& fields, const std::string& detachedName) const;

    struct State
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

    State m_state;
    ParseFieldImplPtr m_elementField;
    ParseFieldImplPtr m_countPrefixField;
    ParseFieldImplPtr m_lengthPrefixField;
    ParseFieldImplPtr m_elemLengthPrefixField;
    ParseFieldImplPtr m_termSuffixField;
};

} // namespace parse

} // namespace commsdsl
