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

#include "commsdsl/gen/GenStringField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenStringFieldImpl
{
    using Base = GenField;
public:

    GenStringFieldImpl(GenGenerator& generator, commsdsl::parse::ParseStringField dslObj, GenElem* parent): 
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

        m_memberPrefixField = GenField::create(m_generator, prefix, m_parent);
        if (!m_memberPrefixField->prepare()) {
            return false;
        }

        return true;
    }

    GenField* externalPrefixField()
    {
        return m_externalPrefixField;
    }

    const GenField* externalPrefixField() const
    {
        return m_externalPrefixField;
    }

    GenField* memberPrefixField()
    {
        return m_memberPrefixField.get();
    }

    const GenField* memberPrefixField() const
    {
        return m_memberPrefixField.get();
    }    

    void setReferenced()
    {
        GenField::setFieldReferencedIfExists(m_externalPrefixField);
        GenField::setFieldReferencedIfExists(m_memberPrefixField.get());
    }

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseStringField m_dslObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalPrefixField = nullptr;
    FieldPtr m_memberPrefixField;
};


GenStringField::GenStringField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenStringFieldImpl>(generator, stringDslObj(), this))
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::String);
}

GenStringField::~GenStringField() = default;

GenField* GenStringField::externalPrefixField()
{
    return m_impl->externalPrefixField();
}

const GenField* GenStringField::externalPrefixField() const
{
    return m_impl->externalPrefixField();
}

GenField* GenStringField::memberPrefixField()
{
    return m_impl->memberPrefixField();
}

const GenField* GenStringField::memberPrefixField() const
{
    return m_impl->memberPrefixField();
}

bool GenStringField::prepareImpl()
{
    return m_impl->prepare();
}

void GenStringField::setReferencedImpl()
{
    m_impl->setReferenced();
}

commsdsl::parse::ParseStringField GenStringField::stringDslObj() const
{
    return commsdsl::parse::ParseStringField(dslObj());
}

} // namespace gen

} // namespace commsdsl
