//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/Layer.h"
#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Field.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class LayerImpl;
class COMMSDSL_API Layer : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Layer>;

    virtual ~Layer();

    static Ptr create(Generator& generator, commsdsl::parse::Layer dslObj, Elem* parent = nullptr);    

    bool prepare();
    bool write() const;

    commsdsl::parse::Layer dslObj() const;

    Field* externalField();
    const Field* externalField() const;
    Field* memberField();
    const Field* memberField() const;

    Generator& generator();
    const Generator& generator() const;    

protected:    
    Layer(Generator& generator, const commsdsl::parse::Layer& dslObj, Elem* parent = nullptr);

    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;


private:
    std::unique_ptr<LayerImpl> m_impl;
};

using LayerPtr = Layer::Ptr;

} // namespace gen

} // namespace commsdsl
