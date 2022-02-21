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

#include "CommsField.h"

#include "commsdsl/gen/BundleField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsBundleField final : public commsdsl::gen::BundleField, public CommsField
{
    using Base = commsdsl::gen::BundleField;
    using CommsBase = CommsField;
public:
    CommsBundleField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

    const CommsFieldsList& commsMembers() const
    {
        return m_members;
    }

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonMembersCodeImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefMembersCodeImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsDefPublicCodeImpl() const override;
    virtual std::string commsDefPrivateCodeImpl() const override;
    virtual std::string commsDefReadFuncBodyImpl() const override;
    virtual std::string commsDefRefreshFuncBodyImpl() const override;    
    virtual bool commsIsVersionDependentImpl() const override;

private:
    bool commsPrepareInternal();
    std::string commsDefFieldOptsInternal() const;
    std::string commsDefAccessCodeInternal() const;
    std::string commsDefAliasesCodeInternal() const;

    void commsAddCustomReadRefreshOptInternal(StringsList& opts) const;

    CommsFieldsList m_members;
    StringsList m_bundledReadPrepareCodes;
    StringsList m_bundledRefreshCodes;
};

} // namespace commsdsl2new
