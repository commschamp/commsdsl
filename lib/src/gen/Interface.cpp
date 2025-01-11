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

#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class InterfaceImpl
{
public:
    using FieldsList = Interface::FieldsList;

    InterfaceImpl(Generator& generator, commsdsl::parse::Interface dslObj, Elem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool createAll()
    {
        if (!m_dslObj.valid()) {
            return true;
        }

        auto fields = m_dslObj.fields();
        m_fields.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = Field::create(m_generator, dslObj, m_parent);
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

    commsdsl::parse::Interface dslObj() const
    {
        return m_dslObj;
    }

    Generator& generator()
    {
        return m_generator;
    }

    const Generator& generator() const
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
    Generator& m_generator;
    commsdsl::parse::Interface m_dslObj;
    Elem* m_parent = nullptr;
    FieldsList m_fields;
    bool m_referenced = false;
}; 

Interface::Interface(Generator& generator, commsdsl::parse::Interface dslObj, Elem* parent) :
    Base(parent),
    m_impl(std::make_unique<InterfaceImpl>(generator, dslObj, this))
{
}

Interface::~Interface() = default;

bool Interface::createAll()
{
    return m_impl->createAll();
}

bool Interface::prepare()
{
    if (!m_impl->prepare()) {
        return false;
    }

    if (!isReferenced()) {
        return true;
    }

    return prepareImpl();
}

bool Interface::write() const
{
    if (!m_impl->write()) {
        return false;
    }

    if (!isReferenced()) {
        return true;
    }

    return writeImpl();
}

const Interface::FieldsList& Interface::fields() const
{
    return m_impl->fields();
}

commsdsl::parse::Interface Interface::dslObj() const
{
    return m_impl->dslObj();
}

Generator& Interface::generator()
{
    return m_impl->generator();
}

const Generator& Interface::generator() const
{
    return m_impl->generator();
}

bool Interface::hasVersionField() const
{
    auto& fList = fields();
    return
        std::any_of(
            fList.begin(), fList.end(),
            [](auto& f)
            {
                return f->dslObj().semanticType() == commsdsl::parse::Field::SemanticType::Version;
            });    
}

bool Interface::isReferenced() const
{
    return m_impl->isReferenced();
}

void Interface::setReferenced(bool value)
{
    m_impl->setReferenced(value);
}

Elem::Type Interface::elemTypeImpl() const
{
    return Type_Interface;
}

bool Interface::prepareImpl()
{
    return true;
}

bool Interface::writeImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
