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

#include "commsdsl/gen/GenFrame.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkInterface;
class WiresharkFrame final : public commsdsl::gen::GenFrame
{
    using GenBase = commsdsl::gen::GenFrame;

public:
    using GenElem = commsdsl::gen::GenElem;
    using GenFrame = commsdsl::gen::GenFrame;

    using WiresharkLayersAccessList = std::vector<const WiresharkLayer*>;

    WiresharkFrame(WiresharkGenerator& generator, ParseFrame parseObj, GenElem* parent);
    virtual ~WiresharkFrame();

    static WiresharkFrame& wiresharkCast(GenFrame& obj)
    {
        return static_cast<WiresharkFrame&>(obj);
    }

    static const WiresharkFrame& wiresharkCast(const GenFrame& obj)
    {
        return static_cast<const WiresharkFrame&>(obj);
    }

    std::string wiresharkDissectName() const;
    std::string wiresharkDissectCode() const;

protected:
    virtual bool genPrepareImpl() override;

private:
    std::string wiresharkDissectBodyInternal() const;
    std::string wiresharkLayersDissectCodeInternal() const;
    const WiresharkInterface* wiresharkInterfaceInternal() const;

    WiresharkLayersAccessList m_wiresharkLayers;
    bool m_validFrame = false;
};

} // namespace commsdsl2wireshark
