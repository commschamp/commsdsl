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

#include "commsdsl/gen/ListField.h"

#include "CommsField.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsListField final : public commsdsl::gen::ListField, public CommsField
{
    using Base = commsdsl::gen::ListField;
    using CommsBase = CommsField;
public:
    CommsListField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

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
    virtual bool commsIsLimitedCustomizableImpl() const override;
    virtual bool commsDoesRequireGeneratedReadRefreshImpl() const override;
    virtual bool commsIsVersionDependentImpl() const override;
    virtual std::string commsCompareToValueCodeImpl(const std::string& op, const std::string& value, const std::string& nameOverride, bool forcedVersionOptional) const override;
    virtual std::string commsCompareToFieldCodeImpl(const std::string& op, const CommsField& field, const std::string& nameOverride, bool forcedVersionOptional) const override;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const override;
    virtual StringsList commsExtraBareMetalDefaultOptionsImpl() const override;

private:
    std::string commsDefFieldOptsInternal() const;
    std::string commsDefElementInternal() const;

    void commsAddFixedLengthOptInternal(StringsList& opts) const;
    void commsAddCountPrefixOptInternal(StringsList& opts) const;
    void commsAddLengthPrefixOptInternal(StringsList& opts) const;
    void commsAddElemLengthPrefixOptInternal(StringsList& opts) const;
    void commsAddLengthForcingOptInternal(StringsList& opts) const;

    CommsField* m_commsExternalElementField = nullptr;
    CommsField* m_commsMemberElementField = nullptr;
    CommsField* m_commsExternalCountPrefixField = nullptr;
    CommsField* m_commsMemberCountPrefixField = nullptr;
    CommsField* m_commsExternalLengthPrefixField = nullptr;
    CommsField* m_commsMemberLengthPrefixField = nullptr;
    CommsField* m_commsExternalElemLengthPrefixField = nullptr;
    CommsField* m_commsMemberElemLengthPrefixField = nullptr;
};

} // namespace commsdsl2new
