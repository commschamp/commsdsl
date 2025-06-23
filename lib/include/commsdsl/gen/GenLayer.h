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
#include "commsdsl/parse/ParseLayer.h"
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenLayerImpl;
class COMMSDSL_API GenLayer : public GenElem
{
    using Base = GenElem;
public:
    using Ptr = std::unique_ptr<GenLayer>;
    using LayersAccessList = std::vector<const GenLayer*>;

    virtual ~GenLayer();

    static Ptr create(GenGenerator& generator, commsdsl::parse::ParseLayer dslObj, GenElem* parent = nullptr);    

    bool prepare();
    bool write() const;

    commsdsl::parse::ParseLayer dslObj() const;

    GenField* externalField();
    const GenField* externalField() const;
    GenField* memberField();
    const GenField* memberField() const;

    GenGenerator& generator();
    const GenGenerator& generator() const;    

    // return true if re-order happened, false otherwise
    bool forceCommsOrder(LayersAccessList& layers, bool& success) const;

    std::string templateScopeOfComms(const std::string& iFaceStr, const std::string& allMessagesStr, const std::string& protOptionsStr) const;

protected:    
    GenLayer(GenGenerator& generator, const commsdsl::parse::ParseLayer& dslObj, GenElem* parent = nullptr);

    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;
    virtual bool forceCommsOrderImpl(LayersAccessList& layers, bool& success) const;


private:
    std::unique_ptr<GenLayerImpl> m_impl;
};

using LayerPtr = GenLayer::Ptr;

} // namespace gen

} // namespace commsdsl
