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

#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigFrame final: public commsdsl::gen::Frame
{
    using Base = commsdsl::gen::Frame;

public:
    using StringsList = commsdsl::gen::util::StringsList;
    using SwigLayersList = SwigLayer::SwigLayersList;

    explicit SwigFrame(SwigGenerator& generator, commsdsl::parse::ParseFrame dslObj, Elem* parent);
    virtual ~SwigFrame();

    void swigAddCodeIncludes(StringsList& list) const;
    void swigAddCode(StringsList& list) const;
    void swigAddDef(StringsList& list) const;

    static const SwigFrame* cast(const commsdsl::gen::Frame* i)
    {
        return static_cast<const SwigFrame*>(i);
    }        

    const SwigLayersList& swigLayers() const
    {
        return m_swigLayers;
    }

protected:    
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

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
