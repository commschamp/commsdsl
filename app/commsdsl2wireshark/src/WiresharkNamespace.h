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

#include "commsdsl/gen/GenNamespace.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkInterface;
class WiresharkNamespace final : public commsdsl::gen::GenNamespace
{
    using GenBase = commsdsl::gen::GenNamespace;

public:
    using GenElem = commsdsl::gen::GenElem;
    using GenNamespace = commsdsl::gen::GenNamespace;

    WiresharkNamespace(WiresharkGenerator& generator, ParseNamespace parseObj, GenElem* parent);
    virtual ~WiresharkNamespace();

    [[nodiscard]] static WiresharkNamespace* wiresharkCast(GenNamespace* obj)
    {
        return static_cast<WiresharkNamespace*>(obj);
    }

    [[nodiscard]] static const WiresharkNamespace* wiresharkCast(const GenNamespace* obj)
    {
        return static_cast<const WiresharkNamespace*>(obj);
    }

    [[nodiscard]] const WiresharkInterface* wiresharkInterface() const;
    [[nodiscard]] std::string wiresharkDissectCode() const;
    [[nodiscard]] std::string wiresharkExtractorsRegCode() const;
    [[nodiscard]] bool wiresharkNeedsOptionalModeDefinition() const;
    [[nodiscard]] bool wiresharkNeedsCrcCalc() const;
};

} // namespace commsdsl2wireshark
