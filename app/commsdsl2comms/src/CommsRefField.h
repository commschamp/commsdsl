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

#include "commsdsl/gen/GenRefField.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsRefField final : public commsdsl::gen::GenRefField, public CommsField
{
    using GenBase = commsdsl::gen::GenRefField;
    using CommsBase = CommsField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;
    using GenRefField = commsdsl::gen::GenRefField;

    CommsRefField(CommsGenerator& generator, ParseField parseObj, GenElem* parent);

    [[nodiscard]] static const CommsRefField* commsCast(const GenRefField* ptr)
    {
        return static_cast<const CommsRefField*>(ptr);
    }

    [[nodiscard]] static const CommsRefField* commsCast(const CommsField* ptr)
    {
        return static_cast<const CommsRefField*>(ptr);
    }

protected:
    // GenBase overrides
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

    // CommsBase overrides
    [[nodiscard]] virtual CommsIncludesList commsCommonIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsCommonCodeBaseClassImpl() const override;
    [[nodiscard]] virtual std::string commsCommonCodeBodyImpl() const override;
    [[nodiscard]] virtual std::string commsCommonMembersBaseClassImpl() const override;
    [[nodiscard]] virtual CommsIncludesList commsDefIncludesImpl() const override;
    [[nodiscard]] virtual std::string commsDefBaseClassImpl() const override;
    [[nodiscard]] virtual bool commsIsLimitedCustomizableImpl() const override;
    [[nodiscard]] virtual bool commsDefHasNameFuncImpl() const override;
    [[nodiscard]] virtual std::size_t commsMinLengthImpl() const override;
    [[nodiscard]] virtual std::size_t commsMaxLengthImpl() const override;
    [[nodiscard]] virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    [[nodiscard]] virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const override;
    virtual void commsCompOptChecksImpl(const std::string& accStr, GenStringsList& checks, const std::string& prefix) const override;
    [[nodiscard]] virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const override;
    [[nodiscard]] virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    [[nodiscard]] virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const override;

private:
    [[nodiscard]] std::string commsDefFieldOptsInternal() const;

    void commsAddProtocolOptInternal(GenStringsList& opts) const;
    void commsAddBitLengthOptInternal(GenStringsList& opts) const;

    CommsField* m_commsReferencedField = nullptr;
};

} // namespace commsdsl2comms
