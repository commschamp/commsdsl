//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/Endian.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class StringFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:

    StringFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    StringFieldImpl(const StringFieldImpl& other);

    const std::string& defaultValue() const
    {
        return m_state.m_defaultValue;
    }

    const std::string& encodingStr() const
    {
        return m_state.m_encoding;
    }

    std::size_t length() const
    {
        return m_state.m_length;
    }

    bool hasPrefixField() const
    {
        return (m_state.m_extPrefixField != nullptr) || static_cast<bool>(m_prefixField);
    }

    Field prefixField() const
    {
        if (m_state.m_extPrefixField != nullptr) {
            return Field(m_state.m_extPrefixField);
        }

        return Field(m_prefixField.get());
    }

    const std::string& detachedPrefixFieldName() const
    {
        return m_state.m_detachedPrefixField;
    }

    bool hasZeroTermSuffix() const
    {
        return m_state.m_haxZeroSuffix;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual bool verifySiblingsImpl(const FieldsList& fields) const override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const override;

private:
    bool updateDefaultValue();
    bool updateEncoding();
    bool updateLength();
    bool updatePrefix();
    bool updateZeroTerm();
    bool checkPrefixFromRef();
    bool checkPrefixAsChild();
    const FieldImpl* getPrefixField() const;
    bool strToValue(const std::string& str, std::string& val) const;

    struct State
    {
        std::string m_defaultValue;
        std::string m_encoding;
        std::size_t m_length = 0U;
        const FieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
        bool m_haxZeroSuffix = false;
    };

    State m_state;
    FieldImplPtr m_prefixField;
};

} // namespace parse

} // namespace commsdsl
