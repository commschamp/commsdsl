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

#include "SwigField.h"

#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigInterface final: public commsdsl::gen::GenInterface
{
    using GenBase = commsdsl::gen::GenInterface;

public:
    using ParseInterface = commsdsl::parse::ParseInterface;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit SwigInterface(SwigGenerator& generator, ParseInterface parseObj, GenElem* parent);
    virtual ~SwigInterface();

    void swigAddCodeIncludes(GenStringsList& list) const; 
    void swigAddCode(GenStringsList& list) const; 
    void swigAddDef(GenStringsList& list) const;

    static const SwigInterface* swigCast(const commsdsl::gen::GenInterface* i)
    {
        return static_cast<const SwigInterface*>(i);
    }

protected:
    virtual bool genPrepareImpl() override;    
    virtual bool genWriteImpl() const override;

private:
    using SwigFieldsList = SwigField::SwigFieldsList;

    std::string swigFieldDeclsInternal() const;
    std::string swigClassDeclInternal() const;
    std::string swigFieldsAccDeclInternal() const;    
    std::string swigFieldsAccCodeInternal() const;
    std::string swigCommonCodeInternal() const;

    SwigFieldsList m_swigFields;        
};

} // namespace commsdsl2swig
