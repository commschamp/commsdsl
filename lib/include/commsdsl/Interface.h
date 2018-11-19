//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "CommsdslApi.h"
#include "Field.h"

namespace commsdsl
{

class InterfaceImpl;
class COMMSDSL_API Interface
{
public:
    using FieldsList = std::vector<Field>;
    using AttributesMap = Schema::AttributesMap;
    using ElementsList = Schema::ElementsList;

    explicit Interface(const InterfaceImpl* impl);
    Interface(const Interface& other);
    ~Interface();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    FieldsList fields() const;
    std::string externalRef() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const InterfaceImpl* m_pImpl;
};

} // namespace commsdsl
