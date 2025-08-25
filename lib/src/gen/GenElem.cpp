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
decltype(auto) genElemName(const TElem& elem)
{
    if (!elem.genParseObj().parseValid()) {
        return strings::genEmptyString();
    }
    return elem.genParseObj().parseName();
}  

const std::string& genNameOfNamespace(const GenElem& elem)
{
    return genElemName(static_cast<const GenNamespace&>(elem));
}

const std::string& genNameOfMessage(const GenElem& elem)
{
    return genElemName(static_cast<const GenMessage&>(elem));
}

const std::string& genNameOfField(const GenElem& elem)
{
    return genElemName(static_cast<const GenField&>(elem));
}

const std::string& genNameOfInterface(const GenElem& elem)
{
    return genElemName(static_cast<const GenInterface&>(elem));
}

const std::string& genNameOfFrame(const GenElem& elem)
{
    return genElemName(static_cast<const GenFrame&>(elem));
}

const std::string& genNameOfLayer(const GenElem& elem)
{
    return genElemName(static_cast<const GenLayer&>(elem));
}

const std::string& genNameOfSchema(const GenElem& elem)
{
    return genElemName(static_cast<const GenSchema&>(elem));
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
        /* GenType_Namespace */ &genNameOfNamespace,
        /* GenType_Message */ &genNameOfMessage,
        /* GenType_Field */ &genNameOfField,
        /* GenType_Interface */ &genNameOfInterface,
        /* GenType_Frame */ &genNameOfFrame,
        /* GenType_Layer */ &genNameOfLayer,    
        /* GenType_Schema */ &genNameOfSchema,    
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
