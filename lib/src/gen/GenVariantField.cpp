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

#include "commsdsl/gen/GenVariantField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{
class GenVariantFieldImpl
{
public:
    using FieldsList = GenVariantField::FieldsList;

    GenVariantFieldImpl(GenGenerator& generator, commsdsl::parse::ParseVariantField dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        auto fields = m_dslObj.parseMembers();
        m_members.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = GenField::create(m_generator, dslObj, m_parent);
            assert(ptr);
            if (!ptr->prepare()) {
                return false;
            }

            m_members.push_back(std::move(ptr));
        }

        return true;
    }

    const FieldsList& members() const
    {
        return m_members;
    }

    void setReferenced()
    {
        for (auto& m : m_members) {
            m->setReferenced();
        }
    }    

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseVariantField m_dslObj;
    GenElem* m_parent = nullptr;
    FieldsList m_members;
};

GenVariantField::GenVariantField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenVariantFieldImpl>(generator, variantDslObj(), this))
{
    assert(dslObj.parseKind() == commsdsl::parse::ParseField::Kind::Variant);
}

GenVariantField::~GenVariantField() = default;

const GenVariantField::FieldsList& GenVariantField::members() const
{
    return m_impl->members();
}

bool GenVariantField::prepareImpl()
{
    return m_impl->prepare();
}

void GenVariantField::setReferencedImpl()
{
    m_impl->setReferenced();
}

GenVariantField::FieldRefInfo GenVariantField::processInnerRefImpl(const std::string& refStr) const
{
    auto& memFields = members();
    if (!memFields.empty()) {
        return processMemberRef(memFields, refStr);
    }    

    return Base::processInnerRefImpl(refStr);
}

commsdsl::parse::ParseVariantField GenVariantField::variantDslObj() const
{
    return commsdsl::parse::ParseVariantField(dslObj());
}

} // namespace gen

} // namespace commsdsl
