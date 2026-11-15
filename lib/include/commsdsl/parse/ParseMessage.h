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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseAlias.h"
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseOptCond.h"
#include "commsdsl/parse/ParseOverrideType.h"

#include <cstdint>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseMessageImpl;
class COMMSDSL_API ParseMessage
{
public:
    enum class ParseSender
    {
        Both,
        Client,
        Server,
        NumOfValues
    };

    using ParseFieldsList = std::vector<ParseField>;
    using ParseAttributesMap = ParseField::ParseAttributesMap;
    using ParseElementsList = ParseField::ParseElementsList;
    using ParsePlatformsList = std::vector<std::string>;
    using ParseAliasesList = std::vector<ParseAlias>;

    explicit ParseMessage(const ParseMessageImpl* impl);
    ParseMessage(const ParseMessage& other);
    ~ParseMessage();

    [[nodiscard]] bool parseValid() const;
    [[nodiscard]] const std::string& parseName() const;
    [[nodiscard]] const std::string& parseDisplayName() const;
    [[nodiscard]] const std::string& parseDescription() const;
    [[nodiscard]] std::uintmax_t parseId() const;
    [[nodiscard]] unsigned parseOrder() const;
    [[nodiscard]] std::size_t parseMinLength() const;
    [[nodiscard]] std::size_t parseMaxLength() const;
    [[nodiscard]] unsigned parseSinceVersion() const;
    [[nodiscard]] unsigned parseDeprecatedSince() const;
    [[nodiscard]] bool parseIsDeprecatedRemoved() const;
    [[nodiscard]] ParseFieldsList parseFields() const;
    [[nodiscard]] ParseAliasesList parseAliases() const;
    [[nodiscard]] std::string parseExternalRef(bool schemaRef = true) const;
    [[nodiscard]] bool parseIsCustomizable() const;
    [[nodiscard]] bool parseIsFailOnInvalid() const;
    [[nodiscard]] ParseSender parseSender() const;
    [[nodiscard]] ParseOverrideType parseReadOverride() const;
    [[nodiscard]] ParseOverrideType parseWriteOverride() const;
    [[nodiscard]] ParseOverrideType parseRefreshOverride() const;
    [[nodiscard]] ParseOverrideType parseLengthOverride() const;
    [[nodiscard]] ParseOverrideType parseValidOverride() const;
    [[nodiscard]] ParseOverrideType parseNameOverride() const;
    [[nodiscard]] const std::string& parseCopyCodeFrom() const;
    [[nodiscard]] ParseOptCond parseConstruct() const;
    [[nodiscard]] ParseOptCond parseReadCond() const;
    [[nodiscard]] ParseOptCond parseValidCond() const;
    [[nodiscard]] unsigned parseReadAlign() const;
    [[nodiscard]] unsigned parseWritePadAlign() const;
    [[nodiscard]] bool parseWritePadInLength() const;

    [[nodiscard]] const ParseAttributesMap& parseExtraAttributes() const;
    [[nodiscard]] const ParseElementsList& parseExtraElements() const;
    [[nodiscard]] const ParsePlatformsList& parsePlatforms() const;

protected:
    const ParseMessageImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
