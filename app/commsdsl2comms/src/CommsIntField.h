//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/IntField.h"

#include "CommsField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsIntField final : public commsdsl::gen::IntField, public CommsField
{
    using Base = commsdsl::gen::IntField;
    using CommsBase = CommsField;
public:
    CommsIntField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

    std::string commsVariantPropKeyType() const;
    std::string commsVariantPropKeyValueStr() const;
    bool commsVariantIsValidPropKey() const;

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsDefPublicCodeImpl() const override;
    virtual std::string commsDefRefreshFuncBodyImpl() const override;
    virtual std::string commsDefValidFuncBodyImpl() const override;
    virtual std::size_t commsMinLengthImpl() const override;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;

private:
    std::string commsCommonHasSpecialsFuncCodeInternal() const;
    std::string commsCommonValueNamesMapCodeInternal() const;
    std::string commsCommonSpecialsCodeInternal() const;
    std::string commsCommonSpecialNamesMapCodeInternal() const;
    std::string commsDefFieldOptsInternal(bool variantPropKey = false) const;
    std::string commsDefValueNamesMapCodeInternal() const;
    std::string commsDefHasSpecialsFuncCodeInternal() const;
    std::string commsDefSpecialsCodeInternal() const;
    std::string commsDefSpecialNamesMapCodeInternal() const;
    std::string commsDefDisplayDecimalsCodeInternal() const;
    std::string commsDefBaseClassInternal(bool variantPropKey = false) const;

    void commsAddLengthOptInternal(StringsList& opts) const;
    void commsAddSerOffsetOptInternal(StringsList& opts) const;
    void commsAddScalingOptInternal(StringsList& opts) const;
    void commsAddUnitsOptInternal(StringsList& opts) const;
    void commsAddDefaultValueOptInternal(StringsList& opts) const;
    void commsAddValidRangesOptInternal(StringsList& opts) const;
    void commsAddCustomRefreshOptInternal(StringsList& opts) const;
    void commsAddAvailableLengthLimitOptInternal(StringsList& opts) const;
    bool commsRequiresFailOnInvalidRefreshInternal() const;
};

} // namespace commsdsl2comms
