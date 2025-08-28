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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseNamespace.h"

#include <map>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseSchemaImpl;
class COMMSDSL_API ParseSchema
{
public:
    using ParseAttributesMap = std::multimap<std::string, std::string>;
    using ParseElementsList = std::vector<std::string>;
    using ParseNamespacesList = std::vector<ParseNamespace>;
    using ParseMessagesList = ParseNamespace::ParseMessagesList;
    using ParsePlatformsList = ParseMessage::ParsePlatformsList;

    explicit ParseSchema(const ParseSchemaImpl* impl);

    bool parseValid() const;

    const std::string& parseName() const;

    const std::string& parseDisplayName() const;

    const std::string& parseDescription() const;

    unsigned parseId() const;

    unsigned parseVersion() const;

    unsigned parseDslVersion() const;

    ParseEndian parseEndian() const;

    bool parseNonUniqueMsgIdAllowed() const;

    const ParseAttributesMap& parseExtraAttributes() const;

    const ParseElementsList& parseExtraElements() const;

    ParseNamespacesList parseNamespaces() const;

    const ParsePlatformsList& parsePlatforms() const;

    ParseMessagesList parseAllMessages() const;

    std::string parseExternalRef() const;

private:
    const ParseSchemaImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
