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

#include <cassert>

#include "commsdsl/gen/comms.h"

namespace commsdsl
{

namespace gen
{

class GenListFieldImpl
{
public:
    GenListFieldImpl(GenGenerator& generator, commsdsl::parse::ParseListField dslObj, GenElem* parent): 
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        do {
            auto elementField = m_dslObj.parseElementField();
            assert(elementField.parseValid());
            if (!elementField.parseExternalRef().empty()) {
                m_externalElementField = m_generator.findField(elementField.parseExternalRef());
                assert(m_externalElementField != nullptr);
                break;
            }

            m_memberElementField = GenField::create(m_generator, elementField, m_parent);
            if (!m_memberElementField->prepare()) {
                return false;
            }
        } while (false);

        do {
            if (!m_dslObj.parseHasCountPrefixField()) {
                break;
            }

            auto prefix = m_dslObj.parseCountPrefixField();
            if (!prefix.parseExternalRef().empty()) {
                m_externalCountPrefixField = m_generator.findField(prefix.parseExternalRef());
                assert(m_externalCountPrefixField != nullptr);
                break;
            }

            m_memberCountPrefixField = GenField::create(m_generator, prefix, m_parent);
            if (!m_memberCountPrefixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberCountPrefixField->name()))) {
                m_generator.logger().error("Count prefix and element fields of \"" + m_dslObj.parseName() + "\" list must have different names.");
                return false;
            }

        } while (false);   

        do {
            if (!m_dslObj.parseHasLengthPrefixField()) {
                break;
            }

            assert(!m_memberCountPrefixField);
            assert(!m_dslObj.parseHasCountPrefixField());
            auto prefix = m_dslObj.parseLengthPrefixField();
            if (!prefix.parseExternalRef().empty()) {
                m_externalLengthPrefixField = m_generator.findField(prefix.parseExternalRef());
                assert(m_externalLengthPrefixField != nullptr);
                break;
            }

            m_memberLengthPrefixField = GenField::create(m_generator, prefix, m_parent);
            if (!m_memberLengthPrefixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberLengthPrefixField->name()))) {
                m_generator.logger().error("Length prefix and element fields of \"" + m_dslObj.parseName() + "\" list must have different names.");
                return false;
            }            
        } while (false);  

        do {
            if (!m_dslObj.parseHasElemLengthPrefixField()) {
                break;
            }

            auto prefix = m_dslObj.parseElemLengthPrefixField();
            if (!prefix.parseExternalRef().empty()) {
                m_externalElemLengthPrefixField = m_generator.findField(prefix.parseExternalRef());
                assert(m_externalElemLengthPrefixField != nullptr);
                break;
            }

            m_memberElemLengthPrefixField = GenField::create(m_generator, prefix, m_parent);
            if (!m_memberElemLengthPrefixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberElemLengthPrefixField->name()))) {
                m_generator.logger().error("Element length prefix and element fields of \"" + m_dslObj.parseName() + "\" list must have different names.");
                return false;
            }  

            if ((m_memberCountPrefixField) &&
                (comms::className(m_memberCountPrefixField->name()) == comms::className(m_memberElemLengthPrefixField->name()))) {
                m_generator.logger().error("Element length prefix and count prefix fields of \"" + m_dslObj.parseName() + "\" list must have different names.");
                return false;
            } 

            if ((m_memberLengthPrefixField) &&
                (comms::className(m_memberLengthPrefixField->name()) == comms::className(m_memberElemLengthPrefixField->name()))) {
                m_generator.logger().error("Element length prefix and list length prefix fields of \"" + m_dslObj.parseName() + "\" list must have different names.");
                return false;
            }                                

        } while (false);   

        do {
            if (!m_dslObj.parseHasTermSuffixField()) {
                break;
            }

            assert(!m_memberCountPrefixField);
            assert(!m_memberLengthPrefixField);
            assert(!m_dslObj.parseHasCountPrefixField());
            assert(!m_dslObj.parseHasLengthPrefixField());
            auto suffix = m_dslObj.parseTermSuffixField();
            if (!suffix.parseExternalRef().empty()) {
                m_externalTermSuffixField = m_generator.findField(suffix.parseExternalRef());
                assert(m_externalTermSuffixField != nullptr);
                break;
            }

            m_memberTermSuffixField = GenField::create(m_generator, suffix, m_parent);
            if (!m_memberTermSuffixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberTermSuffixField->name()))) {
                m_generator.logger().error("Termination suffix and element fields of \"" + m_dslObj.parseName() + "\" list must have different names.");
                return false;
            }            
        } while (false);                          

        return true;
    }

    GenField* externalElementField()
    {
        return m_externalElementField;
    }

    const GenField* externalElementField() const
    {
        return m_externalElementField;
    }

    GenField* memberElementField()
    {
        return m_memberElementField.get();
    }

    const GenField* memberElementField() const
    {
        return m_memberElementField.get();
    }

    GenField* externalCountPrefixField()
    {
        return m_externalCountPrefixField;
    }

    const GenField* externalCountPrefixField() const
    {
        return m_externalCountPrefixField;
    }

    GenField* memberCountPrefixField()
    {
        return m_memberCountPrefixField.get();
    }

    const GenField* memberCountPrefixField() const
    {
        return m_memberCountPrefixField.get();
    }

    GenField* externalLengthPrefixField()
    {
        return m_externalLengthPrefixField;
    }

    const GenField* externalLengthPrefixField() const
    {
        return m_externalLengthPrefixField;
    }

    GenField* memberLengthPrefixField()
    {
        return m_memberLengthPrefixField.get();
    }

    const GenField* memberLengthPrefixField() const
    {
        return m_memberLengthPrefixField.get();
    } 

    GenField* externalElemLengthPrefixField()
    {
        return m_externalElemLengthPrefixField;
    }

    const GenField* externalElemLengthPrefixField() const
    {
        return m_externalElemLengthPrefixField;
    }

    GenField* memberElemLengthPrefixField()
    {
        return m_memberElemLengthPrefixField.get();
    }

    const GenField* memberElemLengthPrefixField() const
    {
        return m_memberElemLengthPrefixField.get();
    }     

    GenField* externalTermSuffixField()
    {
        return m_externalTermSuffixField;
    }

    const GenField* externalTermSuffixField() const
    {
        return m_externalTermSuffixField;
    }             

    GenField* memberTermSuffixField()
    {
        return m_memberTermSuffixField.get();
    }

    const GenField* memberTermSuffixField() const
    {
        return m_memberTermSuffixField.get();
    }   

    void setReferenced()
    {
        GenField::setFieldReferencedIfExists(m_externalElementField);
        GenField::setFieldReferencedIfExists(m_memberElementField.get());
        GenField::setFieldReferencedIfExists(m_externalCountPrefixField);
        GenField::setFieldReferencedIfExists(m_memberCountPrefixField.get());
        GenField::setFieldReferencedIfExists(m_externalLengthPrefixField);
        GenField::setFieldReferencedIfExists(m_memberLengthPrefixField.get());    
        GenField::setFieldReferencedIfExists(m_externalElemLengthPrefixField);
        GenField::setFieldReferencedIfExists(m_memberElemLengthPrefixField.get());   
        GenField::setFieldReferencedIfExists(m_externalTermSuffixField);
        GenField::setFieldReferencedIfExists(m_memberTermSuffixField.get());                     
    }

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseListField m_dslObj;
    GenElem* m_parent = nullptr;
    GenField* m_externalElementField = nullptr;
    FieldPtr m_memberElementField;
    GenField* m_externalCountPrefixField = nullptr;
    FieldPtr m_memberCountPrefixField;    
    GenField* m_externalLengthPrefixField = nullptr;
    FieldPtr m_memberLengthPrefixField;   
    GenField* m_externalElemLengthPrefixField = nullptr;
    FieldPtr m_memberElemLengthPrefixField;        
    GenField* m_externalTermSuffixField = nullptr;
    FieldPtr m_memberTermSuffixField = nullptr;
};    

GenListField::GenListField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenListFieldImpl>(generator, listDslObj(), this))
{
    assert(dslObj.parseKind() == commsdsl::parse::ParseField::Kind::List);
}

GenListField::~GenListField() = default;

GenField* GenListField::externalElementField()
{
    return m_impl->externalElementField();
}

const GenField* GenListField::externalElementField() const
{
    return m_impl->externalElementField();
}

GenField* GenListField::memberElementField()
{
    return m_impl->memberElementField();
}

const GenField* GenListField::memberElementField() const
{
    return m_impl->memberElementField();
}

GenField* GenListField::externalCountPrefixField()
{
    return m_impl->externalCountPrefixField();
}

const GenField* GenListField::externalCountPrefixField() const
{
    return m_impl->externalCountPrefixField();
}

GenField* GenListField::memberCountPrefixField()
{
    return m_impl->memberCountPrefixField();
}

const GenField* GenListField::memberCountPrefixField() const
{
    return m_impl->memberCountPrefixField();
}

GenField* GenListField::externalLengthPrefixField()
{
    return m_impl->externalLengthPrefixField();
}

const GenField* GenListField::externalLengthPrefixField() const
{
    return m_impl->externalLengthPrefixField();
}

GenField* GenListField::memberLengthPrefixField()
{
    return m_impl->memberLengthPrefixField();
}

const GenField* GenListField::memberLengthPrefixField() const
{
    return m_impl->memberLengthPrefixField();
} 

GenField* GenListField::externalElemLengthPrefixField()
{
    return m_impl->externalElemLengthPrefixField();
}

const GenField* GenListField::externalElemLengthPrefixField() const
{
    return m_impl->externalElemLengthPrefixField();
}

GenField* GenListField::memberElemLengthPrefixField()
{
    return m_impl->memberElemLengthPrefixField();
}

const GenField* GenListField::memberElemLengthPrefixField() const
{
    return m_impl->memberElemLengthPrefixField();
}

GenField* GenListField::externalTermSuffixField()
{
    return m_impl->externalTermSuffixField();
}

const GenField* GenListField::externalTermSuffixField() const
{
    return m_impl->externalTermSuffixField();
}

GenField* GenListField::memberTermSuffixField()
{
    return m_impl->memberTermSuffixField();
}

const GenField* GenListField::memberTermSuffixField() const
{
    return m_impl->memberTermSuffixField();
}

bool GenListField::prepareImpl()
{
    return m_impl->prepare();
}

void GenListField::setReferencedImpl()
{
    m_impl->setReferenced();
}

commsdsl::parse::ParseListField GenListField::listDslObj() const
{
    return commsdsl::parse::ParseListField(dslObj());
}

} // namespace gen

} // namespace commsdsl
