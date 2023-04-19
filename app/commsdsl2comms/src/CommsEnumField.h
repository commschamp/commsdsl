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

#include "CommsField.h"

#include "commsdsl/gen/EnumField.h"
#include "commsdsl/gen/util.h"

#include <vector>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsEnumField final : public commsdsl::gen::EnumField, public CommsField
{
    using Base = commsdsl::gen::EnumField;
    using CommsBase = CommsField;
public:
    CommsEnumField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

    commsdsl::gen::util::StringsList commsEnumValues() const;

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonCodeExtraImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefExtraDoxigenImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsDefPublicCodeImpl() const override;
    virtual std::string commsDefValidFuncBodyImpl() const override;
    virtual bool commsIsVersionDependentImpl() const override;
    virtual std::size_t commsMinLengthImpl() const override;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    struct RangeInfo
    {
        std::intmax_t m_min = 0;
        std::intmax_t m_max = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = commsdsl::parse::Protocol::notYetDeprecated();
    };

    using ValidRangesList = std::vector<RangeInfo>;

    bool commsPrepareValidRangesInternal();
    bool commsIsDirectValueNameMappingInternal() const;
    std::string commsCommonEnumInternal() const;
    std::string commsCommonValueNameMapInternal() const;
    std::string commsCommonValueNameFuncCodeInternal() const;
    const std::string& commsCommonValueNameDirectBodyInternal() const;
    const std::string& commsCommonValueNameBinSearchBodyInternal() const;
    std::string commsCommonValueNamesMapFuncCodeInternal() const;
    std::string commsCommonValueNamesMapDirectBodyInternal() const;
    std::string commsCommonValueNamesMapBinSearchBodyInternal() const;
    std::string commsCommonBigUnsignedValueNameBinSearchPairsInternal() const;
    std::string commsCommonValueNameBinSearchPairsInternal() const;
    std::string commsDefFieldOptsInternal() const;
    std::string commsDefValueNameMapInternal() const;
    std::string commsDefValueNameFuncCodeInternal() const;
    std::string commsDefValueNamesMapFuncCodeInternal() const;

    void commsAddDefaultValueOptInternal(StringsList& opts) const;
    void commsAddLengthOptInternal(StringsList& opts) const;
    void commsAddValidRangesOptInternal(StringsList& opts) const;
    void commsAddAvailableLengthLimitOptInternal(StringsList& opts) const;

    ValidRangesList m_validRanges;
};

} // namespace commsdsl2comms
