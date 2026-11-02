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
#include "commsdsl/parse/ParseOverrideType.h"

#include <map>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseFieldImpl;
class COMMSDSL_API ParseField
{
public:

    using ParseAttributesMap = std::multimap<std::string, std::string>;
    using ParseElementsList = std::vector<std::string>;

    enum class ParseKind
    {
        Int,
        Enum,
        Set,
        Float,
        Bitfield,
        Bundle,
        String,
        Data,
        List,
        Ref,
        Optional,
        Variant,
        NumOfValues
    };

    enum class ParseSemanticType
    {
        None,
        Version,
        MessageId,
        Length,
        NumOfValues
    };

    explicit ParseField(const ParseFieldImpl* impl);
    ParseField(const ParseField& other);
    ~ParseField();

    [[nodiscard]] bool parseValid() const;
    [[nodiscard]] const std::string& parseName() const;
    [[nodiscard]] const std::string& parseDisplayName() const;
    [[nodiscard]] const std::string& parseDescription() const;
    [[nodiscard]] ParseKind parseKind() const;
    [[nodiscard]] ParseSemanticType parseSemanticType() const;
    [[nodiscard]] std::size_t parseMinLength() const;
    [[nodiscard]] std::size_t parseMaxLength() const;
    [[nodiscard]] static std::size_t parseMaxPossibleLength();
    [[nodiscard]] std::size_t parseBitLength() const;
    [[nodiscard]] unsigned parseSinceVersion() const;
    [[nodiscard]] unsigned parseDeprecatedSince() const;
    [[nodiscard]] bool parseIsDeprecatedRemoved() const;
    [[nodiscard]] std::string parseExternalRef(bool schemaRef = true) const;
    [[nodiscard]] std::string parseInnerRef(bool schemaRef = true) const;
    [[nodiscard]] bool parseIsPseudo() const;
    [[nodiscard]] bool parseIsFixedValue() const;
    [[nodiscard]] bool parseIsCustomizable() const;
    [[nodiscard]] bool parseIsFailOnInvalid() const;
    [[nodiscard]] bool parseIsForceGen() const;
    [[nodiscard]] std::string parseSchemaPos() const;
    [[nodiscard]] ParseOverrideType parseValueOverride() const;
    [[nodiscard]] ParseOverrideType parseReadOverride() const;
    [[nodiscard]] ParseOverrideType parseWriteOverride() const;
    [[nodiscard]] ParseOverrideType parseRefreshOverride() const;
    [[nodiscard]] ParseOverrideType parseLengthOverride() const;
    [[nodiscard]] ParseOverrideType parseValidOverride() const;
    [[nodiscard]] ParseOverrideType parseNameOverride() const;
    [[nodiscard]] const std::string& parseCopyCodeFrom() const;
    [[nodiscard]] bool parseIsValidInnerRef(const std::string& refStr) const;

    [[nodiscard]] const ParseAttributesMap& parseExtraAttributes() const;
    [[nodiscard]] const ParseElementsList& parseExtraElements() const;

protected:
    const ParseFieldImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
