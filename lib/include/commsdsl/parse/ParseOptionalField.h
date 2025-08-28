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
#include "commsdsl/parse/ParseOptCond.h"

namespace commsdsl
{

namespace parse
{

class ParseOptionalFieldImpl;
class COMMSDSL_API ParseOptionalField : public ParseField
{
    using Base = ParseField;
public:

    enum class ParseMode
    {
        Tentative,
        Exists,
        Missing,
        NumOfValues
    };

    explicit ParseOptionalField(const ParseOptionalFieldImpl* impl);
    explicit ParseOptionalField(ParseField field);

    ParseMode parseDefaultMode() const;
    ParseField parseField() const;
    ParseOptCond parseCond() const;
    bool parseMissingOnReadFail() const;
    bool parseMissingOnInvalid() const;
};

} // namespace parse

} // namespace commsdsl
