//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/Endian.h"
#include "commsdsl/DataField.h"
#include "FieldImpl.h"

namespace commsdsl
{

class DataFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:

    using ValueType = DataField::ValueType;

    DataFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    DataFieldImpl(const DataFieldImpl& other);

    const ValueType& defaultValue() const
    {
        return m_state.m_defaultValue;
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


protected:
    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl& other) override final;
    virtual bool parseImpl() override final;
    virtual bool verifySiblingsImpl(const FieldsList& fields) const override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override final;

private:
    bool updateDefaultValue();
    bool updateLength();
    bool updatePrefix();
    bool checkPrefixFromRef();
    bool checkPrefixAsChild();
    const FieldImpl* getPrefixField() const;

    bool strToValue(const std::string& str, ValueType& val) const;

    struct State
    {
        ValueType m_defaultValue;
        std::size_t m_length = 0U;
        const FieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
    };

    State m_state;
    FieldImplPtr m_prefixField;
};

} // namespace commsdsl
