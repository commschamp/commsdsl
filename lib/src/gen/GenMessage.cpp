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

#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class GenMessageImpl
{
public:
    using FieldsList = GenMessage::FieldsList;

    GenMessageImpl(GenGenerator& generator, commsdsl::parse::ParseMessage dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool isPrepared() const
    {
        return m_prepared;
    }

    void setPrepared()
    {
        m_prepared = true;
    }    

    bool createAll()
    {
        if (!m_dslObj.valid()) {
            return true;
        }

        auto fields = m_dslObj.fields();
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

        if (!m_dslObj.valid()) {
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

    commsdsl::parse::ParseMessage dslObj() const
    {
        return m_dslObj;
    }

    const FieldsList& fields() const
    {
        return m_fields;
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
    commsdsl::parse::ParseMessage m_dslObj;
    GenElem* m_parent = nullptr;
    FieldsList m_fields;
    bool m_prepared = false;
    bool m_referenced = false;
}; 

GenMessage::GenMessage(GenGenerator& generator, commsdsl::parse::ParseMessage dslObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenMessageImpl>(generator, dslObj, this))
{
}

GenMessage::~GenMessage() = default;

bool GenMessage::isPrepared() const
{
    return m_impl->isPrepared();
}

bool GenMessage::createAll()
{
    return m_impl->createAll();
}

bool GenMessage::prepare()
{
    if (m_impl->isPrepared()) {
        return true;
    }

    if (!m_impl->prepare()) {
        return false;
    }

    if (!isReferenced()) {
        return true;
    }

    bool result = prepareImpl();
    if (result) {
        m_impl->setPrepared();
    }
    return result;
}

bool GenMessage::write() const
{
    if (!m_impl->write()) {
        return false;
    }

    if (!m_impl->isReferenced()) {
        return false;
    }

    return writeImpl();
}

bool GenMessage::isReferenced() const
{
    return m_impl->isReferenced();
}

void GenMessage::setReferenced(bool value = true)
{
    m_impl->setReferenced(value);
}

commsdsl::parse::ParseMessage GenMessage::dslObj() const
{
    return m_impl->dslObj();
}

const GenMessage::FieldsList& GenMessage::fields() const
{
    return m_impl->fields();
}

GenGenerator& GenMessage::generator()
{
    return m_impl->generator();
}

const GenGenerator& GenMessage::generator() const
{
    return m_impl->generator();
}

GenElem::Type GenMessage::elemTypeImpl() const
{
    return Type_Message;
}

bool GenMessage::prepareImpl()
{
    return true;
}

bool GenMessage::writeImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
