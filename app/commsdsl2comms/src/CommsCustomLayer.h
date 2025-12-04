//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsLayer.h"

#include "commsdsl/gen/GenCustomLayer.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsCustomLayer final : public commsdsl::gen::GenCustomLayer, public CommsLayer
{
    using GenBase = commsdsl::gen::GenCustomLayer;
    using CommsBase = CommsLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    CommsCustomLayer(CommsGenerator& generator, ParseLayer parseObj, GenElem* parent);

protected:
    virtual bool genPrepareImpl() override;

    // CommsBase overrides
    virtual CommsIncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefBaseTypeImpl(const std::string& prevName) const override;
    virtual bool commsDefHasInputMessagesImpl() const override;
    virtual bool commsIsCustomizableImpl() const override;
    virtual GenStringsList commsExtraBareMetalDefaultOptionsImpl() const override;
    virtual GenStringsList commsExtraMsgFactoryDefaultOptionsImpl() const override;
};

} // namespace commsdsl2comms
