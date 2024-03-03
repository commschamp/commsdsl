//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/SyncLayer.h"

#include "CommsLayer.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsSyncLayer final : public commsdsl::gen::SyncLayer, public CommsLayer
{
    using Base = commsdsl::gen::SyncLayer;
    using CommsBase = CommsLayer;
public:
    CommsSyncLayer(CommsGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent);

protected:
    virtual bool prepareImpl() override;
    
    // CommsBase overrides
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefBaseTypeImpl(const std::string& prevName) const override;
};

} // namespace commsdsl2comms
