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

#include "commsdsl/gen/BitfieldField.h"
#include "commsdsl/gen/util.h"

#include <utility>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsBitfieldField final : public commsdsl::gen::BitfieldField, public CommsField
{
    using Base = commsdsl::gen::BitfieldField;
    using CommsBase = CommsField;
public:
    CommsBitfieldField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

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
    virtual bool commsIsVersionDependentImpl() const override;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const override;
    virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual void commsCompOptChecksImpl(const std::string& accStr, StringsList& checks, const std::string& prefix) const override;
    virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    virtual bool commsHasCustomLengthDeepImpl() const override;
    virtual void commsSetReferencedImpl() override;

private:
    bool commsPrepareInternal();
    std::string commsDefFieldOptsInternal() const;
    std::string commsAccessCodeInternal() const;
    std::pair<const CommsField*, std::string> parseMemRefInternal(const std::string accStr) const;

    CommsFieldsList m_members;
};

} // namespace commsdsl2comms
