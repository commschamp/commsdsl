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

#include <map>
#include <string>
#include <vector>

#include "commsdsl/CommsdslApi.h"
#include "ParseOverrideType.h"

namespace commsdsl
{

namespace parse
{
    
class ParseFieldImpl;
class COMMSDSL_API ParseField
{
public:

    using AttributesMap = std::multimap<std::string, std::string>;
    using ElementsList = std::vector<std::string>;

    enum class Kind
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

    enum class SemanticType
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

    bool valid() const;
    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;
    Kind kind() const;
    SemanticType semanticType() const;
    std::size_t minLength() const;
    std::size_t maxLength() const;
    std::size_t bitLength() const;
    unsigned sinceVersion() const;
    unsigned deprecatedSince() const;
    bool isDeprecatedRemoved() const;
    std::string externalRef(bool schemaRef = true) const;
    bool isPseudo() const;
    bool isFixedValue() const;
    bool isCustomizable() const;
    bool isFailOnInvalid() const;
    bool isForceGen() const;
    std::string schemaPos() const;
    ParseOverrideType valueOverride() const;
    ParseOverrideType readOverride() const;
    ParseOverrideType writeOverride() const;
    ParseOverrideType refreshOverride() const;
    ParseOverrideType lengthOverride() const;
    ParseOverrideType validOverride() const;
    ParseOverrideType nameOverride() const;
    const std::string& copyCodeFrom() const;
    bool isValidInnerRef(const std::string& refStr) const;

    const AttributesMap& parseExtraAttributes() const;
    const ElementsList& parseExtraElements() const;

protected:
    const ParseFieldImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
