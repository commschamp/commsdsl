//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include <map>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseAliasImpl;
class COMMSDSL_API ParseAlias
{
public:
    using ParseAttributesMap = std::multimap<std::string, std::string>;
    using ParseElementsList = std::vector<std::string>;

    explicit ParseAlias(const ParseAliasImpl* impl);
    ParseAlias(const ParseAlias& other);
    ~ParseAlias();

    const std::string& parseName() const;
    const std::string& parseDescription() const;
    const std::string& parseFieldName() const;

    const ParseAttributesMap& parseExtraAttributes() const;
    const ParseElementsList& parseExtraElements() const;

protected:
    const ParseAliasImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
