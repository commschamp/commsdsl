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
#include "commsdsl/parse/ParseRefField.h"
#include "commsdsl/gen/GenField.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class GenRefFieldImpl;
class COMMSDSL_API GenRefField : public GenField
{
    using Base = GenField;
public:

    GenRefField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent = nullptr);
    virtual ~GenRefField();

    GenField* referencedField();
    const GenField* referencedField() const;    

protected:    
    virtual bool prepareImpl() override;
    virtual void setReferencedImpl() override;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const override final;

    commsdsl::parse::ParseRefField refDslObj() const;

private:
    std::unique_ptr<GenRefFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
