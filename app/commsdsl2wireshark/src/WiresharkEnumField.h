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

#include "commsdsl/gen/GenEnumField.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkEnumField final : public commsdsl::gen::GenEnumField, public WiresharkField
{
    using GenBase = commsdsl::gen::GenEnumField;
    using WiresharkBase = WiresharkField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    WiresharkEnumField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    virtual bool genPrepareImpl() override;
    virtual std::string wiresharkFieldRegistrationImpl(const WiresharkField* refField) const override;
    virtual std::string wiresharkTvbRangeAccessImpl() const override;
    virtual std::string wiresharkDissectLengthCheckImpl(const WiresharkField* refField) const override;
    virtual std::string wiresharkDissectBodyImpl(const WiresharkField* refField) const override;
    virtual std::string wiresharkValidFuncBodyImpl(const WiresharkField* refField) const override;
    virtual bool wiresharkHasTrivialValidImpl() const override;

private:
    std::string wiresharkValsInternal(const WiresharkField* refField) const;
    std::string wiresharkValDeclCodeInternal() const;
    std::string wiresharkVarLengthCodeInternal(bool& hasVal) const;
    std::string wiresharkVarLengthCodeLargeNumInternal() const;
    std::string wiresharkVarLengthCodeLittleEndianInternal() const;
    std::string wiresharkVarLengthCodeBigEndianInternal() const;
};

} // namespace commsdsl2wireshark
