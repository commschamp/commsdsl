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

#include <cstddef>
#include <string>

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

    ParseField parseElementField() const;
    std::size_t parseFixedCount() const;
    bool parseHasCountPrefixField() const;
    ParseField parseCountPrefixField() const;
    const std::string& parseDetachedCountPrefixFieldName() const;
    bool parseHasLengthPrefixField() const;
    ParseField parseLengthPrefixField() const;
    const std::string& parseDetachedLengthPrefixFieldName() const;
    bool parseHasElemLengthPrefixField() const;
    ParseField parseElemLengthPrefixField() const;
    const std::string& parseDetachedElemLengthPrefixFieldName() const;
    bool parseElemFixedLength() const;
    bool parseHasTermSuffixField() const;
    ParseField parseTermSuffixField() const;
    const std::string& parseDetachedTermSuffixFieldName() const;
};

} // namespace parse

} // namespace commsdsl
