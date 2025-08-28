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
#include "commsdsl/gen/GenField.h"
#include "commsdsl/parse/ParseDataField.h"

#include <memory>

namespace commsdsl
{

namespace gen
{

class GenDataFieldImpl;
class COMMSDSL_API GenDataField : public GenField
{
    using Base = GenField;
    
public:
    using ParseField = commsdsl::parse::ParseField;
    using ParseDataField = commsdsl::parse::ParseDataField;

    GenDataField(GenGenerator& generator, ParseField parseObj, GenElem* parent = nullptr);
    virtual ~GenDataField();

    GenField* genExternalPrefixField();
    const GenField* genExternalPrefixField() const;

    GenField* genMemberPrefixField();
    const GenField* genMemberPrefixField() const;    

protected:    
    virtual bool genPrepareImpl() override;
    virtual void genSetReferencedImpl() override;
    
    ParseDataField genDataFieldParseObj() const;

private:
    std::unique_ptr<GenDataFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
