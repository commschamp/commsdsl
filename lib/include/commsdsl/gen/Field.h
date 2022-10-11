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

class Generator;
class FieldImpl;
class COMMSDSL_API Field : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Field>;
    using FieldsList = std::vector<Ptr>;

    virtual ~Field();

    static Ptr create(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent = nullptr);    

    bool isPrepared() const;
    bool prepare();
    bool write() const;

    const commsdsl::parse::Field& dslObj() const;    

    Generator& generator();
    const Generator& generator() const;

    bool isReferenced() const;
    void setReferenced();

    static void setFieldReferencedIfExists(Field* field);

protected:    
    Field(Generator& generator, const commsdsl::parse::Field& dslObj, Elem* parent = nullptr);

    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;
    virtual void setReferencedImpl();


private:
    std::unique_ptr<FieldImpl> m_impl;
};

using FieldPtr = Field::Ptr;

} // namespace gen

} // namespace commsdsl
