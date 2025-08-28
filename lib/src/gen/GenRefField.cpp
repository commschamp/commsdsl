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

#include "commsdsl/gen/GenRefField.h"

#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

class GenRefFieldImpl
{
public:
    using ParseRefField = GenRefField::ParseRefField;

    GenRefFieldImpl(GenGenerator& generator, ParseRefField parseObj): 
        m_generator(generator),
        m_parseObj(parseObj)
    {
    }

    bool genPrepare()
    {
        auto fieldObj = m_parseObj.parseField();
        if (fieldObj.parseIsPseudo() != m_parseObj.parseIsPseudo()) {
            m_generator.genLogger().genError(
                m_parseObj.parseSchemaPos() +
                "Having \"pseudo\" property value for <ref> field \"" + m_parseObj.parseName() +
                "\" that differs to one of the referenced field is not supported by the code generator.");
            return false;
        }

        if (fieldObj.parseIsFailOnInvalid() != m_parseObj.parseIsFailOnInvalid()) {
            m_generator.genLogger().genError(
                m_parseObj.parseSchemaPos() +
                "Having \"failOnInvalid\" property value for <ref> field \"" + m_parseObj.parseName() +
                "\" that differs to one of the referenced field is not supported by the code generator.");
            return false;
        }

        m_referencedField = m_generator.genFindField(fieldObj.parseExternalRef());
        if (m_referencedField == nullptr) {
            assert(false);
            return false;
        }

        return true;
    }

    GenField* genReferencedField()
    {
        return m_referencedField;
    }

    const GenField* genReferencedField() const
    {
        return m_referencedField;
    }

    void genSetReferenced()
    {
        assert(m_referencedField != nullptr);
        m_referencedField->genSetReferenced();
    }

private:
    GenGenerator& m_generator;
    ParseRefField m_parseObj;
    GenField* m_referencedField = nullptr;
};       

GenRefField::GenRefField(GenGenerator& generator, ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenRefFieldImpl>(generator, genRefFieldParseObj()))
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Ref);
}

GenRefField::~GenRefField() = default;

GenField* GenRefField::genReferencedField()
{
    return m_impl->genReferencedField();
}

const GenField* GenRefField::genReferencedField() const
{
    return m_impl->genReferencedField();
}

bool GenRefField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenRefField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenRefField::GenFieldRefInfo GenRefField::genProcessInnerRefImpl(const std::string& refStr) const
{
    auto* field = genReferencedField();
    assert(field != nullptr);
    return field->genProcessInnerRef(refStr);
}

GenRefField::ParseRefField GenRefField::genRefFieldParseObj() const
{
    return ParseRefField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
