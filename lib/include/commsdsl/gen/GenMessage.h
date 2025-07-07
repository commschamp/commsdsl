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
#include "commsdsl/parse/ParseMessage.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenMessageImpl;
class Namespace;

class COMMSDSL_API GenMessage : public GenElem
{
    using Base = GenElem;

public:
    using ParseMessage = commsdsl::parse::ParseMessage;
    
    using GenPtr = std::unique_ptr<GenMessage>;
    using GenFieldsList = GenField::GenFieldsList;

    explicit GenMessage(GenGenerator& generator, ParseMessage dslObj, GenElem* parent = nullptr);
    virtual ~GenMessage();

    bool genCreateAll();
    bool genIsPrepared() const;
    bool genPrepare();
    bool genWrite() const;
    bool genIsReferenced() const;
    void genSetReferenced(bool value);

    ParseMessage genParseObj() const;

    const GenFieldsList& genFields() const;

    GenGenerator& genGenerator();
    const GenGenerator& genGenerator() const;

protected:    
    virtual Type genElemTypeImpl() const override final;
    virtual bool genPrepareImpl();
    virtual bool genWriteImpl() const;

private:
    std::unique_ptr<GenMessageImpl> m_impl;
};

using GenMessagePtr = GenMessage::GenPtr;

} // namespace gen

} // namespace commsdsl
