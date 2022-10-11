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
#include "commsdsl/parse/Interface.h"
#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Field.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class InterfaceImpl;
class COMMSDSL_API Interface : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Interface>;
    using FieldsList = Field::FieldsList;

    explicit Interface(Generator& generator, commsdsl::parse::Interface dslObj, Elem* parent = nullptr);
    virtual ~Interface();

    bool createAll();
    bool prepare();
    bool write() const;

    const FieldsList& fields() const;
    commsdsl::parse::Interface dslObj() const;

    Generator& generator();
    const Generator& generator() const;

    bool hasVersionField() const;

    bool isReferenced() const;
    void setReferenced(bool value = true);

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;

private:
    std::unique_ptr<InterfaceImpl> m_impl;
};

using InterfacePtr = Interface::Ptr;

} // namespace gen

} // namespace commsdsl
