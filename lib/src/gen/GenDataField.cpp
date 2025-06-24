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

#include "commsdsl/gen/GenDataField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenDataFieldImpl
{
public:

    GenDataFieldImpl(GenGenerator& generator, commsdsl::parse::ParseDataField dslObj, GenElem* parent): 
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        if (!m_dslObj.parseHasLengthPrefixField()) {
            return true;
        }

        auto prefix = m_dslObj.parseLengthPrefixField();
        if (!prefix.parseExternalRef().empty()) {
            m_externalPrefixField = m_generator.findField(prefix.parseExternalRef());
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
    commsdsl::parse::ParseDataField m_dslObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalPrefixField = nullptr;
    FieldPtr m_memberPrefixField;
};    

GenDataField::GenDataField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenDataFieldImpl>(generator, dataDslObj(), this))
{
    assert(dslObj.parseKind() == commsdsl::parse::ParseField::Kind::Data);
}

GenDataField::~GenDataField() = default;

GenField* GenDataField::externalPrefixField()
{
    return m_impl->externalPrefixField();
}

const GenField* GenDataField::externalPrefixField() const
{
    return m_impl->externalPrefixField();
}

GenField* GenDataField::memberPrefixField()
{
    return m_impl->memberPrefixField();
}

const GenField* GenDataField::memberPrefixField() const
{
    return m_impl->memberPrefixField();
}

bool GenDataField::prepareImpl()
{
    return m_impl->prepare();
}

void GenDataField::setReferencedImpl()
{
    m_impl->setReferenced();
}

commsdsl::parse::ParseDataField GenDataField::dataDslObj() const
{
    return commsdsl::parse::ParseDataField(dslObj());
}

} // namespace gen

} // namespace commsdsl
