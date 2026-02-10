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

#include "commsdsl/gen/GenIntField.h"

#include <string>

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkIntField final : public commsdsl::gen::GenIntField, public WiresharkField
{
    using GenBase = commsdsl::gen::GenIntField;
    using WiresharkBase = WiresharkField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    WiresharkIntField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent);

    static const std::string& wiresharkIntegralType(ParseIntField::ParseType type, std::size_t len);

protected:
    std::string wiresharkFieldRegistrationImpl(const WiresharkField* refField) const override;

private:
    std::string wiresharkSpecialsInternal(const WiresharkField* refField) const;
};

} // namespace commsdsl2wireshark
