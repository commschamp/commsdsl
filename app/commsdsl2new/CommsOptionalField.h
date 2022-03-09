//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2new
{

class CommsGenerator;
class CommsOptionalField final : public commsdsl::gen::OptionalField, public CommsField
{
    using Base = commsdsl::gen::OptionalField;
    using CommsBase = CommsField;
public:
    CommsOptionalField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

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

private:
    std::string commsDefFieldRefInternal() const;
    std::string commsDefFieldOptsInternal() const;

    void commsAddModeOptInternal(StringsList& opts) const;
    static std::string commsDslCondToStringInternal(const CommsFieldsList& siblings, const commsdsl::parse::OptCond& cond, bool bracketsWrap = false);

    CommsField* m_commsExternalField = nullptr;
    CommsField* m_commsMemberField = nullptr;
};

} // namespace commsdsl2new
