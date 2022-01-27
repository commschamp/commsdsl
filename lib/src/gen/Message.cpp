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

#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class MessageImpl
{
public:
    using FieldsList = Message::FieldsList;

    MessageImpl(Generator& generator, commsdsl::parse::Message dslObj, Elem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        if (!m_dslObj.valid()) {
            return true;
        }

        auto fields = m_dslObj.fields();
        m_fields.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = Field::create(m_generator, dslObj, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_fields.push_back(std::move(ptr));
        }

        return true;
    }

    bool write()
    {
        bool result = 
            std::all_of(
                m_fields.begin(), m_fields.end(),
                [](auto& fieldPtr) -> bool
                {
                    return fieldPtr->write();
                });

        return result;
    }

    commsdsl::parse::Message dslObj() const
    {
        return m_dslObj;
    }

    const FieldsList& fields() const
    {
        return m_fields;
    }

    Generator& generator()
    {
        return m_generator;
    }

    const Generator& generator() const
    {
        return m_generator;
    }

private:
    Generator& m_generator;
    commsdsl::parse::Message m_dslObj;
    Elem* m_parent = nullptr;
    FieldsList m_fields;
}; 

Message::Message(Generator& generator, commsdsl::parse::Message dslObj, Elem* parent) :
    Base(parent),
    m_impl(std::make_unique<MessageImpl>(generator, dslObj, this))
{
}

Message::~Message() = default;

bool Message::prepare()
{
    if (!m_impl->prepare()) {
        return false;
    }

    return prepareImpl();
}

bool Message::write()
{
    if (!m_impl->write()) {
        return false;
    }

    return writeImpl();
}

commsdsl::parse::Message Message::dslObj() const
{
    return m_impl->dslObj();
}

const Message::FieldsList& Message::fields() const
{
    return m_impl->fields();
}

Generator& Message::generator()
{
    return m_impl->generator();
}

const Generator& Message::generator() const
{
    return m_impl->generator();
}


Elem::Type Message::elemTypeImpl() const
{
    return Type_Message;
}

bool Message::prepareImpl()
{
    return true;
}

bool Message::writeImpl()
{
    return true;
}

} // namespace gen

} // namespace commsdsl
