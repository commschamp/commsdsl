//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenElem.h"

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/GenSchema.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <type_traits>

namespace commsdsl
{

namespace gen
{

namespace
{

template <typename TElem>
[[nodiscard]] decltype(auto) genElemNameInternal(const TElem& elem)
{
    if (!elem.genParseObj().parseValid()) {
        return strings::genEmptyString();
    }
    return elem.genParseObj().parseName();
}

[[nodiscard]] const std::string& genNameOfNamespaceInternal(const GenElem& elem)
{
    return genElemNameInternal(static_cast<const GenNamespace&>(elem));
}

[[nodiscard]] const std::string& genNameOfMessageInternal(const GenElem& elem)
{
    return genElemNameInternal(static_cast<const GenMessage&>(elem));
}

[[nodiscard]] const std::string& genNameOfFieldInternal(const GenElem& elem)
{
    return genElemNameInternal(static_cast<const GenField&>(elem));
}

[[nodiscard]] const std::string& genNameOfInterfaceInternal(const GenElem& elem)
{
    return genElemNameInternal(static_cast<const GenInterface&>(elem));
}

[[nodiscard]] const std::string& genNameOfFrameInternal(const GenElem& elem)
{
    return genElemNameInternal(static_cast<const GenFrame&>(elem));
}

[[nodiscard]] const std::string& genNameOfLayerInternal(const GenElem& elem)
{
    return genElemNameInternal(static_cast<const GenLayer&>(elem));
}

[[nodiscard]] const std::string& genNameOfSchemaInternal(const GenElem& elem)
{
    return genElemNameInternal(static_cast<const GenSchema&>(elem));
}

} // namespace

GenElem::~GenElem() = default;

void GenElem::genSetParent(GenElem* parent)
{
    m_parent = parent;
}

GenElem* GenElem::genGetParent()
{
    return m_parent;
}

const GenElem* GenElem::genGetParent() const
{
    return m_parent;
}

GenElem::GenType GenElem::genElemType() const
{
    return genElemTypeImpl();
}

const std::string& GenElem::genName() const
{
    using Func = const std::string& (*)(const GenElem& elem);
    static const Func Map[] = {
        /* GenType_Invalid */ nullptr,
        /* GenType_Namespace */ &genNameOfNamespaceInternal,
        /* GenType_Message */ &genNameOfMessageInternal,
        /* GenType_Field */ &genNameOfFieldInternal,
        /* GenType_Interface */ &genNameOfInterfaceInternal,
        /* GenType_Frame */ &genNameOfFrameInternal,
        /* GenType_Layer */ &genNameOfLayerInternal,
        /* GenType_Schema */ &genNameOfSchemaInternal,
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == GenType_NumOfValues, "Invalid map");

    auto idx = static_cast<unsigned>(genElemType());
    if ((MapSize <= idx) || (Map[idx] == nullptr)) {
        assert(false); // Should not happen
        return strings::genEmptyString();
    }

    return Map[idx](*this);
}

GenElem::GenElem(GenElem* parent) :
    m_parent(parent)
{
}

} // namespace gen

} // namespace commsdsl
