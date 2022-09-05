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

#include "commsdsl/gen/Schema.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigSchema final: public commsdsl::gen::Schema
{
    using Base = commsdsl::gen::Schema;

public:
    explicit SwigSchema(SwigGenerator& generator, commsdsl::parse::Schema dslObj, Elem* parent);
    virtual ~SwigSchema();

    bool swigHasAnyMessage() const;
    bool swigHasReferencedMsgId() const;    
};

} // namespace commsdsl2swig
