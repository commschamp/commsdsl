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

#include "commsdsl/gen/GenLayer.h"

#include <string>

namespace commsdsl2wireshark
{

class WiresharkInterface;
class WiresharkLayer
{
public:
    using GenLayer = commsdsl::gen::GenLayer;

    explicit WiresharkLayer(GenLayer& layer);
    virtual ~WiresharkLayer();

    static const WiresharkLayer* wiresharkCast(const GenLayer* layer);
    static WiresharkLayer* wiresharkCast(GenLayer* layer);

    GenLayer& wiresharkGenLayer()
    {
        return m_genLayer;
    }

    const GenLayer& wiresharkGenLayer() const
    {
        return m_genLayer;
    }

    std::string wiresharkDissectName() const;
    std::string wiresharkDissectCode() const;

    bool wiresharkIsInterfaceSupported(const WiresharkInterface& iFace) const;

protected:
    virtual std::string wiresharkDissectBodyImpl() const;
    virtual bool wiresharkIsInterfaceSupportedImpl(const WiresharkInterface& iFace) const;

private:
    std::string wiresharkFieldDissectCodeInternal() const;

    GenLayer& m_genLayer;
};
} // namespace commsdsl2wireshark
