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
#include "commsdsl/parse/ParseStringField.h"
#include "commsdsl/gen/GenField.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class GenStringFieldImpl;
class COMMSDSL_API GenStringField : public GenField
{
    using Base = GenField;
public:

    GenStringField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent = nullptr);
    virtual ~GenStringField();

    GenField* externalPrefixField();
    const GenField* externalPrefixField() const;

    GenField* memberPrefixField();
    const GenField* memberPrefixField() const;

    commsdsl::parse::ParseStringField stringDslObj() const;

protected:    
    virtual bool prepareImpl() override;
    virtual void setReferencedImpl() override;

private:
    std::unique_ptr<GenStringFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
