//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "WiresharkField.h"

#include "commsdsl/gen/GenOptionalField.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkOptionalField final : public commsdsl::gen::GenOptionalField, public WiresharkField
{
    using GenBase = commsdsl::gen::GenOptionalField;
    using WiresharkBase = WiresharkField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    WiresharkOptionalField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual std::string wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const override;
    [[nodiscard]] virtual std::string wiresharkFieldRegistrationImpl(const WiresharkField* refField) const override;
    [[nodiscard]] virtual std::string wiresharkMembersDissectCodeImpl() const override;
    [[nodiscard]] virtual std::string wiresharkDissectBodyImpl(const WiresharkField* refField) const override;
    [[nodiscard]] virtual std::string wiresharkValidFuncBodyImpl(const WiresharkField* refField) const override;
    [[nodiscard]] virtual std::string wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const override;
    [[nodiscard]] virtual std::string wiresharkSizeAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const override;
    [[nodiscard]] virtual std::string wiresharkCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const override;
    [[nodiscard]] virtual std::string wiresharkExistsCheckStrImpl(const std::string& accStr, const WiresharkField* refField) const override;
    [[nodiscard]] virtual bool wiresharkHasTrivialValidImpl() const override;
    [[nodiscard]] virtual std::string wiresharkDefaultValueStrImpl() const override;

private:
    [[nodiscard]] std::string wiresharkDefaultModeInternal() const;
    [[nodiscard]] std::string wiresharkReadFailCodeInternal() const;
    [[nodiscard]] std::string wiresharkMissInvalidCodeInternal() const;
    [[nodiscard]] std::string wiresharkCondCodeInternal() const;
    [[nodiscard]] std::string wiresharkTentativeCheckCodeInternal() const;

    const WiresharkField* m_wiresharkField = nullptr;
};

} // namespace commsdsl2wireshark
