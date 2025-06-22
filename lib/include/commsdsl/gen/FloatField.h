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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseFloatField.h"
#include "commsdsl/gen/Field.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class FloatFieldImpl;
class COMMSDSL_API FloatField : public Field
{
    using Base = Field;
public:
    using SpecialsListElem = std::pair<std::string, commsdsl::parse::ParseFloatField::SpecialValueInfo>;
    using SpecialsList = std::vector<SpecialsListElem>;

    FloatField(Generator& generator, commsdsl::parse::ParseField dslObj, Elem* parent = nullptr);
    virtual ~FloatField();

    const SpecialsList& specialsSortedByValue() const;

protected:    
    virtual bool prepareImpl() override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override final;
    
    commsdsl::parse::ParseFloatField floatDslObj() const;

private:
    std::unique_ptr<FloatFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
