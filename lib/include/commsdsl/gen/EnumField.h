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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/EnumField.h"
#include "commsdsl/gen/Field.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class EnumFieldImpl;
class COMMSDSL_API EnumField : public Field
{
    using Base = Field;
public:

    EnumField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    virtual ~EnumField();

    bool isUnsignedUnderlyingType() const;
    unsigned hexWidth() const;
    std::string valueName(std::intmax_t value) const;

    commsdsl::parse::EnumField enumDslObj() const;

    using RevValueInfo = std::pair<std::intmax_t, const std::string*>;
    using SortedRevValues = std::vector<RevValueInfo>;
    const SortedRevValues& sortedRevValues() const;

    std::string valueToString(std::intmax_t val) const;

    bool hasValuesLimit() const;

protected:    
    virtual bool prepareImpl() override;

private:
    std::unique_ptr<EnumFieldImpl> m_impl;        
};

} // namespace gen

} // namespace commsdsl
