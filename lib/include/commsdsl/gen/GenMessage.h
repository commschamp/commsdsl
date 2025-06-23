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
#include "commsdsl/parse/ParseMessage.h"
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/gen/GenField.h"

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
    using Ptr = std::unique_ptr<GenMessage>;
    using FieldsList = GenField::FieldsList;

    explicit GenMessage(GenGenerator& generator, commsdsl::parse::ParseMessage dslObj, GenElem* parent = nullptr);
    virtual ~GenMessage();

    bool createAll();
    bool isPrepared() const;
    bool prepare();
    bool write() const;
    bool isReferenced() const;
    void setReferenced(bool value);

    commsdsl::parse::ParseMessage dslObj() const;

    const FieldsList& fields() const;

    GenGenerator& generator();
    const GenGenerator& generator() const;

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;

private:
    std::unique_ptr<GenMessageImpl> m_impl;
};

using MessagePtr = GenMessage::Ptr;

} // namespace gen

} // namespace commsdsl
