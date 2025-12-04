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

#include "commsdsl/gen/GenDataField.h"

#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenDataFieldImpl
{
public:
    using ParseDataField = GenDataField::ParseDataField;

    GenDataFieldImpl(GenGenerator& generator, ParseDataField parseObj, GenElem* parent):
        m_generator(generator),
        m_dataParseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        if (!m_dataParseObj.parseHasLengthPrefixField()) {
            return true;
        }

        auto prefix = m_dataParseObj.parseLengthPrefixField();
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
    ParseDataField m_dataParseObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalPrefixField = nullptr;
    GenFieldPtr m_memberPrefixField;
};

GenDataField::GenDataField(GenGenerator& generator, ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenDataFieldImpl>(generator, genDataFieldParseObj(), this))
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Data);
}

GenDataField::~GenDataField() = default;

GenField* GenDataField::genExternalPrefixField()
{
    return m_impl->genExternalPrefixField();
}

const GenField* GenDataField::genExternalPrefixField() const
{
    return m_impl->genExternalPrefixField();
}

GenField* GenDataField::genMemberPrefixField()
{
    return m_impl->genMemberPrefixField();
}

const GenField* GenDataField::genMemberPrefixField() const
{
    return m_impl->genMemberPrefixField();
}

bool GenDataField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenDataField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenDataField::ParseDataField GenDataField::genDataFieldParseObj() const
{
    return ParseDataField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
