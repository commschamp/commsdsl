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

#include "commsdsl/gen/OptionalField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class OptionalFieldImpl
{
public:
    OptionalFieldImpl(Generator& generator, commsdsl::parse::OptionalField dslObj, Elem* parent): 
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        auto field = m_dslObj.field();
        assert(field.valid());

        if (!field.externalRef().empty()) {
            m_externalField = m_generator.findField(field.externalRef());
            assert(m_externalField != nullptr);
            return true;
        }

        m_memberField = Field::create(m_generator, field, m_parent);
        assert(m_memberField);
        if (!m_memberField->prepare()) {
            return false;
        }

        return true;
    }

    Field* externalField()
    {
        return m_externalField;
    }

    const Field* externalField() const
    {
        return m_externalField;
    }

    Field* memberField()
    {
        return m_memberField.get();
    }

    const Field* memberField() const
    {
        return m_memberField.get();
    }    

private:
    Generator& m_generator;
    commsdsl::parse::OptionalField m_dslObj;
    Elem* m_parent = nullptr;
    Field* m_externalField = nullptr;
    FieldPtr m_memberField;
}; 

OptionalField::OptionalField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<OptionalFieldImpl>(generator, optionalDslObj(), this))
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Optional);
}

OptionalField::~OptionalField() = default;

bool OptionalField::prepareImpl()
{
    return m_impl->prepare();
}

Field* OptionalField::externalField()
{
    return m_impl->externalField();
}

const Field* OptionalField::externalField() const
{
    return m_impl->externalField();
}

Field* OptionalField::memberField()
{
    return m_impl->memberField();
}

const Field* OptionalField::memberField() const
{
    return m_impl->memberField();
}   

commsdsl::parse::OptionalField OptionalField::optionalDslObj() const
{
    return commsdsl::parse::OptionalField(dslObj());
}

} // namespace gen

} // namespace commsdsl
