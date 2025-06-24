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

#include "commsdsl/parse/ParseAlias.h"
#include "commsdsl/parse/ParseOptCond.h"
#include "commsdsl/parse/ParseField.h"

namespace commsdsl
{

namespace parse
{

class ParseBundleFieldImpl;
class COMMSDSL_API ParseBundleField : public ParseField
{
    using Base = ParseField;
public:

    using Members = std::vector<ParseField>;
    using Aliases = std::vector<ParseAlias>;

    explicit ParseBundleField(const ParseBundleFieldImpl* impl);
    explicit ParseBundleField(ParseField field);

    Members parseMembers() const;
    Aliases parseAliases() const;
    ParseOptCond parseValidCond() const;    
};

} // namespace parse

} // namespace commsdsl
