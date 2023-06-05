//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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
#include "commsdsl/parse/OptionalField.h"
#include "FieldImpl.h"
#include "OptCondImpl.h"

namespace commsdsl
{

namespace parse
{

class OptionalFieldImpl final : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Mode = OptionalField::Mode;

    OptionalFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    OptionalFieldImpl(const OptionalFieldImpl& other);

    Mode defaultMode() const
    {
        return m_state.m_mode;
    }

    bool externalModeCtrl() const
    {
        return m_state.m_externalModeCtrl;
    }

    bool missingOnReadFail() const
    {
        return m_state.m_missingOnReadFail;
    }  

    bool missingOnInvalid() const
    {
        return m_state.m_missingOnInvalid;
    }      

    bool hasField() const
    {
        return (m_state.m_extField != nullptr) || static_cast<bool>(m_field);
    }

    Field field() const
    {
        if (m_state.m_extField != nullptr) {
            return Field(m_state.m_extField);
        }

        return Field(m_field.get());
    }

    OptCond wrappedCondition() const
    {
        return OptCond(m_cond.get());
    }

    OptCondImplPtr& cond()
    {
        return m_cond;
    }

    const OptCondImplPtr& cond() const
    {
        return m_cond;
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
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override;
    virtual bool isValidRefTypeImpl(FieldRefType type) const override;

private:
    using StrToValueFieldConvertFunc = std::function<bool (const FieldImpl& f, const std::string& ref)>;

    bool updateMode();
    bool updateExternalModeCtrl();
    bool updateMissingOnReadFail();
    bool updateMissingOnInvalid();
    bool updateField();
    bool updateSingleCondition();
    bool updateMultiCondition();
    bool checkFieldFromRef();
    bool checkFieldAsChild();
    const FieldImpl* getField() const;

    bool strToValue(
        const std::string& ref,
        StrToValueFieldConvertFunc&& forwardFunc) const;


    struct State
    {
        Mode m_mode = Mode::Tentative;
        const FieldImpl* m_extField = nullptr;
        bool m_externalModeCtrl = false;
        bool m_missingOnReadFail = false;
        bool m_missingOnInvalid = false;
    };

    State m_state;
    FieldImplPtr m_field;
    OptCondImplPtr m_cond;
};

} // namespace parse

} // namespace commsdsl
