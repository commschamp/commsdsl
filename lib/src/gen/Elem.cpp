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

#include "commsdsl/gen/Elem.h"

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/Layer.h"
#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Namespace.h"
#include "commsdsl/gen/Schema.h"

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
    if (!elem.dslObj().valid()) {
        return strings::emptyString();
    }
    return elem.dslObj().name();
}  

const std::string& nameOfNamespace(const Elem& elem)
{
    return elemName(static_cast<const Namespace&>(elem));
}

const std::string& nameOfMessage(const Elem& elem)
{
    return elemName(static_cast<const Message&>(elem));
}

const std::string& nameOfField(const Elem& elem)
{
    return elemName(static_cast<const Field&>(elem));
}

const std::string& nameOfInterface(const Elem& elem)
{
    return elemName(static_cast<const Interface&>(elem));
}

const std::string& nameOfFrame(const Elem& elem)
{
    return elemName(static_cast<const Frame&>(elem));
}

const std::string& nameOfLayer(const Elem& elem)
{
    return elemName(static_cast<const Layer&>(elem));
}

const std::string& nameOfSchema(const Elem& elem)
{
    return elemName(static_cast<const Schema&>(elem));
}

} // namespace 
    

Elem::~Elem() = default;

void Elem::setParent(Elem* parent)
{
    m_parent = parent;
}

Elem* Elem::getParent()
{
    return m_parent;
}

const Elem* Elem::getParent() const
{
    return m_parent;
}

Elem::Type Elem::elemType() const
{
    return elemTypeImpl();
}

const std::string& Elem::name() const
{
    using Func = const std::string& (*)(const Elem& elem);
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

Elem::Elem(Elem* parent) :
    m_parent(parent)
{
}

} // namespace gen

} // namespace commsdsl
