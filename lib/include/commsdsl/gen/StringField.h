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
#include "commsdsl/parse/StringField.h"
#include "commsdsl/gen/Field.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class StringFieldImpl;
class COMMSDSL_API StringField : public Field
{
    using Base = Field;
public:

    StringField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent = nullptr);
    virtual ~StringField();

    Field* externalPrefixField();
    const Field* externalPrefixField() const;

    Field* memberPrefixField();
    const Field* memberPrefixField() const;

    commsdsl::parse::StringField stringDslObj() const;

protected:    
    virtual bool prepareImpl() override;
    virtual void setReferencedImpl() override;

private:
    std::unique_ptr<StringFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
