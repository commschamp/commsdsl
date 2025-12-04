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
#include "commsdsl/parse/ParseEnumField.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenEnumFieldImpl;
class COMMSDSL_API GenEnumField : public GenField
{
    using Base = GenField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using ParseEnumField = commsdsl::parse::ParseEnumField;

    using GenRevValueInfo = std::pair<std::intmax_t, const std::string*>;
    using GenSortedRevValues = std::vector<GenRevValueInfo>;

    GenEnumField(GenGenerator& generator, ParseField dslObj, GenElem* parent = nullptr);
    virtual ~GenEnumField();

    bool genIsUnsignedUnderlyingType() const;
    unsigned genHexWidth() const;
    std::string genValueName(std::intmax_t value) const;
    std::string genAdjustName(const std::string& val) const;

    ParseEnumField genEnumFieldParseObj() const;

    const GenSortedRevValues& genSortedRevValues() const;

    std::string genValueToString(std::intmax_t val) const;

    bool genHasValuesLimit() const;

    std::string genFirstValueStr() const;
    std::string genLastValueStr() const;
    std::string genValuesLimitStr() const;

    bool genIsUnsignedType() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual GenFieldRefInfo genProcessInnerRefImpl(const std::string& refStr) const override final;

private:
    std::unique_ptr<GenEnumFieldImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
