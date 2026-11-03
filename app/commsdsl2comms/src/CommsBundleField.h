//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

    [[nodiscard]] const CommsFieldsList& commsMembers() const
    {
        return m_commsMembers;
    }

protected:
    // GenBase overrides
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

    // CommsBase overrides
    [[nodiscard]] virtual CommsIncludesList commsCommonIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsCommonCodeBodyImpl() const override;
    [[nodiscard]] virtual std::string commsCommonMembersCodeImpl() const override;
    [[nodiscard]] virtual CommsIncludesList commsDefIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsDefMembersCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefBaseClassImpl() const override;
    [[nodiscard]] virtual std::string commsDefPublicCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefPrivateCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefReadFuncBodyImpl() const override;
    [[nodiscard]] virtual std::string commsDefRefreshFuncBodyImpl() const override;
    [[nodiscard]] virtual std::string commsDefValidFuncBodyImpl() const override;
    [[nodiscard]] virtual bool commsIsVersionDependentImpl() const override;
    [[nodiscard]] virtual std::string commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const override;
    [[nodiscard]] virtual std::size_t commsMinLengthImpl() const override;
    [[nodiscard]] virtual std::size_t commsMaxLengthImpl() const override;
    [[nodiscard]] virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    [[nodiscard]] virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual void commsCompOptChecksImpl(const std::string& accStr, GenStringsList& checks, const std::string& prefix) const override;
    [[nodiscard]] virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const override;
    [[nodiscard]] virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    [[nodiscard]] virtual bool commsHasCustomLengthDeepImpl() const override;
    [[nodiscard]] virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    [[nodiscard]] bool commsPrepareInternal();
    [[nodiscard]] std::string commsDefFieldOptsInternal() const;
    [[nodiscard]] std::string commsDefAccessCodeInternal() const;
    [[nodiscard]] std::string commsDefAliasesCodeInternal() const;

    void commsAddCustomReadRefreshOptInternal(GenStringsList& opts) const;
    void commsAddRemLengthMemberOptInternal(GenStringsList& opts) const;

    [[nodiscard]] std::pair<const CommsField*, std::string> commsParseMemRefInternal(const std::string accStr) const;

    CommsFieldsList m_commsMembers;
    GenStringsList m_bundledReadPrepareCodes;
    GenStringsList m_bundledRefreshCodes;
};

} // namespace commsdsl2comms
