//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/OptionalField.h"

#include "CommsField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsOptionalField final : public commsdsl::gen::OptionalField, public CommsField
{
    using Base = commsdsl::gen::OptionalField;
    using CommsBase = CommsField;
public:
    CommsOptionalField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

    static std::string commsDslCondToString(
        const CommsGenerator& generator, 
        const CommsFieldsList& siblings, 
        const commsdsl::parse::OptCond& cond, 
        bool bracketsWrap = false);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonMembersCodeImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefMembersCodeImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual std::string commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual bool commsIsVersionDependentImpl() const override;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const override;
    virtual std::size_t commsMaxLengthImpl() const override;
    virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual void commsCompOptChecksImpl(const std::string& accStr, StringsList& checks, const std::string& prefix) const override;
    virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;


private:
    bool commsCheckCondSupportedInternal() const;
    std::string commsDefFieldRefInternal() const;
    std::string commsDefFieldOptsInternal() const;

    void commsAddModeOptInternal(StringsList& opts) const;
    void commsAddMissingOnReadFailOptInternal(StringsList& opts) const;
    void commsAddMissingOnInvalidOptInternal(StringsList& opts) const;

    std::string commsDslCondToStringInternal(const CommsFieldsList& siblings, const commsdsl::parse::OptCond& cond, bool bracketsWrap = false) const;
    std::string commsMemberAccessStringInternal(const std::string& accStr) const;
    static std::string commsDslCondToStringFieldValueCompInternal(
        const CommsField* field, 
        const std::string& accStr,
        const std::string& op, 
        const std::string& value);

    static std::string commsDslCondToStringFieldFieldCompInternal(
        const CommsField* leftField, 
        const std::string& leftAccStr,
        const std::string& op, 
        const CommsField* rightField, 
        const std::string& rightAccStr);  

    static std::string commsDslCondToStringFieldSizeCompInternal(
        const CommsField* field, 
        const std::string& accStr,
        const std::string& op, 
        const std::string& value);   

    static std::string commsDslCondToStringFieldExistsCompInternal(
        const CommsField* field, 
        const std::string& accStr,
        const std::string& op);
          
    CommsField* m_commsExternalField = nullptr;
    CommsField* m_commsMemberField = nullptr;
};

} // namespace commsdsl2comms
