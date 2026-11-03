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

class WiresharkField;
class WiresharkInterface;
class WiresharkLayer
{
public:
    using GenLayer = commsdsl::gen::GenLayer;

    explicit WiresharkLayer(GenLayer& layer);
    virtual ~WiresharkLayer();

    [[nodiscard]] static const WiresharkLayer* wiresharkCast(const GenLayer* layer);
    [[nodiscard]] static WiresharkLayer* wiresharkCast(GenLayer* layer);

    [[nodiscard]] GenLayer& wiresharkGenLayer()
    {
        return m_genLayer;
    }

    [[nodiscard]] const GenLayer& wiresharkGenLayer() const
    {
        return m_genLayer;
    }

    [[nodiscard]] std::string wiresharkDissectName() const;
    [[nodiscard]] std::string wiresharkDissectCode() const;
    [[nodiscard]] std::string wiresharkExtractorsRegCode() const;

    [[nodiscard]] bool wiresharkIsInterfaceSupported(const WiresharkInterface& iFace) const;
    [[nodiscard]] bool wiresharkNeedsOptionalModeDefinition() const;
    [[nodiscard]] bool wiresharkNeedsCrcCalc() const;

protected:
    [[nodiscard]] virtual std::string wiresharkDissectBodyImpl() const;
    [[nodiscard]] virtual bool wiresharkIsInterfaceSupportedImpl(const WiresharkInterface& iFace) const;
    [[nodiscard]] virtual std::string wiresharkExtraDissectCodeImpl() const;
    [[nodiscard]] virtual std::string wiresharkExtractorsRegCodeImpl() const;
    [[nodiscard]] virtual bool wiresharkNeedsCrcCalcImpl() const;

    [[nodiscard]] std::string wiresharkDissectFieldCode() const;
    [[nodiscard]] std::string wiresharkNextFuncCode() const;

    [[nodiscard]] const WiresharkField* wiresharkField() const;

private:
    [[nodiscard]] std::string wiresharkFieldDissectCodeInternal() const;
    [[nodiscard]] std::string wiresharkDissectBodyInternal() const;

    GenLayer& m_genLayer;
};
} // namespace commsdsl2wireshark
