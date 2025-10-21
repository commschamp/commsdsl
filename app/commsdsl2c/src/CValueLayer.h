//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CLayer.h"

#include "commsdsl/gen/GenValueLayer.h"

namespace commsdsl2c
{

class CGenerator;
class CValueLayer final : public commsdsl::gen::GenValueLayer, public CLayer
{
    using GenBase = commsdsl::gen::GenValueLayer;
    using CBase = CLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    CValueLayer(CGenerator& generator, ParseLayer parseObj, GenElem* parent);

protected:
    virtual bool genPrepareImpl() override;
    virtual std::string cHeaderCodeImpl() const override;    
    virtual std::string cSourceCodeImpl() const override;    
    virtual bool cIsInterfaceSupportedImpl(const CInterface& iFace) const override;

private:
    bool m_hasPseudoField = false;    
};

} // namespace commsdsl2c
