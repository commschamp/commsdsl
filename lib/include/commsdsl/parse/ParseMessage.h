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

    bool parseValid() const;
    const std::string& parseName() const;
    const std::string& parseDisplayName() const;
    const std::string& parseDescription() const;
    std::uintmax_t parseId() const;
    unsigned parseOrder() const;
    std::size_t parseMinLength() const;
    std::size_t parseMaxLength() const;
    unsigned parseSinceVersion() const;
    unsigned parseDeprecatedSince() const;
    bool parseIsDeprecatedRemoved() const;
    FieldsList parseFields() const;
    AliasesList parseAliases() const;
    std::string parseExternalRef(bool schemaRef = true) const;
    bool parseIsCustomizable() const;
    bool parseIsFailOnInvalid() const;
    Sender parseSender() const;
    ParseOverrideType parseReadOverride() const;
    ParseOverrideType parseWriteOverride() const;
    ParseOverrideType parseRefreshOverride() const;
    ParseOverrideType parseLengthOverride() const;
    ParseOverrideType parseValidOverride() const;
    ParseOverrideType parseNameOverride() const;    
    const std::string& parseCopyCodeFrom() const;
    ParseOptCond parseConstruct() const;
    ParseOptCond parseReadCond() const;
    ParseOptCond parseValidCond() const;

    const AttributesMap& parseExtraAttributes() const;
    const ElementsList& parseExtraElements() const;
    const PlatformsList& parsePlatforms() const;

protected:
    const ParseMessageImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
