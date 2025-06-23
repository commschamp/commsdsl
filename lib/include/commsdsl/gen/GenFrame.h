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
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenLayer.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenFrameImpl;
class GenNamespace;

class COMMSDSL_API GenFrame : public GenElem
{
    using Base = GenElem;
public:
    using Ptr = std::unique_ptr<GenFrame>;
    using LayersList = std::vector<LayerPtr>;
    using LayersAccessList = GenLayer::LayersAccessList;

    explicit GenFrame(GenGenerator& generator, commsdsl::parse::ParseFrame dslObj, GenElem* parent = nullptr);
    virtual ~GenFrame();

    bool prepare();
    bool write() const;

    commsdsl::parse::ParseFrame dslObj() const;
    const LayersList& layers() const;

    GenGenerator& generator();
    const GenGenerator& generator() const;

    LayersAccessList getCommsOrderOfLayers(bool& success) const;

    const GenNamespace* parentNamespace() const;

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;

private:
    std::unique_ptr<GenFrameImpl> m_impl;
};

using FramePtr = GenFrame::Ptr;

} // namespace gen

} // namespace commsdsl
