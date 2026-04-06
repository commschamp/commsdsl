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

#include "commsdsl/gen/GenSchema.h"

namespace commsdsl2wireshark
{

class WiresharkGenerator;
class WiresharkSchema final : public commsdsl::gen::GenSchema
{
    using GenBase = commsdsl::gen::GenSchema;

public:
    using GenElem = commsdsl::gen::GenElem;
    using GenSchema = commsdsl::gen::GenSchema;

    WiresharkSchema(WiresharkGenerator& generator, ParseSchema parseObj, GenElem* parent);
    virtual ~WiresharkSchema();

    static WiresharkSchema& wiresharkCast(GenSchema& obj)
    {
        return static_cast<WiresharkSchema&>(obj);
    }

    static const WiresharkSchema& wiresharkCast(const GenSchema& obj)
    {
        return static_cast<const WiresharkSchema&>(obj);
    }

    std::string wiresharkDissectCode() const;
    std::string wiresharkExtractorsRegCode() const;
    bool wiresharkNeedsOptionalModeDefinition() const;
};

} // namespace commsdsl2wireshark
