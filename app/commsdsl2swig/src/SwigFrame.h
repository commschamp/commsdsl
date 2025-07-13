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

#include "SwigLayer.h"

#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigFrame final: public commsdsl::gen::GenFrame
{
    using GenBase = commsdsl::gen::GenFrame;

public:
    using ParseFrame = commsdsl::parse::ParseFrame;

    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    
    using SwigLayersList = SwigLayer::SwigLayersList;

    explicit SwigFrame(SwigGenerator& generator, ParseFrame parseObj, GenElem* parent);
    virtual ~SwigFrame();

    void swigAddCodeIncludes(GenStringsList& list) const;
    void swigAddCode(GenStringsList& list) const;
    void swigAddDef(GenStringsList& list) const;

    static const SwigFrame* swigCast(const commsdsl::gen::GenFrame* i)
    {
        return static_cast<const SwigFrame*>(i);
    }        

    const SwigLayersList& swigLayers() const
    {
        return m_swigLayers;
    }

protected:    
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

private:

    std::string swigLayerDeclsInternal() const;
    std::string swigClassDeclInternal() const;
    std::string swigLayersAccDeclInternal() const;
    std::string swigLayersAccCodeInternal() const;
    std::string swigFrameCodeInternal() const;
    std::string swigAllFieldsInternal() const;

    SwigLayersList m_swigLayers;
    bool m_validFrame = true;
};

} // namespace commsdsl2swig
