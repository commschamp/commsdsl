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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/IntField.h"
#include "commsdsl/gen/Field.h"

#include <memory>
#include <vector>
#include <utility>

namespace commsdsl
{

namespace gen
{

class IntFieldImpl;
class COMMSDSL_API IntField : public Field
{
    using Base = Field;
public:

    using SpecialsListElem = std::pair<std::string, commsdsl::parse::IntField::SpecialValueInfo>;
    using SpecialsList = std::vector<SpecialsListElem>;

    IntField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    virtual ~IntField();

    static bool isUnsignedType(commsdsl::parse::IntField::Type value);
    bool isUnsignedType() const;

    const SpecialsList& specialsSortedByValue() const;

protected:    
    virtual bool prepareImpl() override;

    commsdsl::parse::IntField intDslObj() const;

private:
    std::unique_ptr<IntFieldImpl> m_impl;    
};

} // namespace gen

} // namespace commsdsl
