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

#include "WiresharkLayer.h"

#include "commsdsl/gen/GenIdLayer.h"

#include <string>

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkIdLayer final : public commsdsl::gen::GenIdLayer, public WiresharkLayer
{
    using GenBase = commsdsl::gen::GenIdLayer;
    using WiresharkBase = WiresharkLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    WiresharkIdLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent);

protected:
    virtual std::string wiresharkDissectBodyImpl() const override;
    virtual std::string wiresharkExtraDissectCodeImpl() const override;

private:
    std::string wiresharkMsgMapNameInternal() const;
};

} // namespace commsdsl2wireshark
