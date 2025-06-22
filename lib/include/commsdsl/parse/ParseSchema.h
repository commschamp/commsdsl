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
#include <map>
#include <vector>

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseNamespace.h"

namespace commsdsl
{

namespace parse
{

class ParseSchemaImpl;
class COMMSDSL_API ParseSchema
{
public:
    using AttributesMap = std::multimap<std::string, std::string>;
    using ElementsList = std::vector<std::string>;
    using NamespacesList = std::vector<ParseNamespace>;
    using MessagesList = ParseNamespace::MessagesList;
    using PlatformsList = ParseMessage::PlatformsList;

    explicit ParseSchema(const ParseSchemaImpl* impl);

    bool valid() const;

    const std::string& name() const;

    const std::string& description() const;

    unsigned id() const;

    unsigned version() const;

    unsigned dslVersion() const;

    ParseEndian endian() const;

    bool nonUniqueMsgIdAllowed() const;

    const AttributesMap& extraAttributes() const;

    const ElementsList& extraElements() const;

    NamespacesList namespaces() const;

    const PlatformsList& platforms() const;

    MessagesList allMessages() const;

    std::string externalRef() const;

private:
    const ParseSchemaImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
