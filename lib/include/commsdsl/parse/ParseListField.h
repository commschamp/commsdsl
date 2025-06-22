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

#include "ParseField.h"

namespace commsdsl
{

namespace parse
{

class ParseListFieldImpl;
class COMMSDSL_API ParseListField : public ParseField
{
    using Base = ParseField;
public:

    explicit ParseListField(const ParseListFieldImpl* impl);
    explicit ParseListField(ParseField field);

    ParseField elementField() const;
    std::size_t fixedCount() const;
    bool hasCountPrefixField() const;
    ParseField countPrefixField() const;
    const std::string& detachedCountPrefixFieldName() const;
    bool hasLengthPrefixField() const;
    ParseField lengthPrefixField() const;
    const std::string& detachedLengthPrefixFieldName() const;
    bool hasElemLengthPrefixField() const;
    ParseField elemLengthPrefixField() const;
    const std::string& detachedElemLengthPrefixFieldName() const;
    bool elemFixedLength() const;
    bool hasTermSuffixField() const;
    ParseField termSuffixField() const;
    const std::string& detachedTermSuffixFieldName() const;
};

} // namespace parse

} // namespace commsdsl
