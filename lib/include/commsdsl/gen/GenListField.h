//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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

    [[nodiscard]] GenField* genExternalElementField();
    [[nodiscard]] const GenField* genExternalElementField() const;

    [[nodiscard]] GenField* genMemberElementField();
    [[nodiscard]] const GenField* genMemberElementField() const;

    [[nodiscard]] GenField* genExternalCountPrefixField();
    [[nodiscard]] const GenField* genExternalCountPrefixField() const;

    [[nodiscard]] GenField* genMemberCountPrefixField();
    [[nodiscard]] const GenField* genMemberCountPrefixField() const;

    [[nodiscard]] GenField* genExternalLengthPrefixField();
    [[nodiscard]] const GenField* genExternalLengthPrefixField() const;

    [[nodiscard]] GenField* genMemberLengthPrefixField();
    [[nodiscard]] const GenField* genMemberLengthPrefixField() const;

    [[nodiscard]] GenField* genExternalElemLengthPrefixField();
    [[nodiscard]] const GenField* genExternalElemLengthPrefixField() const;

    [[nodiscard]] GenField* genMemberElemLengthPrefixField();
    [[nodiscard]] const GenField* genMemberElemLengthPrefixField() const;

    [[nodiscard]] GenField* genExternalTermSuffixField();
    [[nodiscard]] const GenField* genExternalTermSuffixField() const;

    [[nodiscard]] GenField* genMemberTermSuffixField();
    [[nodiscard]] const GenField* genMemberTermSuffixField() const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    virtual void genSetReferencedImpl(bool referenced) override;

    [[nodiscard]] ParseListField genListFieldParseObj() const;

private:
    std::unique_ptr<GenListFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
