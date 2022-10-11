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

#include "commsdsl/gen/RefField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class RefFieldImpl
{
public:

    RefFieldImpl(Generator& generator, commsdsl::parse::RefField dslObj): 
        m_generator(generator),
        m_dslObj(dslObj)
    {
    }

    bool prepare()
    {
        auto fieldObj = m_dslObj.field();
        if (fieldObj.isPseudo() != m_dslObj.isPseudo()) {
            m_generator.logger().error(
                m_dslObj.schemaPos() +
                "Having \"pseudo\" property value for <ref> field \"" + m_dslObj.name() +
                "\" that differs to one of the referenced field is not supported by the code generator.");
            return false;
        }

        if (fieldObj.isFailOnInvalid() != m_dslObj.isFailOnInvalid()) {
            m_generator.logger().error(
                m_dslObj.schemaPos() +
                "Having \"failOnInvalid\" property value for <ref> field \"" + m_dslObj.name() +
                "\" that differs to one of the referenced field is not supported by the code generator.");
            return false;
        }

        m_referencedField = m_generator.findField(fieldObj.externalRef());
        if (m_referencedField == nullptr) {
            assert(false);
            return false;
        }

        return true;
    }

    Field* referencedField()
    {
        return m_referencedField;
    }

    const Field* referencedField() const
    {
        return m_referencedField;
    }

    void setReferenced()
    {
        assert(m_referencedField != nullptr);
        m_referencedField->setReferenced();
    }

private:
    Generator& m_generator;
    commsdsl::parse::RefField m_dslObj;
    Field* m_referencedField = nullptr;
};       

RefField::RefField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<RefFieldImpl>(generator, refDslObj()))
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Ref);
}

RefField::~RefField() = default;

Field* RefField::referencedField()
{
    return m_impl->referencedField();
}

const Field* RefField::referencedField() const
{
    return m_impl->referencedField();
}

bool RefField::prepareImpl()
{
    return m_impl->prepare();
}

void RefField::setReferencedImpl()
{
    m_impl->setReferenced();
}

commsdsl::parse::RefField RefField::refDslObj() const
{
    return commsdsl::parse::RefField(dslObj());
}

} // namespace gen

} // namespace commsdsl
