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
#include "commsdsl/parse/ParseListField.h"

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
    using ParseField = commsdsl::parse::ParseField;
    using ParseListField = commsdsl::parse::ParseListField;

    GenListField(GenGenerator& generator, ParseField parseObj, GenElem* parent = nullptr);
    virtual ~GenListField();

    GenField* genExternalElementField();
    const GenField* genExternalElementField() const;

    GenField* genMemberElementField();
    const GenField* genMemberElementField() const;

    GenField* genExternalCountPrefixField();
    const GenField* genExternalCountPrefixField() const;

    GenField* genMemberCountPrefixField();
    const GenField* genMemberCountPrefixField() const;

    GenField* genExternalLengthPrefixField();
    const GenField* genExternalLengthPrefixField() const;

    GenField* genMemberLengthPrefixField();
    const GenField* genMemberLengthPrefixField() const;

    GenField* genExternalElemLengthPrefixField();
    const GenField* genExternalElemLengthPrefixField() const;

    GenField* genMemberElemLengthPrefixField();
    const GenField* genMemberElemLengthPrefixField() const;

    GenField* genExternalTermSuffixField();
    const GenField* genExternalTermSuffixField() const;

    GenField* genMemberTermSuffixField();
    const GenField* genMemberTermSuffixField() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual void genSetReferencedImpl() override;

    ParseListField genListFieldParseObj() const;

private:
    std::unique_ptr<GenListFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
