//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include <memory>

#include "commsdsl/parse/Namespace.h"

#include "commsdsl/gen/Elem.h"

namespace commsdsl
{

namespace gen
{

class NamespaceImpl;
class Namespace : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Namespace>;
    using NamespacesList = std::vector<Ptr>;

    explicit Namespace(Generator& generator, const commsdsl::parse::Namespace& dslObj, Elem* parent = nullptr);
    virtual ~Namespace();

    bool prepare();
    bool write();

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool writeImpl();

private:
    std::unique_ptr<NamespaceImpl> m_impl;
};

using NamespacePtr = Namespace::Ptr;

} // namespace gen

} // namespace commsdsl
