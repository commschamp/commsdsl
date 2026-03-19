//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/parse/ParseLayer.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenSyncLayerImpl;
class COMMSDSL_API GenSyncLayer : public GenLayer
{
    using Base = GenLayer;
public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using ParseSyncLayer = commsdsl::parse::ParseSyncLayer;

    GenSyncLayer(GenGenerator& generator, ParseLayer parseObj, GenElem* parent = nullptr);
    virtual ~GenSyncLayer();

    GenField* genExternalEscField();
    const GenField* genExternalEscField() const;

    GenField* genMemberEscField();
    const GenField* genMemberEscField() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genForceCommsOrderImpl(GenLayersAccessList& layers, bool& success) const override final;

    ParseSyncLayer genSyncLayerDslObj() const;

private:
    std::unique_ptr<GenSyncLayerImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
