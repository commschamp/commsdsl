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
#include "commsdsl/parse/ParseOptionalField.h"
#include "ParseFieldImpl.h"
#include "ParseOptCondImpl.h"

namespace commsdsl
{

namespace parse
{

class ParseOptionalFieldImpl final : public ParseFieldImpl
{
    using Base = ParseFieldImpl;
public:
    using Mode = ParseOptionalField::Mode;

    ParseOptionalFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseOptionalFieldImpl(const ParseOptionalFieldImpl& other);

    Mode defaultMode() const
    {
        return m_state.m_mode;
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

    ParseField field() const
    {
        if (m_state.m_extField != nullptr) {
            return ParseField(m_state.m_extField);
        }

        return ParseField(m_field.get());
    }

    ParseOptCond wrappedCondition() const
    {
        return ParseOptCond(m_cond.get());
    }

    ParseOptCondImplPtr& cond()
    {
        return m_cond;
    }

    const ParseOptCondImplPtr& cond() const
    {
        return m_cond;
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
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override;
    virtual bool isValidRefTypeImpl(FieldRefType type) const override;

private:
    using StrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;

    bool updateMode();
    bool updateExternalModeCtrl();
    bool updateMissingOnReadFail();
    bool updateMissingOnInvalid();
    bool updateField();
    bool updateSingleCondition();
    bool updateMultiCondition();
    bool checkFieldFromRef();
    bool checkFieldAsChild();
    const ParseFieldImpl* getField() const;

    bool strToValue(
        const std::string& ref,
        StrToValueFieldConvertFunc&& forwardFunc) const;


    struct State
    {
        Mode m_mode = Mode::Tentative;
        const ParseFieldImpl* m_extField = nullptr;
        bool m_missingOnReadFail = false;
        bool m_missingOnInvalid = false;
    };

    State m_state;
    ParseFieldImplPtr m_field;
    ParseOptCondImplPtr m_cond;
};

} // namespace parse

} // namespace commsdsl
