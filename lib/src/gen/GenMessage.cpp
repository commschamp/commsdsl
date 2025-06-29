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
    using ParseMessage = GenMessage::ParseMessage;

    GenMessageImpl(GenGenerator& generator, ParseMessage parseObj, GenElem* parent) :
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genIsPrepared() const
    {
        return m_prepared;
    }

    void genSetPrepared()
    {
        m_prepared = true;
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

        if (!m_parseObj.parseValid()) {
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

    ParseMessage genParseObj() const
    {
        return m_parseObj;
    }

    const FieldsList& genFields() const
    {
        return m_fields;
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
    ParseMessage m_parseObj;
    GenElem* m_parent = nullptr;
    FieldsList m_fields;
    bool m_prepared = false;
    bool m_referenced = false;
}; 

GenMessage::GenMessage(GenGenerator& generator, ParseMessage parseObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenMessageImpl>(generator, parseObj, this))
{
}

GenMessage::~GenMessage() = default;

bool GenMessage::genIsPrepared() const
{
    return m_impl->genIsPrepared();
}

bool GenMessage::genCreateAll()
{
    return m_impl->genCreateAll();
}

bool GenMessage::genPrepare()
{
    if (m_impl->genIsPrepared()) {
        return true;
    }

    if (!m_impl->genPrepare()) {
        return false;
    }

    if (!genIsReferenced()) {
        return true;
    }

    bool result = genPrepareImpl();
    if (result) {
        m_impl->genSetPrepared();
    }
    return result;
}

bool GenMessage::genWrite() const
{
    if (!m_impl->genWrite()) {
        return false;
    }

    if (!m_impl->genIsReferenced()) {
        return false;
    }

    return genWriteImpl();
}

bool GenMessage::genIsReferenced() const
{
    return m_impl->genIsReferenced();
}

void GenMessage::genSetReferenced(bool value = true)
{
    m_impl->genSetReferenced(value);
}

GenMessage::ParseMessage GenMessage::genParseObj() const
{
    return m_impl->genParseObj();
}

const GenMessage::FieldsList& GenMessage::genFields() const
{
    return m_impl->genFields();
}

GenGenerator& GenMessage::genGenerator()
{
    return m_impl->genGenerator();
}

const GenGenerator& GenMessage::genGenerator() const
{
    return m_impl->genGenerator();
}

GenElem::Type GenMessage::genElemTypeImpl() const
{
    return Type_Message;
}

bool GenMessage::genPrepareImpl()
{
    return true;
}

bool GenMessage::genWriteImpl() const
{
    return true;
}

} // namespace gen

} // namespace commsdsl
