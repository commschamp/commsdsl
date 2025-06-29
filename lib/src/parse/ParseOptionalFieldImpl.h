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

    Mode parseDefaultMode() const
    {
        return m_state.m_mode;
    }

    bool parseMissingOnReadFail() const
    {
        return m_state.m_missingOnReadFail;
    }  

    bool parseMissingOnInvalid() const
    {
        return m_state.m_missingOnInvalid;
    }      

    bool parseHasField() const
    {
        return (m_state.m_extField != nullptr) || static_cast<bool>(m_field);
    }

    ParseField parseField() const
    {
        if (m_state.m_extField != nullptr) {
            return ParseField(m_state.m_extField);
        }

        return ParseField(m_field.get());
    }

    ParseOptCond parseWrappedCondition() const
    {
        return ParseOptCond(m_cond.get());
    }

    ParseOptCondImplPtr& parseCond()
    {
        return m_cond;
    }

    const ParseOptCondImplPtr& parseCond() const
    {
        return m_cond;
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
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const override;
    virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const override;
    virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const override;
    virtual bool parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const override;
    virtual FieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const override;
    virtual bool parseIsValidRefTypeImpl(FieldRefType type) const override;

private:
    using StrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;

    bool parseUpdateMode();
    bool parseUpdateExternalModeCtrl();
    bool parseUpdateMissingOnReadFail();
    bool parseUpdateMissingOnInvalid();
    bool parseUpdateField();
    bool parseUpdateSingleCondition();
    bool parseUpdateMultiCondition();
    bool parseCheckFieldFromRef();
    bool parseCheckFieldAsChild();
    const ParseFieldImpl* parseGetField() const;

    bool parseStrToValue(
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
