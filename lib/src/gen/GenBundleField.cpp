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

#include "commsdsl/gen/GenBundleField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenBundleFieldImpl
{
public:
    using FieldsList = GenBundleField::FieldsList;

    GenBundleFieldImpl(GenGenerator& generator, commsdsl::parse::ParseBundleField dslObj, GenElem* parent) :
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

        auto fields = m_dslObj.members();
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
    commsdsl::parse::ParseBundleField m_dslObj;
    GenElem* m_parent = nullptr;
    FieldsList m_members;
};

GenBundleField::GenBundleField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenBundleFieldImpl>(generator, bundleDslObj(), this))
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Bundle);
}

GenBundleField::~GenBundleField() = default;

const GenBundleField::FieldsList& GenBundleField::members() const
{
    return m_impl->members();
}

bool GenBundleField::prepareImpl()
{
    return m_impl->prepare();
}

void GenBundleField::setReferencedImpl()
{
    m_impl->setReferenced();
}

GenBundleField::FieldRefInfo GenBundleField::processInnerRefImpl(const std::string& refStr) const
{
    auto& memFields = members();
    if (!memFields.empty()) {
        return processMemberRef(memFields, refStr);
    }    

    return Base::processInnerRefImpl(refStr);
}

commsdsl::parse::ParseBundleField GenBundleField::bundleDslObj() const
{
    return commsdsl::parse::ParseBundleField(dslObj());
}

} // namespace gen

} // namespace commsdsl
