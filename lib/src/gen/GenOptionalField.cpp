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

#include "commsdsl/gen/GenOptionalField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenOptionalFieldImpl
{
public:
    GenOptionalFieldImpl(GenGenerator& generator, commsdsl::parse::ParseOptionalField dslObj, GenElem* parent): 
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        auto field = m_dslObj.parseField();
        assert(field.parseValid());

        if (!field.parseExternalRef().empty()) {
            m_externalField = m_generator.findField(field.parseExternalRef());
            assert(m_externalField != nullptr);
            return true;
        }

        m_memberField = GenField::create(m_generator, field, m_parent);
        assert(m_memberField);
        if (!m_memberField->prepare()) {
            return false;
        }

        return true;
    }

    GenField* externalField()
    {
        return m_externalField;
    }

    const GenField* externalField() const
    {
        return m_externalField;
    }

    GenField* memberField()
    {
        return m_memberField.get();
    }

    const GenField* memberField() const
    {
        return m_memberField.get();
    }    

    void setReferenced()
    {
        GenField::setFieldReferencedIfExists(m_externalField);
        GenField::setFieldReferencedIfExists(m_memberField.get());
    }

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseOptionalField m_dslObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalField = nullptr;
    FieldPtr m_memberField;
}; 

GenOptionalField::GenOptionalField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenOptionalFieldImpl>(generator, optionalDslObj(), this))
{
    assert(dslObj.parseKind() == commsdsl::parse::ParseField::Kind::Optional);
}

GenOptionalField::~GenOptionalField() = default;

GenField* GenOptionalField::externalField()
{
    return m_impl->externalField();
}

const GenField* GenOptionalField::externalField() const
{
    return m_impl->externalField();
}

GenField* GenOptionalField::memberField()
{
    return m_impl->memberField();
}

const GenField* GenOptionalField::memberField() const
{
    return m_impl->memberField();
}  

bool GenOptionalField::prepareImpl()
{
    return m_impl->prepare();
}

void GenOptionalField::setReferencedImpl()
{
    m_impl->setReferenced();
}

commsdsl::parse::ParseOptionalField GenOptionalField::optionalDslObj() const
{
    return commsdsl::parse::ParseOptionalField(dslObj());
}

} // namespace gen

} // namespace commsdsl
