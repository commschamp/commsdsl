//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseProtocol.h"

#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseStringFieldImpl;
class COMMSDSL_API ParseStringField : public ParseField
{
    using Base = ParseField;
public:

    struct ValidValueInfo
    {
        std::string m_value;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::notYetDeprecated();
    };

    using ValidValuesList = std::vector<ValidValueInfo>;

    explicit ParseStringField(const ParseStringFieldImpl* impl);
    explicit ParseStringField(ParseField field);

    const std::string& defaultValue() const;
    const std::string& encodingStr() const;
    std::size_t fixedLength() const;
    bool hasLengthPrefixField() const;
    ParseField lengthPrefixField() const;
    bool hasZeroTermSuffix() const;
    const std::string& detachedPrefixFieldName() const;
    const ValidValuesList& validValues() const;
};

} // namespace parse

} // namespace commsdsl
