//
// Copyright 2021 - 2024 (C). Alex Robenko. All rights reserved.
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
#include "commsdsl/parse/OptionalField.h"
#include "commsdsl/gen/Field.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class OptionalFieldImpl;
class COMMSDSL_API OptionalField : public Field
{
    using Base = Field;
public:

    OptionalField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    virtual ~OptionalField();

    Field* externalField();
    const Field* externalField() const;

    Field* memberField();
    const Field* memberField() const;

protected:    
    virtual bool prepareImpl() override;
    virtual void setReferencedImpl() override;

    commsdsl::parse::OptionalField optionalDslObj() const;

private:
    std::unique_ptr<OptionalFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
