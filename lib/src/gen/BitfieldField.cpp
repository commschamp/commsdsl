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

#include "commsdsl/gen/BitfieldField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class BitfieldFieldImpl
{
public:
    using FieldsList = BitfieldField::FieldsList;

    BitfieldFieldImpl(Generator& generator, commsdsl::parse::BitfieldField dslObj, Elem* parent) :
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
            auto ptr = Field::create(m_generator, dslObj, m_parent);
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
    Generator& m_generator;
    commsdsl::parse::BitfieldField m_dslObj;
    Elem* m_parent = nullptr;
    FieldsList m_members;
};

BitfieldField::BitfieldField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<BitfieldFieldImpl>(generator, bitfieldDslObj(), this))
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Bitfield);
}

BitfieldField::~BitfieldField() = default;

const BitfieldField::FieldsList& BitfieldField::members() const
{
    return m_impl->members();
}

bool BitfieldField::prepareImpl()
{
    return m_impl->prepare();
}

void BitfieldField::setReferencedImpl()
{
    m_impl->setReferenced();
}

BitfieldField::FieldRefInfo BitfieldField::processInnerRefImpl(const std::string& refStr) const
{
    auto& memFields = members();
    if (!memFields.empty()) {
        return processMemberRef(memFields, refStr);
    }    

    return Base::processInnerRefImpl(refStr);
}


commsdsl::parse::BitfieldField BitfieldField::bitfieldDslObj() const
{
    return commsdsl::parse::BitfieldField(dslObj());
}

} // namespace gen

} // namespace commsdsl
