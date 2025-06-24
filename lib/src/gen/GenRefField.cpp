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

#include "commsdsl/gen/GenRefField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenRefFieldImpl
{
public:

    GenRefFieldImpl(GenGenerator& generator, commsdsl::parse::ParseRefField dslObj): 
        m_generator(generator),
        m_dslObj(dslObj)
    {
    }

    bool prepare()
    {
        auto fieldObj = m_dslObj.parseField();
        if (fieldObj.parseIsPseudo() != m_dslObj.parseIsPseudo()) {
            m_generator.logger().error(
                m_dslObj.parseSchemaPos() +
                "Having \"pseudo\" property value for <ref> field \"" + m_dslObj.parseName() +
                "\" that differs to one of the referenced field is not supported by the code generator.");
            return false;
        }

        if (fieldObj.parseIsFailOnInvalid() != m_dslObj.parseIsFailOnInvalid()) {
            m_generator.logger().error(
                m_dslObj.parseSchemaPos() +
                "Having \"failOnInvalid\" property value for <ref> field \"" + m_dslObj.parseName() +
                "\" that differs to one of the referenced field is not supported by the code generator.");
            return false;
        }

        m_referencedField = m_generator.findField(fieldObj.parseExternalRef());
        if (m_referencedField == nullptr) {
            assert(false);
            return false;
        }

        return true;
    }

    GenField* referencedField()
    {
        return m_referencedField;
    }

    const GenField* referencedField() const
    {
        return m_referencedField;
    }

    void setReferenced()
    {
        assert(m_referencedField != nullptr);
        m_referencedField->setReferenced();
    }

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseRefField m_dslObj;
    GenField* m_referencedField = nullptr;
};       

GenRefField::GenRefField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenRefFieldImpl>(generator, refDslObj()))
{
    assert(dslObj.parseKind() == commsdsl::parse::ParseField::Kind::Ref);
}

GenRefField::~GenRefField() = default;

GenField* GenRefField::referencedField()
{
    return m_impl->referencedField();
}

const GenField* GenRefField::referencedField() const
{
    return m_impl->referencedField();
}

bool GenRefField::prepareImpl()
{
    return m_impl->prepare();
}

void GenRefField::setReferencedImpl()
{
    m_impl->setReferenced();
}

GenRefField::FieldRefInfo GenRefField::processInnerRefImpl(const std::string& refStr) const
{
    auto* field = referencedField();
    assert(field != nullptr);
    return field->processInnerRef(refStr);
}

commsdsl::parse::ParseRefField GenRefField::refDslObj() const
{
    return commsdsl::parse::ParseRefField(dslObj());
}

} // namespace gen

} // namespace commsdsl
