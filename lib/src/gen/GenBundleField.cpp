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
    using ParseBundleField = GenBundleField::ParseBundleField;

    GenBundleFieldImpl(GenGenerator& generator, ParseBundleField parseBundleObj, GenElem* parent) :
        m_generator(generator),
        m_bundleParseObj(parseBundleObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        if (!m_bundleParseObj.parseValid()) {
            return true;
        }

        auto fields = m_bundleParseObj.parseMembers();
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
    ParseBundleField m_bundleParseObj;
    GenElem* m_parent = nullptr;
    FieldsList m_members;
};

GenBundleField::GenBundleField(GenGenerator& generator, commsdsl::parse::ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenBundleFieldImpl>(generator, genBundleFieldParseObj(), this))
{
    assert(parseObj.parseKind() == commsdsl::parse::ParseField::Kind::Bundle);
}

GenBundleField::~GenBundleField() = default;

const GenBundleField::FieldsList& GenBundleField::genMembers() const
{
    return m_impl->genMembers();
}

bool GenBundleField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenBundleField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenBundleField::FieldRefInfo GenBundleField::genProcessInnerRefImpl(const std::string& refStr) const
{
    auto& memFields = genMembers();
    if (!memFields.empty()) {
        return genProcessMemberRef(memFields, refStr);
    }    

    return Base::genProcessInnerRefImpl(refStr);
}

GenBundleField::ParseBundleField GenBundleField::genBundleFieldParseObj() const
{
    return ParseBundleField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
