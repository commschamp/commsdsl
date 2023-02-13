//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "Alias.h"
#include "Field.h"
#include "OptCond.h"
#include "OverrideType.h"

namespace commsdsl
{

namespace parse
{

class MessageImpl;
class COMMSDSL_API Message
{
public:
    enum class Sender
    {
        Both,
        Client,
        Server,
        NumOfValues
    };

    using FieldsList = std::vector<Field>;
    using AttributesMap = Field::AttributesMap;
    using ElementsList = Field::ElementsList;
    using PlatformsList = std::vector<std::string>;
    using AliasesList = std::vector<Alias>;

    explicit Message(const MessageImpl* impl);
    Message(const Message& other);
    ~Message();

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
    Sender sender() const;
    OverrideType readOverride() const;
    OverrideType writeOverride() const;
    OverrideType refreshOverride() const;
    OverrideType lengthOverride() const;
    OverrideType validOverride() const;
    OverrideType nameOverride() const;    
    const std::string& copyCodeFrom() const;
    OptCond construct() const;
    OptCond readCond() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;
    const PlatformsList& platforms() const;

protected:
    const MessageImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
