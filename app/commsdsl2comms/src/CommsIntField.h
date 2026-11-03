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

#include "commsdsl/gen/GenIntField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsIntField final : public commsdsl::gen::GenIntField, public CommsField
{
    using GenBase = commsdsl::gen::GenIntField;
    using CommsBase = CommsField;
public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    CommsIntField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

    [[nodiscard]] std::string commsVariantPropKeyType() const;
    [[nodiscard]] std::string commsVariantPropKeyValueStr() const;
    [[nodiscard]] bool commsVariantIsValidPropKey() const;
    [[nodiscard]] bool commsVariantIsPropKeyEquivalent(const CommsIntField& other) const;

protected:
    // GenBase overrides
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

    // CommsBase overrides
    [[nodiscard]] virtual CommsIncludesList commsCommonIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsCommonCodeBodyImpl() const override;
    [[nodiscard]] virtual CommsIncludesList commsDefIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsDefBaseClassImpl() const override;
    [[nodiscard]] virtual std::string commsDefPublicCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefRefreshFuncBodyImpl() const override;
    [[nodiscard]] virtual std::string commsDefValidFuncBodyImpl() const override;
    [[nodiscard]] virtual bool commsIsVersionDependentImpl() const override;
    [[nodiscard]] virtual std::size_t commsMinLengthImpl() const override;
    [[nodiscard]] virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    [[nodiscard]] virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    [[nodiscard]] std::string commsCommonHasSpecialsFuncCodeInternal() const;
    [[nodiscard]] std::string commsCommonValueNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsCommonSpecialsCodeInternal() const;
    [[nodiscard]] std::string commsCommonSpecialNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsDefFieldOptsInternal(bool variantPropKey = false) const;
    [[nodiscard]] std::string commsDefValueNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsDefHasSpecialsFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefSpecialsCodeInternal() const;
    [[nodiscard]] std::string commsDefSpecialNamesMapCodeInternal() const;
    [[nodiscard]] std::string commsDefDisplayDecimalsCodeInternal() const;
    [[nodiscard]] std::string commsDefBaseClassInternal(bool variantPropKey = false) const;

    void commsAddLengthOptInternal(GenStringsList& opts) const;
    void commsAddSerOffsetOptInternal(GenStringsList& opts) const;
    void commsAddDisplayOffsetOptInternal(GenStringsList& opts) const;
    void commsAddScalingOptInternal(GenStringsList& opts) const;
    void commsAddUnitsOptInternal(GenStringsList& opts) const;
    void commsAddDefaultValueOptInternal(GenStringsList& opts) const;
    void commsAddValidRangesOptInternal(GenStringsList& opts) const;
    void commsAddCustomRefreshOptInternal(GenStringsList& opts) const;
    void commsAddAvailableLengthLimitOptInternal(GenStringsList& opts) const;
    [[nodiscard]] bool commsRequiresFailOnInvalidRefreshInternal() const;
};

} // namespace commsdsl2comms
