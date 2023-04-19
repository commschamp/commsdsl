//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/DataField.h"

#include "CommsField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsDataField final : public commsdsl::gen::DataField, public CommsField
{
    using Base = commsdsl::gen::DataField;
    using CommsBase = CommsField;
public:
    CommsDataField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

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
    virtual std::string commsDefConstructCodeImpl() const override;
    virtual std::string commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual std::string commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual bool commsIsLimitedCustomizableImpl() const override;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const override;
    virtual StringsList commsExtraDataViewDefaultOptionsImpl() const override;
    virtual StringsList commsExtraBareMetalDefaultOptionsImpl() const override;
    virtual std::size_t commsMaxLengthImpl() const override; 

private:
    std::string commsDefFieldOptsInternal() const;

    void commsAddFixedLengthOptInternal(StringsList& opts) const;
    void commsAddLengthPrefixOptInternal(StringsList& opts) const;
    void commsAddLengthForcingOptInternal(StringsList& opts) const;

    CommsField* m_commsExternalPrefixField = nullptr;
    CommsField* m_commsMemberPrefixField = nullptr;
};

} // namespace commsdsl2comms
