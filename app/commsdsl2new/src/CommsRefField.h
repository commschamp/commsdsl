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

#include "commsdsl/gen/RefField.h"

#include "CommsField.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsRefField final : public commsdsl::gen::RefField, public CommsField
{
    using Base = commsdsl::gen::RefField;
    using CommsBase = CommsField;
public:
    CommsRefField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBaseClassImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonMembersBaseClassImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsCompareToValueCodeImpl(const std::string& op, const std::string& value, const std::string& nameOverride, bool forcedVersionOptional) const override;  
    virtual std::string commsCompareToFieldCodeImpl(const std::string& op, const CommsField& field, const std::string& nameOverride, bool forcedVersionOptional) const override;
    virtual bool commsDefHasNameFuncImpl() const override;

private:
    std::string commsDefFieldOptsInternal() const;

    void commsAddProtocolOptInternal(StringsList& opts) const;
    void commsAddBitLengthOptInternal(StringsList& opts) const;

    CommsField* m_commsReferencedField = nullptr;
};

} // namespace commsdsl2new
