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
#include "commsdsl/parse/ParseListField.h"
#include "commsdsl/gen/GenField.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class GenListFieldImpl;
class COMMSDSL_API GenListField : public GenField
{
    using Base = GenField;
public:

    GenListField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent = nullptr);
    virtual ~GenListField();

    GenField* externalElementField();
    const GenField* externalElementField() const;

    GenField* memberElementField();
    const GenField* memberElementField() const;

    GenField* externalCountPrefixField();
    const GenField* externalCountPrefixField() const;

    GenField* memberCountPrefixField();
    const GenField* memberCountPrefixField() const;

    GenField* externalLengthPrefixField();
    const GenField* externalLengthPrefixField() const;

    GenField* memberLengthPrefixField();
    const GenField* memberLengthPrefixField() const;

    GenField* externalElemLengthPrefixField();
    const GenField* externalElemLengthPrefixField() const;

    GenField* memberElemLengthPrefixField();
    const GenField* memberElemLengthPrefixField() const;

    GenField* externalTermSuffixField();
    const GenField* externalTermSuffixField() const;

    GenField* memberTermSuffixField();
    const GenField* memberTermSuffixField() const;


protected:    
    virtual bool prepareImpl() override;
    virtual void setReferencedImpl() override;

    commsdsl::parse::ParseListField listDslObj() const;

private:
    std::unique_ptr<GenListFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
