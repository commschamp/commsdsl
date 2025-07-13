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

#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2swig
{

class SwigLayer
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using SwigLayersList = std::vector<SwigLayer*>;
    
    explicit SwigLayer(commsdsl::gen::GenLayer& layer);
    virtual ~SwigLayer();

    static const SwigLayer* swigCast(const commsdsl::gen::GenLayer* layer);

    commsdsl::gen::GenLayer& swigGenLayer()
    {
        return m_genLayer;
    }

    const commsdsl::gen::GenLayer& swigGenLayer() const
    {
        return m_genLayer;
    }

    std::string swigDeclCode() const;
    void swigAddDef(GenStringsList& list) const;
    void swigAddCode(GenStringsList& list) const;
    void swigAddToAllFieldsDecl(GenStringsList& list) const;

    bool swigIsMainInterfaceSupported() const;
    std::string swigFieldAccName() const;

protected:
    virtual std::string swigDeclFuncsImpl() const;    
    virtual std::string swigCodeFuncsImpl() const;    
    virtual bool swigIsMainInterfaceSupportedImpl() const;
    virtual std::string swigMemberFieldDeclImpl() const;
    virtual void swigAddCodeImpl(GenStringsList& list) const;
    virtual std::string swigFieldTypeImpl() const;

    std::string swigTemplateScope() const;
    
private:
    commsdsl::gen::GenLayer& m_genLayer;
};

} // namespace commsdsl2swig
