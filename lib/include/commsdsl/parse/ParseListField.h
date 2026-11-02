//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
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

    [[nodiscard]] ParseField parseElementField() const;
    [[nodiscard]] std::size_t parseFixedCount() const;
    [[nodiscard]] bool parseHasCountPrefixField() const;
    [[nodiscard]] ParseField parseCountPrefixField() const;
    [[nodiscard]] const std::string& parseDetachedCountPrefixFieldName() const;
    [[nodiscard]] bool parseHasLengthPrefixField() const;
    [[nodiscard]] ParseField parseLengthPrefixField() const;
    [[nodiscard]] const std::string& parseDetachedLengthPrefixFieldName() const;
    [[nodiscard]] bool parseHasElemLengthPrefixField() const;
    [[nodiscard]] ParseField parseElemLengthPrefixField() const;
    [[nodiscard]] const std::string& parseDetachedElemLengthPrefixFieldName() const;
    [[nodiscard]] bool parseElemFixedLength() const;
    [[nodiscard]] bool parseHasTermSuffixField() const;
    [[nodiscard]] ParseField parseTermSuffixField() const;
    [[nodiscard]] const std::string& parseDetachedTermSuffixFieldName() const;
};

} // namespace parse

} // namespace commsdsl
