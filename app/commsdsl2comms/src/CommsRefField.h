//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenRefField.h"

#include "CommsField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsRefField final : public commsdsl::gen::GenRefField, public CommsField
{
    using Base = commsdsl::gen::GenRefField;
    using CommsBase = CommsField;
public:
    CommsRefField(CommsGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent);

protected:
    // Base overrides
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBaseClassImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonMembersBaseClassImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual bool commsIsLimitedCustomizableImpl() const override;
    virtual bool commsDefHasNameFuncImpl() const override;
    virtual std::size_t commsMinLengthImpl() const override;
    virtual std::size_t commsMaxLengthImpl() const override;
    virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override; 
    virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const override; 
    virtual void commsCompOptChecksImpl(const std::string& accStr, StringsList& checks, const std::string& prefix) const override;
    virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    std::string commsDefFieldOptsInternal() const;

    void commsAddProtocolOptInternal(StringsList& opts) const;
    void commsAddBitLengthOptInternal(StringsList& opts) const;

    CommsField* m_commsReferencedField = nullptr;
};

} // namespace commsdsl2comms
