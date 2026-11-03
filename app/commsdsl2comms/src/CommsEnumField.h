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

#include "commsdsl/gen/GenEnumField.h"
#include "commsdsl/gen/util.h"

#include <vector>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsNamespace;

class CommsEnumField final : public commsdsl::gen::GenEnumField, public CommsField
{
    using GenBase = commsdsl::gen::GenEnumField;
    using CommsBase = CommsField;
public:
    using ParseField = commsdsl::parse::ParseField;

    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    CommsEnumField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

    [[nodiscard]] GenStringsList commsEnumValues() const;

protected:
    // GenBase overrides
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

    // CommsBase overrides
    [[nodiscard]] virtual CommsIncludesList commsCommonIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsCommonCodeBodyImpl() const override;
    [[nodiscard]] virtual std::string commsCommonCodeExtraImpl() const override;
    [[nodiscard]] virtual CommsIncludesList commsDefIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsDefExtraDoxigenImpl() const override;
    [[nodiscard]] virtual std::string commsDefBaseClassImpl() const override;
    [[nodiscard]] virtual std::string commsDefPublicCodeImpl() const override;
    [[nodiscard]] virtual std::string commsDefValidFuncBodyImpl() const override;
    [[nodiscard]] virtual bool commsIsVersionDependentImpl() const override;
    [[nodiscard]] virtual std::size_t commsMinLengthImpl() const override;
    [[nodiscard]] virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    [[nodiscard]] virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    struct CommsRangeInfo
    {
        std::intmax_t m_min = 0;
        std::intmax_t m_max = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = commsdsl::parse::ParseProtocol::parseNotYetDeprecated();
    };

    using CommsValidRangesList = std::vector<CommsRangeInfo>;

    [[nodiscard]] bool commsPrepareValidRangesInternal();
    [[nodiscard]] bool commsIsDirectValueNameMappingInternal() const;
    [[nodiscard]] std::string commsCommonEnumInternal() const;
    [[nodiscard]] std::string commsCommonValueNameMapInternal() const;
    [[nodiscard]] std::string commsCommonValueNameFuncCodeInternal() const;
    [[nodiscard]] const std::string& commsCommonValueNameDirectBodyInternal() const;
    [[nodiscard]] const std::string& commsCommonValueNameBinSearchBodyInternal() const;
    [[nodiscard]] std::string commsCommonValueNamesMapFuncCodeInternal() const;
    [[nodiscard]] std::string commsCommonValueNamesMapDirectBodyInternal() const;
    [[nodiscard]] std::string commsCommonValueNamesMapBinSearchBodyInternal() const;
    [[nodiscard]] std::string commsCommonBigUnsignedValueNameBinSearchPairsInternal() const;
    [[nodiscard]] std::string commsCommonValueNameBinSearchPairsInternal() const;
    [[nodiscard]] std::string commsDefFieldOptsInternal() const;
    [[nodiscard]] std::string commsDefValueNameMapInternal() const;
    [[nodiscard]] std::string commsDefValueNameFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefValueNamesMapFuncCodeInternal() const;

    void commsAddDefaultValueOptInternal(GenStringsList& opts) const;
    void commsAddLengthOptInternal(GenStringsList& opts) const;
    void commsAddValidRangesOptInternal(GenStringsList& opts) const;
    void commsAddAvailableLengthLimitOptInternal(GenStringsList& opts) const;
    [[nodiscard]] const CommsNamespace* commsNamespaceForMsgId() const;

    CommsValidRangesList m_validRanges;
};

} // namespace commsdsl2comms
