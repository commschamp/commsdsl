//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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
decltype(auto) elemName(const TElem& elem)
{
    if (!elem.dslObj().parseValid()) {
        return strings::emptyString();
    }
    return elem.dslObj().parseName();
}  

const std::string& nameOfNamespace(const GenElem& elem)
{
    return elemName(static_cast<const GenNamespace&>(elem));
}

const std::string& nameOfMessage(const GenElem& elem)
{
    return elemName(static_cast<const GenMessage&>(elem));
}

const std::string& nameOfField(const GenElem& elem)
{
    return elemName(static_cast<const GenField&>(elem));
}

const std::string& nameOfInterface(const GenElem& elem)
{
    return elemName(static_cast<const GenInterface&>(elem));
}

const std::string& nameOfFrame(const GenElem& elem)
{
    return elemName(static_cast<const GenFrame&>(elem));
}

const std::string& nameOfLayer(const GenElem& elem)
{
    return elemName(static_cast<const GenLayer&>(elem));
}

const std::string& nameOfSchema(const GenElem& elem)
{
    return elemName(static_cast<const GenSchema&>(elem));
}

} // namespace 
    

GenElem::~GenElem() = default;

void GenElem::setParent(GenElem* parent)
{
    m_parent = parent;
}

GenElem* GenElem::getParent()
{
    return m_parent;
}

const GenElem* GenElem::getParent() const
{
    return m_parent;
}

GenElem::Type GenElem::elemType() const
{
    return elemTypeImpl();
}

const std::string& GenElem::name() const
{
    using Func = const std::string& (*)(const GenElem& elem);
    static const Func Map[] = {
        /* Type_Invalid */ nullptr,
        /* Type_Namespace */ &nameOfNamespace,
        /* Type_Message */ &nameOfMessage,
        /* Type_Field */ &nameOfField,
        /* Type_Interface */ &nameOfInterface,
        /* Type_Frame */ &nameOfFrame,
        /* Type_Layer */ &nameOfLayer,    
        /* Type_Schema */ &nameOfSchema,    
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == Type_NumOfValues, "Invalid map");

    auto idx = static_cast<unsigned>(elemType());
    if ((MapSize <= idx) || (Map[idx] == nullptr)) {
        assert(false); // Should not happen
        return strings::emptyString();
    }

    return Map[idx](*this);
}

GenElem::GenElem(GenElem* parent) :
    m_parent(parent)
{
}

} // namespace gen

} // namespace commsdsl
