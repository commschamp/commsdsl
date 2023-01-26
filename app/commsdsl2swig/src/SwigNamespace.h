//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Namespace.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigNamespace final: public commsdsl::gen::Namespace
{
    using Base = commsdsl::gen::Namespace;

public:
    using StringsList = commsdsl::gen::util::StringsList;

    explicit SwigNamespace(SwigGenerator& generator, commsdsl::parse::Namespace dslObj, Elem* parent);
    virtual ~SwigNamespace();

    void swigAddCodeIncludes(StringsList& list) const; 
    void swigAddCode(StringsList& list) const; 

    void swigAddDef(StringsList& list) const;

    static const SwigNamespace* cast(const commsdsl::gen::Namespace* ns)
    {
        return static_cast<const SwigNamespace*>(ns);
    }

protected:
    virtual bool prepareImpl() override;    

private:
    using SwigFieldsList = SwigField::SwigFieldsList;

    SwigFieldsList m_swigFields;        
};

} // namespace commsdsl2swig
