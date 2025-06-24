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

    GenInterfaceImpl(GenGenerator& generator, commsdsl::parse::ParseInterface dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool createAll()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        auto fields = m_dslObj.parseFields();
        m_fields.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = GenField::create(m_generator, dslObj, m_parent);
            assert(ptr);
            m_fields.push_back(std::move(ptr));
        }

        return true;        
    }

    bool prepare()
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
                bool result = f->prepare();
                if (result) {
                    f->setReferenced();
                }
                return result;
            });
    }

    bool write() const
    {
        if (!m_referenced) {
            return true;
        }

        bool result = 
            std::all_of(
                m_fields.begin(), m_fields.end(),
                [](auto& fieldPtr) -> bool
                {
                    return fieldPtr->write();
                });

        return result;
    }

    const FieldsList& fields() const
    {
        return m_fields;
    }

    commsdsl::parse::ParseInterface dslObj() const
    {
        return m_dslObj;
    }

    GenGenerator& generator()
    {
        return m_generator;
    }

    const GenGenerator& generator() const
    {
        return m_generator;
    }

    bool isReferenced() const
    {
        return m_referenced;
    }

    void setReferenced(bool value)
    {
        m_referenced = value;
    }

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseInterface m_dslObj;
    GenElem* m_parent = nullptr;
    FieldsList m_fields;
    bool m_referenced = false;
}; 

GenInterface::GenInterface(GenGenerator& generator, commsdsl::parse::ParseInterface dslObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenInterfaceImpl>(generator, dslObj, this))
{
}

GenInterface::~GenInterface() = default;

bool GenInterface::createAll()
{
    return m_impl->createAll();
}

bool GenInterface::prepare()
{
    if (!m_impl->prepare()) {
        return false;
    }

    if (!isReferenced()) {
        return true;
    }

    return prepareImpl();
}

bool GenInterface::write() const
{
    if (!m_impl->write()) {
        return false;
    }

    if (!isReferenced()) {
        return true;
    }

    return writeImpl();
}

const GenInterface::FieldsList& GenInterface::fields() const
{
    return m_impl->fields();
}

commsdsl::parse::ParseInterface GenInterface::dslObj() const
{
    return m_impl->dslObj();
}

std::string GenInterface::adjustedExternalRef() const
{
    auto obj = dslObj();
    if (obj.parseValid()) {
        return obj.parseExternalRef();
    }

    auto* ns = static_cast<const GenNamespace*>(getParent());
    assert(ns != nullptr);
    return ns->adjustedExternalRef() + '.' + strings::messageClassStr();
}

const std::string& GenInterface::adjustedName() const
{
    auto& str = name();
    if (!str.empty()) {
        return str;
    }

    return strings::messageClassStr();
}

GenGenerator& GenInterface::generator()
{
    return m_impl->generator();
}

const GenGenerator& GenInterface::generator() const
{
    return m_impl->generator();
}

bool GenInterface::hasVersionField() const
{
    auto& fList = fields();
    return
        std::any_of(
            fList.begin(), fList.end(),
            [](auto& f)
            {
                return f->dslObj().parseSemanticType() == commsdsl::parse::ParseField::SemanticType::Version;
            });    
}

bool GenInterface::isReferenced() const
{
    return m_impl->isReferenced();
}

void GenInterface::setReferenced(bool value)
{
    m_impl->setReferenced(value);
}

const GenNamespace* GenInterface::parentNamespace() const
{
    auto* parent = getParent();
    assert(parent != nullptr);
    assert(parent->elemType() == GenElem::Type_Namespace);
    return static_cast<const GenNamespace*>(parent);
}

GenElem::Type GenInterface::elemTypeImpl() const
{
    return Type_Interface;
}

bool GenInterface::prepareImpl()
{
    return true;
}

bool GenInterface::writeImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
