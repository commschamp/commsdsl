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

#include "commsdsl/gen/GenSchema.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigSchema final: public commsdsl::gen::GenSchema
{
    using Base = commsdsl::gen::GenSchema;

public:
    using StringsList = commsdsl::gen::util::StringsList;

    explicit SwigSchema(SwigGenerator& generator, commsdsl::parse::ParseSchema dslObj, commsdsl::gen::GenElem* parent);
    virtual ~SwigSchema();

    bool swigHasAnyMessage() const;
    bool swigHasReferencedMsgId() const;   

    void swigAddCodeIncludes(StringsList& list) const; 
    void swigAddCode(StringsList& list) const; 
    void swigAddDef(StringsList& list) const;

    static const SwigSchema* cast(const commsdsl::gen::GenSchema* schema)
    {
        return static_cast<const SwigSchema*>(schema);
    }
};

} // namespace commsdsl2swig
