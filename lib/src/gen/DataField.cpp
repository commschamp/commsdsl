//
// Copyright 2021 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/DataField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class DataFieldImpl
{
public:

    DataFieldImpl(Generator& generator, commsdsl::parse::DataField dslObj, Elem* parent): 
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        if (!m_dslObj.hasLengthPrefixField()) {
            return true;
        }

        auto prefix = m_dslObj.lengthPrefixField();
        if (!prefix.externalRef().empty()) {
            m_externalPrefixField = m_generator.findField(prefix.externalRef());
            assert(m_externalPrefixField != nullptr);
            return true;
        }

        m_memberPrefixField = Field::create(m_generator, prefix, m_parent);
        if (!m_memberPrefixField->prepare()) {
            return false;
        }

        return true;
    }

    Field* externalPrefixField()
    {
        return m_externalPrefixField;
    }

    const Field* externalPrefixField() const
    {
        return m_externalPrefixField;
    }

    Field* memberPrefixField()
    {
        return m_memberPrefixField.get();
    }

    const Field* memberPrefixField() const
    {
        return m_memberPrefixField.get();
    }    

    void setReferenced()
    {
        Field::setFieldReferencedIfExists(m_externalPrefixField);
        Field::setFieldReferencedIfExists(m_memberPrefixField.get());
    }

private:
    Generator& m_generator;
    commsdsl::parse::DataField m_dslObj;
    Elem* m_parent = nullptr;
    Field* m_externalPrefixField = nullptr;
    FieldPtr m_memberPrefixField;
};    

DataField::DataField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<DataFieldImpl>(generator, dataDslObj(), this))
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Data);
}

DataField::~DataField() = default;

Field* DataField::externalPrefixField()
{
    return m_impl->externalPrefixField();
}

const Field* DataField::externalPrefixField() const
{
    return m_impl->externalPrefixField();
}

Field* DataField::memberPrefixField()
{
    return m_impl->memberPrefixField();
}

const Field* DataField::memberPrefixField() const
{
    return m_impl->memberPrefixField();
}

bool DataField::prepareImpl()
{
    return m_impl->prepare();
}

void DataField::setReferencedImpl()
{
    m_impl->setReferenced();
}

commsdsl::parse::DataField DataField::dataDslObj() const
{
    return commsdsl::parse::DataField(dslObj());
}

} // namespace gen

} // namespace commsdsl
