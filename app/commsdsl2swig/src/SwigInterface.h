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

#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigInterface final: public commsdsl::gen::Interface
{
    using Base = commsdsl::gen::Interface;

public:
    using StringsList = commsdsl::gen::util::StringsList;
    
    explicit SwigInterface(SwigGenerator& generator, commsdsl::parse::Interface dslObj, Elem* parent);
    virtual ~SwigInterface();

    void swigAddCodeIncludes(StringsList& list) const; 
    void swigAddDef(StringsList& list) const;

    static const SwigInterface* cast(const commsdsl::gen::Interface* i)
    {
        return static_cast<const SwigInterface*>(i);
    }

protected:
    virtual bool prepareImpl() override;    
    virtual bool writeImpl() const override;

private:
    using SwigFieldsList = SwigField::SwigFieldsList;

    std::string swigFieldDeclsInternal() const;
    std::string swigClassDeclInternal() const;
    std::string swigFieldsAccessDeclInternal() const;    

    SwigFieldsList m_swigFields;        
};

} // namespace commsdsl2swig
