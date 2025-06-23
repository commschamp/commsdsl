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

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseMessage.h"
#include "commsdsl/parse/ParseInterface.h"
#include "commsdsl/parse/ParseFrame.h"

namespace commsdsl
{

namespace parse
{

class ParseNamespaceImpl;
class COMMSDSL_API ParseNamespace
{
public:
    using NamespacesList = std::vector<ParseNamespace>;
    using FieldsList = std::vector<ParseField>;
    using MessagesList = std::vector<ParseMessage>;
    using InterfacesList = std::vector<ParseInterface>;
    using FramesList = std::vector<ParseFrame>;
    using AttributesMap = ParseField::AttributesMap;
    using ElementsList = ParseField::ElementsList;

    explicit ParseNamespace(const ParseNamespaceImpl* impl);
    ParseNamespace(const ParseNamespace& other);
    ~ParseNamespace();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    NamespacesList namespaces() const;
    FieldsList fields() const;
    MessagesList messages() const;
    bool hasMessages() const;
    InterfacesList interfaces() const;
    FramesList frames() const;
    std::string externalRef(bool schemaRef = true) const;

    const AttributesMap& parseExtraAttributes() const;
    const ElementsList& parseExtraElements() const;

private:
    const ParseNamespaceImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
