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
#include "commsdsl/parse/Message.h"
#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Field.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class MessageImpl;
class COMMSDSL_API Message : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Message>;
    using FieldsList = Field::FieldsList;

    explicit Message(Generator& generator, commsdsl::parse::Message dslObj, Elem* parent = nullptr);
    virtual ~Message();

    bool createAll();
    bool isPrepared() const;
    bool prepare();
    bool write() const;
    bool isReferenced() const;
    void setReferenced(bool value);

    commsdsl::parse::Message dslObj() const;

    const FieldsList& fields() const;

    Generator& generator();
    const Generator& generator() const;

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;

private:
    std::unique_ptr<MessageImpl> m_impl;
};

using MessagePtr = Message::Ptr;

} // namespace gen

} // namespace commsdsl
