//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class CustomLayer : public Layer
{
    using Base = Layer;
public:
    CustomLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

    bool isIdReplacement() const
    {
        return customLayerDslObj().isIdReplacement();
    }

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override final;
    virtual bool isCustomizableImpl() const override final;

private:
    commsdsl::CustomLayer customLayerDslObj() const
    {
        return commsdsl::CustomLayer(dslObj());
    }
};

inline
LayerPtr createCustomLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<CustomLayer>(generator, layer);
}

} // namespace commsdsl2comms
