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

#include "commsdsl/gen/GenVariantField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsVariantField final : public commsdsl::gen::GenVariantField, public CommsField
{
    using GenBase = commsdsl::gen::GenVariantField;
    using CommsBase = CommsField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    CommsVariantField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

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
    [[nodiscard]] virtual std::string commsDefConstructCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefDestructCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefPublicCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefPrivateCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefReadFuncBodyImpl() const override;
    [[nodiscard]] virtual GenStringsList commsDefReadMsvcSuppressWarningsImpl() const override;
    [[nodiscard]] virtual std::string commsDefWriteFuncBodyImpl() const override;
    [[nodiscard]] virtual std::string commsDefRefreshFuncBodyImpl() const override;
    [[nodiscard]] virtual std::string commsDefLengthFuncBodyImpl() const override;
    [[nodiscard]] virtual std::string commsDefValidFuncBodyImpl() const override;
    [[nodiscard]] virtual bool commsIsVersionDependentImpl() const override;
    [[nodiscard]] virtual std::string commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const override;
    [[nodiscard]] virtual std::size_t commsMaxLengthImpl() const override;
    [[nodiscard]] virtual bool commsHasCustomLengthDeepImpl() const override;
    [[nodiscard]] virtual bool commsMustDefineDefaultConstructorImpl() const override;

private:
    [[nodiscard]] bool commsPrepareInternal();
    [[nodiscard]] std::string commsDefFieldOptsInternal() const;
    [[nodiscard]] std::string commsDefCopyCodeInternal() const;
    [[nodiscard]] std::string commsDefAccessCodeInternal() const;
    [[nodiscard]] std::string commsDefAccessCodeByCommsInternal() const;
    [[nodiscard]] std::string commsDefAccessCodeGeneratedInternal() const;
    [[nodiscard]] std::string commsDefFieldExecCodeInternal() const;
    [[nodiscard]] std::string commsDefResetCodeInternal() const;
    [[nodiscard]] std::string commsDefCanWriteCodeInternal() const;
    [[nodiscard]] std::string commsDefSelectFieldCodeInternal() const;

    void commsAddCustomReadOptInternal(GenStringsList& opts) const;
    [[nodiscard]] std::string commsOptimizedReadKeyInternal() const;
    [[nodiscard]] const std::string& commsCommonMemberNameMapInternal() const;
    [[nodiscard]] std::string commsCommonMemberNameFuncsCodeInternal() const;
    [[nodiscard]] std::string commsDefMemberNamesTypesInternal() const;
    [[nodiscard]] std::string commsDefMemberNamesFuncsInternal() const;

    CommsFieldsList m_commsMembers;
    std::string m_optimizedReadKey;
};

} // namespace commsdsl2comms
