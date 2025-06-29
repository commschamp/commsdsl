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

#include "commsdsl/gen/GenStringField.h"
#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenStringFieldImpl
{
    using Base = GenField;
public:
    using ParseStringField = GenStringField::ParseStringField;

    GenStringFieldImpl(GenGenerator& generator, ParseStringField parseObj, GenElem* parent): 
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        if (!m_parseObj.parseHasLengthPrefixField()) {
            return true;
        }

        auto prefix = m_parseObj.parseLengthPrefixField();
        if (!prefix.parseExternalRef().empty()) {
            m_externalPrefixField = m_generator.genFindField(prefix.parseExternalRef());
            assert(m_externalPrefixField != nullptr);
            return true;
        }

        m_memberPrefixField = GenField::genCreate(m_generator, prefix, m_parent);
        if (!m_memberPrefixField->genPrepare()) {
            return false;
        }

        return true;
    }

    GenField* genExternalPrefixField()
    {
        return m_externalPrefixField;
    }

    const GenField* genExternalPrefixField() const
    {
        return m_externalPrefixField;
    }

    GenField* genMemberPrefixField()
    {
        return m_memberPrefixField.get();
    }

    const GenField* genMemberPrefixField() const
    {
        return m_memberPrefixField.get();
    }    

    void genSetReferenced()
    {
        GenField::genSetFieldReferencedIfExists(m_externalPrefixField);
        GenField::genSetFieldReferencedIfExists(m_memberPrefixField.get());
    }

private:
    GenGenerator& m_generator;
    ParseStringField m_parseObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalPrefixField = nullptr;
    GenFieldPtr m_memberPrefixField;
};


GenStringField::GenStringField(GenGenerator& generator, ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenStringFieldImpl>(generator, genStringFieldParseObj(), this))
{
    assert(parseObj.parseKind() == ParseField::Kind::String);
}

GenStringField::~GenStringField() = default;

GenField* GenStringField::genExternalPrefixField()
{
    return m_impl->genExternalPrefixField();
}

const GenField* GenStringField::genExternalPrefixField() const
{
    return m_impl->genExternalPrefixField();
}

GenField* GenStringField::genMemberPrefixField()
{
    return m_impl->genMemberPrefixField();
}

const GenField* GenStringField::genMemberPrefixField() const
{
    return m_impl->genMemberPrefixField();
}

bool GenStringField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenStringField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenStringField::ParseStringField GenStringField::genStringFieldParseObj() const
{
    return ParseStringField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
