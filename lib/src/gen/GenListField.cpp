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

#include "commsdsl/gen/GenListField.h"

#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/comms.h"

#include <cassert>


namespace commsdsl
{

namespace gen
{

class GenListFieldImpl
{
public:
    using ParseListField = GenListField::ParseListField;

    GenListFieldImpl(GenGenerator& generator, ParseListField parseObj, GenElem* parent): 
        m_generator(generator),
        m_parseObj(parseObj),
        m_parent(parent)
    {
    }

    bool genPrepare()
    {
        do {
            auto elementField = m_parseObj.parseElementField();
            assert(elementField.parseValid());
            if (!elementField.parseExternalRef().empty()) {
                m_externalElementField = m_generator.genFindField(elementField.parseExternalRef());
                assert(m_externalElementField != nullptr);
                break;
            }

            m_memberElementField = GenField::genCreate(m_generator, elementField, m_parent);
            if (!m_memberElementField->genPrepare()) {
                return false;
            }
        } while (false);

        do {
            if (!m_parseObj.parseHasCountPrefixField()) {
                break;
            }

            auto prefix = m_parseObj.parseCountPrefixField();
            if (!prefix.parseExternalRef().empty()) {
                m_externalCountPrefixField = m_generator.genFindField(prefix.parseExternalRef());
                assert(m_externalCountPrefixField != nullptr);
                break;
            }

            m_memberCountPrefixField = GenField::genCreate(m_generator, prefix, m_parent);
            if (!m_memberCountPrefixField->genPrepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::genClassName(m_memberElementField->genName()) == comms::genClassName(m_memberCountPrefixField->genName()))) {
                m_generator.genLogger().genError("Count prefix and element fields of \"" + m_parseObj.parseName() + "\" list must have different names.");
                return false;
            }

        } while (false);   

        do {
            if (!m_parseObj.parseHasLengthPrefixField()) {
                break;
            }

            assert(!m_memberCountPrefixField);
            assert(!m_parseObj.parseHasCountPrefixField());
            auto prefix = m_parseObj.parseLengthPrefixField();
            if (!prefix.parseExternalRef().empty()) {
                m_externalLengthPrefixField = m_generator.genFindField(prefix.parseExternalRef());
                assert(m_externalLengthPrefixField != nullptr);
                break;
            }

            m_memberLengthPrefixField = GenField::genCreate(m_generator, prefix, m_parent);
            if (!m_memberLengthPrefixField->genPrepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::genClassName(m_memberElementField->genName()) == comms::genClassName(m_memberLengthPrefixField->genName()))) {
                m_generator.genLogger().genError("Length prefix and element fields of \"" + m_parseObj.parseName() + "\" list must have different names.");
                return false;
            }            
        } while (false);  

        do {
            if (!m_parseObj.parseHasElemLengthPrefixField()) {
                break;
            }

            auto prefix = m_parseObj.parseElemLengthPrefixField();
            if (!prefix.parseExternalRef().empty()) {
                m_externalElemLengthPrefixField = m_generator.genFindField(prefix.parseExternalRef());
                assert(m_externalElemLengthPrefixField != nullptr);
                break;
            }

            m_memberElemLengthPrefixField = GenField::genCreate(m_generator, prefix, m_parent);
            if (!m_memberElemLengthPrefixField->genPrepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::genClassName(m_memberElementField->genName()) == comms::genClassName(m_memberElemLengthPrefixField->genName()))) {
                m_generator.genLogger().genError("Element length prefix and element fields of \"" + m_parseObj.parseName() + "\" list must have different names.");
                return false;
            }  

            if ((m_memberCountPrefixField) &&
                (comms::genClassName(m_memberCountPrefixField->genName()) == comms::genClassName(m_memberElemLengthPrefixField->genName()))) {
                m_generator.genLogger().genError("Element length prefix and count prefix fields of \"" + m_parseObj.parseName() + "\" list must have different names.");
                return false;
            } 

            if ((m_memberLengthPrefixField) &&
                (comms::genClassName(m_memberLengthPrefixField->genName()) == comms::genClassName(m_memberElemLengthPrefixField->genName()))) {
                m_generator.genLogger().genError("Element length prefix and list length prefix fields of \"" + m_parseObj.parseName() + "\" list must have different names.");
                return false;
            }                                

        } while (false);   

        do {
            if (!m_parseObj.parseHasTermSuffixField()) {
                break;
            }

            assert(!m_memberCountPrefixField);
            assert(!m_memberLengthPrefixField);
            assert(!m_parseObj.parseHasCountPrefixField());
            assert(!m_parseObj.parseHasLengthPrefixField());
            auto suffix = m_parseObj.parseTermSuffixField();
            if (!suffix.parseExternalRef().empty()) {
                m_externalTermSuffixField = m_generator.genFindField(suffix.parseExternalRef());
                assert(m_externalTermSuffixField != nullptr);
                break;
            }

            m_memberTermSuffixField = GenField::genCreate(m_generator, suffix, m_parent);
            if (!m_memberTermSuffixField->genPrepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::genClassName(m_memberElementField->genName()) == comms::genClassName(m_memberTermSuffixField->genName()))) {
                m_generator.genLogger().genError("Termination suffix and element fields of \"" + m_parseObj.parseName() + "\" list must have different names.");
                return false;
            }            
        } while (false);                          

        return true;
    }

    GenField* genExternalElementField()
    {
        return m_externalElementField;
    }

    const GenField* genExternalElementField() const
    {
        return m_externalElementField;
    }

    GenField* genMemberElementField()
    {
        return m_memberElementField.get();
    }

    const GenField* genMemberElementField() const
    {
        return m_memberElementField.get();
    }

    GenField* genExternalCountPrefixField()
    {
        return m_externalCountPrefixField;
    }

    const GenField* genExternalCountPrefixField() const
    {
        return m_externalCountPrefixField;
    }

    GenField* genMemberCountPrefixField()
    {
        return m_memberCountPrefixField.get();
    }

    const GenField* genMemberCountPrefixField() const
    {
        return m_memberCountPrefixField.get();
    }

    GenField* genExternalLengthPrefixField()
    {
        return m_externalLengthPrefixField;
    }

    const GenField* genExternalLengthPrefixField() const
    {
        return m_externalLengthPrefixField;
    }

    GenField* genMemberLengthPrefixField()
    {
        return m_memberLengthPrefixField.get();
    }

    const GenField* genMemberLengthPrefixField() const
    {
        return m_memberLengthPrefixField.get();
    } 

    GenField* genExternalElemLengthPrefixField()
    {
        return m_externalElemLengthPrefixField;
    }

    const GenField* genExternalElemLengthPrefixField() const
    {
        return m_externalElemLengthPrefixField;
    }

    GenField* genMemberElemLengthPrefixField()
    {
        return m_memberElemLengthPrefixField.get();
    }

    const GenField* genMemberElemLengthPrefixField() const
    {
        return m_memberElemLengthPrefixField.get();
    }     

    GenField* genExternalTermSuffixField()
    {
        return m_externalTermSuffixField;
    }

    const GenField* genExternalTermSuffixField() const
    {
        return m_externalTermSuffixField;
    }             

    GenField* genMemberTermSuffixField()
    {
        return m_memberTermSuffixField.get();
    }

    const GenField* genMemberTermSuffixField() const
    {
        return m_memberTermSuffixField.get();
    }   

    void genSetReferenced()
    {
        GenField::genSetFieldReferencedIfExists(m_externalElementField);
        GenField::genSetFieldReferencedIfExists(m_memberElementField.get());
        GenField::genSetFieldReferencedIfExists(m_externalCountPrefixField);
        GenField::genSetFieldReferencedIfExists(m_memberCountPrefixField.get());
        GenField::genSetFieldReferencedIfExists(m_externalLengthPrefixField);
        GenField::genSetFieldReferencedIfExists(m_memberLengthPrefixField.get());    
        GenField::genSetFieldReferencedIfExists(m_externalElemLengthPrefixField);
        GenField::genSetFieldReferencedIfExists(m_memberElemLengthPrefixField.get());   
        GenField::genSetFieldReferencedIfExists(m_externalTermSuffixField);
        GenField::genSetFieldReferencedIfExists(m_memberTermSuffixField.get());                     
    }

private:
    GenGenerator& m_generator;
    ParseListField m_parseObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalElementField = nullptr;
    GenFieldPtr m_memberElementField;
    GenField* m_externalCountPrefixField = nullptr;
    GenFieldPtr m_memberCountPrefixField;    
    GenField* m_externalLengthPrefixField = nullptr;
    GenFieldPtr m_memberLengthPrefixField;   
    GenField* m_externalElemLengthPrefixField = nullptr;
    GenFieldPtr m_memberElemLengthPrefixField;        
    GenField* m_externalTermSuffixField = nullptr;
    GenFieldPtr m_memberTermSuffixField = nullptr;
};    

GenListField::GenListField(GenGenerator& generator, ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenListFieldImpl>(generator, genListFieldParseObj(), this))
{
    assert(parseObj.parseKind() == commsdsl::parse::ParseField::ParseKind::List);
}

GenListField::~GenListField() = default;

GenField* GenListField::genExternalElementField()
{
    return m_impl->genExternalElementField();
}

const GenField* GenListField::genExternalElementField() const
{
    return m_impl->genExternalElementField();
}

GenField* GenListField::genMemberElementField()
{
    return m_impl->genMemberElementField();
}

const GenField* GenListField::genMemberElementField() const
{
    return m_impl->genMemberElementField();
}

GenField* GenListField::genExternalCountPrefixField()
{
    return m_impl->genExternalCountPrefixField();
}

const GenField* GenListField::genExternalCountPrefixField() const
{
    return m_impl->genExternalCountPrefixField();
}

GenField* GenListField::genMemberCountPrefixField()
{
    return m_impl->genMemberCountPrefixField();
}

const GenField* GenListField::genMemberCountPrefixField() const
{
    return m_impl->genMemberCountPrefixField();
}

GenField* GenListField::genExternalLengthPrefixField()
{
    return m_impl->genExternalLengthPrefixField();
}

const GenField* GenListField::genExternalLengthPrefixField() const
{
    return m_impl->genExternalLengthPrefixField();
}

GenField* GenListField::genMemberLengthPrefixField()
{
    return m_impl->genMemberLengthPrefixField();
}

const GenField* GenListField::genMemberLengthPrefixField() const
{
    return m_impl->genMemberLengthPrefixField();
} 

GenField* GenListField::genExternalElemLengthPrefixField()
{
    return m_impl->genExternalElemLengthPrefixField();
}

const GenField* GenListField::genExternalElemLengthPrefixField() const
{
    return m_impl->genExternalElemLengthPrefixField();
}

GenField* GenListField::genMemberElemLengthPrefixField()
{
    return m_impl->genMemberElemLengthPrefixField();
}

const GenField* GenListField::genMemberElemLengthPrefixField() const
{
    return m_impl->genMemberElemLengthPrefixField();
}

GenField* GenListField::genExternalTermSuffixField()
{
    return m_impl->genExternalTermSuffixField();
}

const GenField* GenListField::genExternalTermSuffixField() const
{
    return m_impl->genExternalTermSuffixField();
}

GenField* GenListField::genMemberTermSuffixField()
{
    return m_impl->genMemberTermSuffixField();
}

const GenField* GenListField::genMemberTermSuffixField() const
{
    return m_impl->genMemberTermSuffixField();
}

bool GenListField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

void GenListField::genSetReferencedImpl()
{
    m_impl->genSetReferenced();
}

GenListField::ParseListField GenListField::genListFieldParseObj() const
{
    return ParseListField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
