//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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
#include "commsdsl/parse/DataField.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class DataFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    using ValueType = DataField::ValueType;
    using ValidValueInfo = DataField::ValidValueInfo;
    using ValidValuesList = DataField::ValidValuesList;    

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

    const ValidValuesList& validValues() const
    {
        return m_state.m_validValues;
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
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual bool isValidRefTypeImpl(FieldRefType type) const override;

private:
    bool updateDefaultValue();
    bool updateLength();
    bool updatePrefix();
    bool updateValidValues();    
    bool checkPrefixFromRef();
    bool checkPrefixAsChild();
    bool checkValidValueAsAttr(const PropsMap& xmlAttrs);
    bool checkValidValueAsChild(::xmlNodePtr child);        
    const FieldImpl* getPrefixField() const;

    bool strToValue(const std::string& str, ValueType& val) const;

    struct State
    {
        ValueType m_defaultValue;
        std::size_t m_length = 0U;
        const FieldImpl* m_extPrefixField = nullptr;
        std::string m_detachedPrefixField;
        ValidValuesList m_validValues;
    };

    State m_state;
    FieldImplPtr m_prefixField;
};

} // namespace parse

} // namespace commsdsl
