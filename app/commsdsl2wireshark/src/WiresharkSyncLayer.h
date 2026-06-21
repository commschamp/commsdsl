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

#include "commsdsl/gen/GenSyncLayer.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkSyncLayer final : public commsdsl::gen::GenSyncLayer, public WiresharkLayer
{
    using GenBase = commsdsl::gen::GenSyncLayer;
    using WiresharkBase = WiresharkLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    WiresharkSyncLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent);

protected:
    virtual std::string wiresharkDissectBodyImpl() const override;
    virtual std::string wiresharkExtraDissectCodeImpl() const override;
    virtual std::string wiresharkExtractorsRegCodeImpl() const override;

private:
    std::string wiresharkPrefixDissectCodeInternal() const;
    std::string wiresharkSuffixDissectCodeInternal() const;
    std::string wiresharkSeekPrefixFieldCodeInternal() const;
    std::string wiresharkSeekSuffixFieldCodeInternal() const;
    std::string wiresharkSyncValueCheckCodeInternal() const;
    std::string wiresharkAdjustSuffixOffsetInternal() const;
};

} // namespace commsdsl2wireshark
