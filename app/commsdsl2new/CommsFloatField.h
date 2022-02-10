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

#include "commsdsl/gen/FloatField.h"

#include "CommsField.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsFloatField final : public commsdsl::gen::FloatField, public CommsField
{
    using Base = commsdsl::gen::FloatField;
    using CommsBase = CommsField;
public:
    CommsFloatField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsBaseClassDefImpl() const override;
    virtual std::string commsDefPublicCodeImpl() const override;
    virtual std::string commsDefValidFuncBodyImpl() const override;

private:
    std::string commsCommonHasSpecialsFuncCodeInternal() const;
    std::string commsCommonValueNamesMapCodeInternal() const;
    std::string commsCommonSpecialsCodeInternal() const;
    std::string commsCommonSpecialNamesMapCodeInternal() const;
    std::string commsDefFieldOptsInternal() const;
    std::string commsDefValueNamesMapCodeInternal() const;
    std::string commsDefConstructorCodeInternal() const;
    std::string commsDefHasSpecialsFuncCodeInternal() const;
    std::string commsDefSpecialsCodeInternal() const;
    std::string commsDefSpecialNamesMapCodeInternal() const;
    std::string commsDefDisplayDecimalsCodeInternal() const;

    void commsAddUnitsOptInternal(StringsList& opts) const;
    void commsAddVersionOptInternal(StringsList& opts) const;
    void commsAddInvalidOptInternal(StringsList& opts) const;

    StringsList commsValidNormalConditionsInternal() const;
    StringsList commsValidVersionBasedConditionsInternal() const;
};

} // namespace commsdsl2new
