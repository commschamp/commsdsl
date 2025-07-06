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

#include "commsdsl/gen/GenBitfieldField.h"

#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenBitfieldFieldImpl
{
public:
    using FieldsList = GenBitfieldField::FieldsList;
    using ParseBitfieldField = GenBitfieldField::ParseBitfieldField;

    GenBitfieldFieldImpl(GenGenerator& generator, ParseBitfieldField dslObj, GenElem* parent) :
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        if (!m_dslObj.parseValid()) {
            return true;
        }

        auto fields = m_dslObj.parseMembers();
        m_members.reserve(fields.size());
        for (auto& dslObj : fields) {
            auto ptr = GenField::genCreate(m_generator, dslObj, m_parent);
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
    ParseBitfieldField m_dslObj;
    GenElem* m_parent = nullptr;
    FieldsList m_members;
};

GenBitfieldField::GenBitfieldField(GenGenerator& generator, ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenBitfieldFieldImpl>(generator, genBitfieldFieldParseObj(), this))
{
    assert(dslObj.parseKind() == ParseField::Kind::Bitfield);
}

GenBitfieldField::~GenBitfieldField() = default;

const GenBitfieldField::FieldsList& GenBitfieldField::genMembers() const
{
    return m_impl->genMembers();
}

bool GenBitfieldField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenBitfieldField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenBitfieldField::FieldRefInfo GenBitfieldField::genProcessInnerRefImpl(const std::string& refStr) const
{
    auto& memFields = genMembers();
    if (!memFields.empty()) {
        return genProcessMemberRef(memFields, refStr);
    }    

    return Base::genProcessInnerRefImpl(refStr);
}

GenBitfieldField::ParseBitfieldField GenBitfieldField::genBitfieldFieldParseObj() const
{
    return GenBitfieldField::ParseBitfieldField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
