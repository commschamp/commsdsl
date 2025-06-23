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
#include "commsdsl/parse/ParseInterface.h"
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenInterfaceImpl;
class GenNamespace;

class COMMSDSL_API GenInterface : public GenElem
{
    using Base = GenElem;
public:
    using Ptr = std::unique_ptr<GenInterface>;
    using FieldsList = GenField::FieldsList;

    explicit GenInterface(GenGenerator& generator, commsdsl::parse::ParseInterface dslObj, GenElem* parent = nullptr);
    virtual ~GenInterface();

    bool createAll();
    bool prepare();
    bool write() const;

    const FieldsList& fields() const;
    commsdsl::parse::ParseInterface dslObj() const;
    std::string adjustedExternalRef() const;
    const std::string& adjustedName() const;

    GenGenerator& generator();
    const GenGenerator& generator() const;

    bool hasVersionField() const;

    bool isReferenced() const;
    void setReferenced(bool value = true);

    const GenNamespace* parentNamespace() const;

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;

private:
    std::unique_ptr<GenInterfaceImpl> m_impl;
};

using InterfacePtr = GenInterface::Ptr;

} // namespace gen

} // namespace commsdsl
