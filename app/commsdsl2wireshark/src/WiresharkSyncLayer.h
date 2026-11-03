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
    [[nodiscard]] virtual std::string wiresharkDissectBodyImpl() const override;
    [[nodiscard]] virtual std::string wiresharkExtraDissectCodeImpl() const override;
    [[nodiscard]] virtual std::string wiresharkExtractorsRegCodeImpl() const override;

private:
    [[nodiscard]] std::string wiresharkPrefixDissectCodeInternal() const;
    [[nodiscard]] std::string wiresharkSuffixDissectCodeInternal() const;
    [[nodiscard]] std::string wiresharkSeekPrefixFieldCodeInternal() const;
    [[nodiscard]] std::string wiresharkSeekSuffixFieldCodeInternal() const;
    [[nodiscard]] std::string wiresharkSyncValueCheckCodeInternal() const;
    [[nodiscard]] std::string wiresharkAdjustSuffixOffsetInternal() const;
};

} // namespace commsdsl2wireshark
