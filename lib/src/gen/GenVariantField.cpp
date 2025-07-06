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
    using ParseVariantField = GenVariantField::ParseVariantField;

    GenVariantFieldImpl(GenGenerator& generator, ParseVariantField parseObj, GenElem* parent) :
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        if (!m_parseObj.parseValid()) {
            return true;
        }

        auto fields = m_parseObj.parseMembers();
        m_members.reserve(fields.size());
        for (auto& parseObj : fields) {
            auto ptr = GenField::genCreate(m_generator, parseObj, m_parent);
            assert(ptr);
            if (!ptr->genPrepare()) {
                return false;
            }

            m_members.push_back(std::move(ptr));
        }

        return true;
    }

    const FieldsList& genMembers() const
    {
        return m_members;
    }

    void genSetReferenced()
    {
        for (auto& m : m_members) {
            m->genSetReferenced();
        }
    }    

private:
    GenGenerator& m_generator;
    ParseVariantField m_parseObj;
    GenElem* m_parent = nullptr;
    FieldsList m_members;
};

GenVariantField::GenVariantField(GenGenerator& generator, ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenVariantFieldImpl>(generator, genVariantFieldParseObj(), this))
{
    assert(parseObj.parseKind() == ParseField::Kind::Variant);
}

GenVariantField::~GenVariantField() = default;

const GenVariantField::FieldsList& GenVariantField::genMembers() const
{
    return m_impl->genMembers();
}

bool GenVariantField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenVariantField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenVariantField::FieldRefInfo GenVariantField::genProcessInnerRefImpl(const std::string& refStr) const
{
    auto& memFields = genMembers();
    if (!memFields.empty()) {
        return genProcessMemberRef(memFields, refStr);
    }    

    return Base::genProcessInnerRefImpl(refStr);
}

GenVariantField::ParseVariantField GenVariantField::genVariantFieldParseObj() const
{
    return ParseVariantField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
