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

#include "CommsField.h"

#include "commsdsl/gen/GenListField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsListField final : public commsdsl::gen::GenListField, public CommsField
{
    using Base = commsdsl::gen::GenListField;
    using CommsBase = CommsField;
public:
    CommsListField(CommsGenerator& generator, commsdsl::parse::ParseField parseObj, commsdsl::gen::GenElem* parent);

protected:
    // Base overrides
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

    // CommsBase overrides
    virtual CommsIncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonMembersCodeImpl() const override;
    virtual CommsIncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefMembersCodeImpl() const override;
    virtual std::string commsDefBaseClassImpl() const override;
    virtual std::string commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual std::string commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const override;
    virtual bool commsIsLimitedCustomizableImpl() const override;
    virtual bool commsIsVersionDependentImpl() const override;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const override;
    virtual GenStringsList commsExtraBareMetalDefaultOptionsImpl() const override;
    virtual std::size_t commsMaxLengthImpl() const override;
    virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;

private:
    std::string commsDefFieldOptsInternal() const;
    std::string commsDefElementInternal() const;

    void commsAddFixedLengthOptInternal(GenStringsList& opts) const;
    void commsAddCountPrefixOptInternal(GenStringsList& opts) const;
    void commsAddLengthPrefixOptInternal(GenStringsList& opts) const;
    void commsAddElemLengthPrefixOptInternal(GenStringsList& opts) const;
    void commsAddTermSuffixOptInternal(GenStringsList& opts) const;
    void commsAddLengthForcingOptInternal(GenStringsList& opts) const;

    CommsField* m_commsExternalElementField = nullptr;
    CommsField* m_commsMemberElementField = nullptr;
    CommsField* m_commsExternalCountPrefixField = nullptr;
    CommsField* m_commsMemberCountPrefixField = nullptr;
    CommsField* m_commsExternalLengthPrefixField = nullptr;
    CommsField* m_commsMemberLengthPrefixField = nullptr;
    CommsField* m_commsExternalElemLengthPrefixField = nullptr;
    CommsField* m_commsMemberElemLengthPrefixField = nullptr;
    CommsField* m_commsExternalTermSuffixField = nullptr;
    CommsField* m_commsMemberTermSuffixField = nullptr;

};

} // namespace commsdsl2comms
