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

#include "commsdsl/gen/GenBundleField.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkBundleField final : public commsdsl::gen::GenBundleField, public WiresharkField
{
    using GenBase = commsdsl::gen::GenBundleField;
    using WiresharkBase = WiresharkField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    WiresharkBundleField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    virtual bool genPrepareImpl() override;

    virtual std::string wiresharkFieldRegistrationImpl(const WiresharkField* refField) const override;
    virtual std::string wiresharkMembersDissectCodeImpl() const override;

private:
    WiresharkFieldsList m_wiresharkFields;
};

} // namespace commsdsl2wireshark
