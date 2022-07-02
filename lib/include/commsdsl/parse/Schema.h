//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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
#include "commsdsl/parse/Endian.h"
#include "commsdsl/parse/Namespace.h"

namespace commsdsl
{

namespace parse
{

class SchemaImpl;
class COMMSDSL_API Schema
{
public:
    using AttributesMap = std::multimap<std::string, std::string>;
    using ElementsList = std::vector<std::string>;
    using NamespacesList = std::vector<Namespace>;
    using MessagesList = Namespace::MessagesList;
    using PlatformsList = Message::PlatformsList;

    explicit Schema(const SchemaImpl* impl);

    bool valid() const;

    const std::string& name() const;

    const std::string& description() const;

    unsigned id() const;

    unsigned version() const;

    unsigned dslVersion() const;

    Endian endian() const;

    bool nonUniqueMsgIdAllowed() const;

    const AttributesMap& extraAttributes() const;

    const ElementsList& extraElements() const;

    NamespacesList namespaces() const;

    const PlatformsList& platforms() const;

    MessagesList allMessages() const;

private:
    const SchemaImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
