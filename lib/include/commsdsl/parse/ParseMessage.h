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

#include "commsdsl/parse/ParseAlias.h"
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseOptCond.h"
#include "commsdsl/parse/ParseOverrideType.h"

namespace commsdsl
{

namespace parse
{

class ParseMessageImpl;
class COMMSDSL_API ParseMessage
{
public:
    enum class Sender
    {
        Both,
        Client,
        Server,
        NumOfValues
    };

    using FieldsList = std::vector<ParseField>;
    using AttributesMap = ParseField::AttributesMap;
    using ElementsList = ParseField::ElementsList;
    using PlatformsList = std::vector<std::string>;
    using AliasesList = std::vector<ParseAlias>;

    explicit ParseMessage(const ParseMessageImpl* impl);
    ParseMessage(const ParseMessage& other);
    ~ParseMessage();

    bool valid() const;
    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;
    std::uintmax_t id() const;
    unsigned order() const;
    std::size_t minLength() const;
    std::size_t maxLength() const;
    unsigned sinceVersion() const;
    unsigned deprecatedSince() const;
    bool isDeprecatedRemoved() const;
    FieldsList fields() const;
    AliasesList aliases() const;
    std::string externalRef(bool schemaRef = true) const;
    bool isCustomizable() const;
    bool isFailOnInvalid() const;
    Sender sender() const;
    ParseOverrideType readOverride() const;
    ParseOverrideType writeOverride() const;
    ParseOverrideType refreshOverride() const;
    ParseOverrideType lengthOverride() const;
    ParseOverrideType validOverride() const;
    ParseOverrideType nameOverride() const;    
    const std::string& copyCodeFrom() const;
    ParseOptCond construct() const;
    ParseOptCond readCond() const;
    ParseOptCond validCond() const;

    const AttributesMap& parseExtraAttributes() const;
    const ElementsList& parseExtraElements() const;
    const PlatformsList& platforms() const;

protected:
    const ParseMessageImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
