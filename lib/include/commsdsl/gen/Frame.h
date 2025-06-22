//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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
#include "commsdsl/parse/ParseFrame.h"
#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Layer.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class FrameImpl;
class Namespace;

class COMMSDSL_API Frame : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Frame>;
    using LayersList = std::vector<LayerPtr>;
    using LayersAccessList = Layer::LayersAccessList;

    explicit Frame(Generator& generator, commsdsl::parse::ParseFrame dslObj, Elem* parent = nullptr);
    virtual ~Frame();

    bool prepare();
    bool write() const;

    commsdsl::parse::ParseFrame dslObj() const;
    const LayersList& layers() const;

    Generator& generator();
    const Generator& generator() const;

    LayersAccessList getCommsOrderOfLayers(bool& success) const;

    const Namespace* parentNamespace() const;

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;

private:
    std::unique_ptr<FrameImpl> m_impl;
};

using FramePtr = Frame::Ptr;

} // namespace gen

} // namespace commsdsl
