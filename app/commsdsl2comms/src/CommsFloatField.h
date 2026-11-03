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

#include "commsdsl/gen/GenFloatField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsFloatField final : public commsdsl::gen::GenFloatField, public CommsField
{
    using GenBase = commsdsl::gen::GenFloatField;
    using CommsBase = CommsField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    CommsFloatField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    // GenBase overrides
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

    // CommsBase overrides
    [[nodiscard]] virtual CommsIncludesList commsCommonIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsCommonCodeBodyImpl() const override;
    [[nodiscard]] virtual CommsIncludesList commsDefIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsDefConstructCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefBaseClassImpl() const override;
    [[nodiscard]] virtual std::string commsDefPublicCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefValidFuncBodyImpl() const override;
    [[nodiscard]] virtual bool commsIsVersionDependentImpl() const override;
    [[nodiscard]] virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    [[nodiscard]] std::string commsCommonHasSpecialsFuncCodeInternal() const;
    [[nodiscard]] std::string commsCommonValueNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsCommonSpecialsCodeInternal() const;
    [[nodiscard]] std::string commsCommonSpecialNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsDefFieldOptsInternal() const;
    [[nodiscard]] std::string commsDefValueNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsDefHasSpecialsFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefSpecialsCodeInternal() const;
    [[nodiscard]] std::string commsDefSpecialNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsDefDisplayDecimalsCodeInternal() const;

    void commsAddUnitsOptInternal(GenStringsList& opts) const;
    void commsAddVersionOptInternal(GenStringsList& opts) const;
    void commsAddInvalidOptInternal(GenStringsList& opts) const;

    [[nodiscard]] GenStringsList commsValidNormalConditionsInternal() const;
    [[nodiscard]] GenStringsList commsValidVersionBasedConditionsInternal() const;
};

} // namespace commsdsl2comms
