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

#include "commsdsl/gen/GenIdLayer.h"

namespace commsdsl2comms
{

class CommsGenerator;
class CommsIdLayer final : public commsdsl::gen::GenIdLayer, public CommsLayer
{
    using Base = commsdsl::gen::GenIdLayer;
    using CommsBase = CommsLayer;
public:
    CommsIdLayer(CommsGenerator& generator, commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent);

protected:
    virtual bool genPrepareImpl() override;
    
    // CommsBase overrides
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefBaseTypeImpl(const std::string& prevName) const override;
    virtual bool commsDefHasInputMessagesImpl() const override;
    virtual bool commsIsCustomizableImpl() const override;    
    virtual StringsList commsExtraBareMetalDefaultOptionsImpl() const override;
    virtual StringsList commsExtraMsgFactoryDefaultOptionsImpl() const override;

private:
};

} // namespace commsdsl2comms
