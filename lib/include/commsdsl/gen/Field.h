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
#include "commsdsl/parse/Field.h"
#include "commsdsl/gen/Elem.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class COMMSDSL_API Field : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Field>;
    using FieldsList = std::vector<Ptr>;

    virtual ~Field();

    static Ptr create(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent = nullptr);    

    bool prepare();
    std::string genCode();
    bool write();

protected:    
    Field(Generator& generator, const commsdsl::parse::Field& dslObj, Elem* parent = nullptr);

    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual std::string genCodeImpl();
    virtual bool writeImpl();

    Generator& generator();
    const commsdsl::parse::Field& dslObj() const;

private:
    Generator& m_generator;
    commsdsl::parse::Field m_dslObj;
};

using FieldPtr = Field::Ptr;

} // namespace gen

} // namespace commsdsl
