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

#include "commsdsl/gen/GenSetField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsSetField final : public commsdsl::gen::GenSetField, public CommsField
{
    using GenBase = commsdsl::gen::GenSetField;
    using CommsBase = CommsField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    CommsSetField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

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
    [[nodiscard]] virtual std::string commsDefValidFuncBodyImpl() const override;
    [[nodiscard]] virtual bool commsIsVersionDependentImpl() const override;
    [[nodiscard]] virtual std::size_t commsMinLengthImpl() const override;
    [[nodiscard]] virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    [[nodiscard]] virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    [[nodiscard]] std::string commsCommonBitNameFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefFieldOptsInternal() const;
    [[nodiscard]] std::string commsDefBitsAccessCodeInternal() const;
    [[nodiscard]] std::string commsDefBitNameFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefMasksCodeInternal() const;

    void commsAddLengthOptInternal(commsdsl::gen::util::GenStringsList& opts) const;
    void commsAddDefaultValueOptInternal(commsdsl::gen::util::GenStringsList& opts) const;
    void commsAddReservedBitsOptInternal(commsdsl::gen::util::GenStringsList& opts) const;
    void commsAddAvailableLengthLimitOptInternal(GenStringsList& opts) const;
};

} // namespace commsdsl2comms
