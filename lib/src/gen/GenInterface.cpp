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

#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class GenInterfaceImpl
{
public:
    using FieldsList = GenInterface::FieldsList;
    using ParseInterface = GenInterface::ParseInterface;

    GenInterfaceImpl(GenGenerator& generator, ParseInterface parseObj, GenElem* parent) :
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genCreateAll()
    {
        if (!m_parseObj.parseValid()) {
            return true;
        }

        auto fields = m_parseObj.parseFields();
        m_fields.reserve(fields.size());
        for (auto& parseObj : fields) {
            auto ptr = GenField::genCreate(m_generator, parseObj, m_parent);
            assert(ptr);
            m_fields.push_back(std::move(ptr));
        }

        return true;        
    }

    bool genPrepare()
    {
        if (!m_referenced) {
            return true;
        }

        if (m_fields.empty()) {
            return true;
        }

        return std::all_of(
            m_fields.begin(), m_fields.end(),
            [](auto& f)
            {
                bool result = f->genPrepare();
                if (result) {
                    f->genSetReferenced();
                }
                return result;
            });
    }

    bool genWrite() const
    {
        if (!m_referenced) {
            return true;
        }

        bool result = 
            std::all_of(
                m_fields.begin(), m_fields.end(),
                [](auto& fieldPtr) -> bool
                {
                    return fieldPtr->genWrite();
                });

        return result;
    }

    const FieldsList& genFields() const
    {
        return m_fields;
    }

    ParseInterface genParseObj() const
    {
        return m_parseObj;
    }

    GenGenerator& genGenerator()
    {
        return m_generator;
    }

    const GenGenerator& genGenerator() const
    {
        return m_generator;
    }

    bool genIsReferenced() const
    {
        return m_referenced;
    }

    void genSetReferenced(bool value)
    {
        m_referenced = value;
    }

private:
    GenGenerator& m_generator;
    ParseInterface m_parseObj;
    GenElem* m_parent = nullptr;
    FieldsList m_fields;
    bool m_referenced = false;
}; 

GenInterface::GenInterface(GenGenerator& generator, ParseInterface parseObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenInterfaceImpl>(generator, parseObj, this))
{
}

GenInterface::~GenInterface() = default;

bool GenInterface::genCreateAll()
{
    return m_impl->genCreateAll();
}

bool GenInterface::genPrepare()
{
    if (!m_impl->genPrepare()) {
        return false;
    }

    if (!genIsReferenced()) {
        return true;
    }

    return genPrepareImpl();
}

bool GenInterface::genWrite() const
{
    if (!m_impl->genWrite()) {
        return false;
    }

    if (!genIsReferenced()) {
        return true;
    }

    return genWriteImpl();
}

const GenInterface::FieldsList& GenInterface::genFields() const
{
    return m_impl->genFields();
}

GenInterface::ParseInterface GenInterface::genParseObj() const
{
    return m_impl->genParseObj();
}

std::string GenInterface::genAdjustedExternalRef() const
{
    auto obj = genParseObj();
    if (obj.parseValid()) {
        return obj.parseExternalRef();
    }

    auto* ns = static_cast<const GenNamespace*>(genGetParent());
    assert(ns != nullptr);
    return ns->genAdjustedExternalRef() + '.' + strings::genMessageClassStr();
}

const std::string& GenInterface::genAdjustedName() const
{
    auto& str = genName();
    if (!str.empty()) {
        return str;
    }

    return strings::genMessageClassStr();
}

GenGenerator& GenInterface::genGenerator()
{
    return m_impl->genGenerator();
}

const GenGenerator& GenInterface::genGenerator() const
{
    return m_impl->genGenerator();
}

bool GenInterface::genHasVersionField() const
{
    auto& fList = genFields();
    return
        std::any_of(
            fList.begin(), fList.end(),
            [](auto& f)
            {
                return f->genParseObj().parseSemanticType() == commsdsl::parse::ParseField::SemanticType::Version;
            });    
}

bool GenInterface::genIsReferenced() const
{
    return m_impl->genIsReferenced();
}

void GenInterface::genSetReferenced(bool value)
{
    m_impl->genSetReferenced(value);
}

const GenNamespace* GenInterface::genParentNamespace() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == GenElem::Type_Namespace);
    return static_cast<const GenNamespace*>(parent);
}

GenElem::Type GenInterface::genElemTypeImpl() const
{
    return Type_Interface;
}

bool GenInterface::genPrepareImpl()
{
    return true;
}

bool GenInterface::genWriteImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
