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

#include "CommsField.h"

#include "commsdsl/gen/GenStringField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsStringField final : public commsdsl::gen::GenStringField, public CommsField
{
    using GenBase = commsdsl::gen::GenStringField;
    using CommsBase = CommsField;
public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    CommsStringField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    // GenBase overrides
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

    // CommsBase overrides
    virtual CommsIncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonMembersCodeImpl() const override;
    virtual CommsIncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefMembersCodeImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsDefConstructCodeImpl() const override;
    virtual std::string commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual std::string commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual std::string commsDefValidFuncBodyImpl() const override;
    virtual bool commsIsLimitedCustomizableImpl() const override;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const override;
    virtual GenStringsList commsExtraDataViewDefaultOptionsImpl() const override;
    virtual GenStringsList commsExtraBareMetalDefaultOptionsImpl() const override;
    virtual std::size_t commsMaxLengthImpl() const override;
    virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;

private:
    std::string commsDefFieldOptsInternal() const;

    void commsAddFixedLengthOptInternal(GenStringsList& opts) const;
    void commsAddLengthPrefixOptInternal(GenStringsList& opts) const;
    void commsAddTermSuffixOptInternal(GenStringsList& opts) const;
    void commsAddLengthForcingOptInternal(GenStringsList& opts) const;

    CommsField* m_commsExternalPrefixField = nullptr;
    CommsField* m_commsMemberPrefixField = nullptr;
};

} // namespace commsdsl2comms
