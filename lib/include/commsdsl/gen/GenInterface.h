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
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/parse/ParseInterface.h"

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
    using ParseInterface = commsdsl::parse::ParseInterface;

    using GenPtr = std::unique_ptr<GenInterface>;
    using GenFieldsList = GenField::GenFieldsList;

    explicit GenInterface(GenGenerator& generator, ParseInterface parseObj, GenElem* parent = nullptr);
    virtual ~GenInterface();

    bool genCreateAll();
    bool genPrepare();
    bool genWrite() const;

    const GenFieldsList& genFields() const;
    ParseInterface genParseObj() const;
    std::string genAdjustedExternalRef() const;
    const std::string& genAdjustedName() const;

    GenGenerator& genGenerator();
    const GenGenerator& genGenerator() const;

    bool genHasVersionField() const;

    bool genIsReferenced() const;
    void genSetReferenced(bool value = true);

    const GenNamespace* genParentNamespace() const;

protected:
    virtual GenType genElemTypeImpl() const override final;
    virtual bool genPrepareImpl();
    virtual bool genWriteImpl() const;

private:
    std::unique_ptr<GenInterfaceImpl> m_impl;
};

using GenInterfacePtr = GenInterface::GenPtr;

} // namespace gen

} // namespace commsdsl
