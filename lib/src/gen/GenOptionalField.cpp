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

#include "commsdsl/gen/GenOptionalField.h"

#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenOptionalFieldImpl
{
public:
    using ParseOptionalField = GenOptionalField::ParseOptionalField;

    GenOptionalFieldImpl(GenGenerator& generator, ParseOptionalField parseObj, GenElem* parent): 
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        auto field = m_parseObj.parseField();
        assert(field.parseValid());

        if (!field.parseExternalRef().empty()) {
            m_externalField = m_generator.genFindField(field.parseExternalRef());
            assert(m_externalField != nullptr);
            return true;
        }

        m_memberField = GenField::genCreate(m_generator, field, m_parent);
        assert(m_memberField);
        if (!m_memberField->genPrepare()) {
            return false;
        }

        return true;
    }

    GenField* genExternalField()
    {
        return m_externalField;
    }

    const GenField* genExternalField() const
    {
        return m_externalField;
    }

    GenField* genMemberField()
    {
        return m_memberField.get();
    }

    const GenField* genMemberField() const
    {
        return m_memberField.get();
    }    

    void genSetReferenced()
    {
        GenField::genSetFieldReferencedIfExists(m_externalField);
        GenField::genSetFieldReferencedIfExists(m_memberField.get());
    }

private:
    GenGenerator& m_generator;
    ParseOptionalField m_parseObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalField = nullptr;
    GenFieldPtr m_memberField;
}; 

GenOptionalField::GenOptionalField(GenGenerator& generator, ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenOptionalFieldImpl>(generator, genOptionalFieldParseObj(), this))
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Optional);
}

GenOptionalField::~GenOptionalField() = default;

GenField* GenOptionalField::genExternalField()
{
    return m_impl->genExternalField();
}

const GenField* GenOptionalField::genExternalField() const
{
    return m_impl->genExternalField();
}

GenField* GenOptionalField::genMemberField()
{
    return m_impl->genMemberField();
}

const GenField* GenOptionalField::genMemberField() const
{
    return m_impl->genMemberField();
}  

bool GenOptionalField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenOptionalField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenOptionalField::ParseOptionalField GenOptionalField::genOptionalFieldParseObj() const
{
    return ParseOptionalField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
