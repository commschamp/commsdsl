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

#include <string>
#include <vector>
#include <cstdint>

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseAlias.h"

namespace commsdsl
{

namespace parse
{

class ParseInterfaceImpl;
class COMMSDSL_API ParseInterface
{
public:
    using FieldsList = std::vector<ParseField>;
    using AttributesMap = ParseField::AttributesMap;
    using ElementsList = ParseField::ElementsList;
    using AliasesList = std::vector<ParseAlias>;

    explicit ParseInterface(const ParseInterfaceImpl* impl);
    ParseInterface(const ParseInterface& other);
    ~ParseInterface();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    FieldsList fields() const;
    AliasesList aliases() const;
    std::string externalRef(bool schemaRef = true) const;
    const std::string& copyCodeFrom() const;

    const AttributesMap& parseExtraAttributes() const;
    const ElementsList& parseExtraElements() const;

protected:
    const ParseInterfaceImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
