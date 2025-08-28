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

#include "commsdsl/gen/GenBundleField.h"
#include "commsdsl/gen/util.h"

#include <utility>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsBundleField final : public commsdsl::gen::GenBundleField, public CommsField
{
    using GenBase = commsdsl::gen::GenBundleField;
    using CommsBase = CommsField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    CommsBundleField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

    const CommsFieldsList& commsMembers() const
    {
        return m_commsMembers;
    }

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
    virtual std::string commsDefPublicCodeImpl() const override;
    virtual std::string commsDefPrivateCodeImpl() const override;
    virtual std::string commsDefReadFuncBodyImpl() const override;
    virtual std::string commsDefRefreshFuncBodyImpl() const override;    
    virtual std::string commsDefValidFuncBodyImpl() const override;
    virtual bool commsIsVersionDependentImpl() const override;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const override;
    virtual std::size_t commsMinLengthImpl() const override;
    virtual std::size_t commsMaxLengthImpl() const override;       
    virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual void commsCompOptChecksImpl(const std::string& accStr, GenStringsList& checks, const std::string& prefix) const override;
    virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    virtual bool commsHasCustomLengthDeepImpl() const override;
    virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    bool commsPrepareInternal();
    std::string commsDefFieldOptsInternal() const;
    std::string commsDefAccessCodeInternal() const;
    std::string commsDefAliasesCodeInternal() const;

    void commsAddCustomReadRefreshOptInternal(GenStringsList& opts) const;
    void commsAddRemLengthMemberOptInternal(GenStringsList& opts) const;

    std::pair<const CommsField*, std::string> commsParseMemRefInternal(const std::string accStr) const;

    CommsFieldsList m_commsMembers;
    GenStringsList m_bundledReadPrepareCodes;
    GenStringsList m_bundledRefreshCodes;
};

} // namespace commsdsl2comms
