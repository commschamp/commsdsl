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

#include "commsdsl/gen/GenDataField.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkDataField final : public commsdsl::gen::GenDataField, public WiresharkField
{
    using GenBase = commsdsl::gen::GenDataField;
    using WiresharkBase = WiresharkField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    WiresharkDataField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    virtual bool genPrepareImpl() override;
    virtual std::string wiresharkExtractorsRegCodeImpl(const WiresharkField* refField) const override;
    virtual std::string wiresharkMembersDissectCodeImpl() const override;
    virtual std::string wiresharkDissectBodyImpl(const WiresharkField* refField) const override;
    virtual std::string wiresharkValidFuncBodyImpl(const WiresharkField* refField) const override;
    virtual std::string wiresharkValueAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const override;
    virtual std::string wiresharkSizeAccessStrImpl(const std::string& accStr, const WiresharkField* refField) const override;
    virtual std::string wiresharkCompPrepValueStrImpl(const std::string& value) const override;
    virtual bool wiresharkHasTrivialValidImpl() const override;

private:
    std::string wiresharkDissectLengthCodeInternal() const;

    const WiresharkField* m_prefixField = nullptr;
};

} // namespace commsdsl2wireshark
