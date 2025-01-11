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
#include "Field.h"
#include "Message.h"
#include "Interface.h"
#include "Frame.h"

namespace commsdsl
{

namespace parse
{

class NamespaceImpl;
class COMMSDSL_API Namespace
{
public:
    using NamespacesList = std::vector<Namespace>;
    using FieldsList = std::vector<Field>;
    using MessagesList = std::vector<Message>;
    using InterfacesList = std::vector<Interface>;
    using FramesList = std::vector<Frame>;
    using AttributesMap = Field::AttributesMap;
    using ElementsList = Field::ElementsList;

    explicit Namespace(const NamespaceImpl* impl);
    Namespace(const Namespace& other);
    ~Namespace();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    NamespacesList namespaces() const;
    FieldsList fields() const;
    MessagesList messages() const;
    InterfacesList interfaces() const;
    FramesList frames() const;
    std::string externalRef(bool schemaRef = true) const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

private:
    const NamespaceImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
