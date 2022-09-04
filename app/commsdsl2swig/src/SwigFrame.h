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

#include "SwigField.h"

#include "commsdsl/gen/Frame.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigFrame final: public commsdsl::gen::Frame
{
    using Base = commsdsl::gen::Frame;

public:
    explicit SwigFrame(SwigGenerator& generator, commsdsl::parse::Frame dslObj, Elem* parent);
    virtual ~SwigFrame();

protected:    
    virtual bool writeImpl() const override;    

private:
    std::string swigLayerDeclsInternal() const;
    std::string swigHandlerDeclInternal() const;
    std::string swigClassDeclInternal() const;
    std::string swigLayersAccessInternal() const;
};

} // namespace commsdsl2swig
