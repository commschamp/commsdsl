//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/ListField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>

#include "commsdsl/gen/comms.h"

namespace commsdsl
{

namespace gen
{

class ListFieldImpl
{
public:
    ListFieldImpl(Generator& generator, commsdsl::parse::ListField dslObj, Elem* parent): 
        m_generator(generator),
        m_dslObj(dslObj),
        m_parent(parent)
    {
    }

    bool prepare()
    {
        do {
            auto elementField = m_dslObj.elementField();
            assert(elementField.valid());
            if (!elementField.externalRef().empty()) {
                m_externalElementField = m_generator.findField(elementField.externalRef());
                assert(m_externalElementField != nullptr);
                break;
            }

            m_memberElementField = Field::create(m_generator, elementField, m_parent);
            if (!m_memberElementField->prepare()) {
                return false;
            }
        } while (false);

        do {
            if (!m_dslObj.hasCountPrefixField()) {
                break;
            }

            auto prefix = m_dslObj.countPrefixField();
            if (!prefix.externalRef().empty()) {
                m_externalCountPrefixField = m_generator.findField(prefix.externalRef());
                assert(m_externalCountPrefixField != nullptr);
                break;
            }

            m_memberCountPrefixField = Field::create(m_generator, prefix, m_parent);
            if (!m_memberCountPrefixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberCountPrefixField->name()))) {
                m_generator.logger().error("Count prefix and element fields of \"" + m_dslObj.name() + "\" list must have different names.");
                return false;
            }

        } while (false);   

        do {
            if (!m_dslObj.hasLengthPrefixField()) {
                break;
            }

            assert(!m_memberCountPrefixField);
            assert(!m_dslObj.hasCountPrefixField());
            auto prefix = m_dslObj.lengthPrefixField();
            if (!prefix.externalRef().empty()) {
                m_externalLengthPrefixField = m_generator.findField(prefix.externalRef());
                assert(m_externalLengthPrefixField != nullptr);
                break;
            }

            m_memberLengthPrefixField = Field::create(m_generator, prefix, m_parent);
            if (!m_memberLengthPrefixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberLengthPrefixField->name()))) {
                m_generator.logger().error("Length prefix and element fields of \"" + m_dslObj.name() + "\" list must have different names.");
                return false;
            }            
        } while (false);  

        do {
            if (!m_dslObj.hasElemLengthPrefixField()) {
                break;
            }

            auto prefix = m_dslObj.elemLengthPrefixField();
            if (!prefix.externalRef().empty()) {
                m_externalElemLengthPrefixField = m_generator.findField(prefix.externalRef());
                assert(m_externalElemLengthPrefixField != nullptr);
                break;
            }

            m_memberElemLengthPrefixField = Field::create(m_generator, prefix, m_parent);
            if (!m_memberElemLengthPrefixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberElemLengthPrefixField->name()))) {
                m_generator.logger().error("Element length prefix and element fields of \"" + m_dslObj.name() + "\" list must have different names.");
                return false;
            }  

            if ((m_memberCountPrefixField) &&
                (comms::className(m_memberCountPrefixField->name()) == comms::className(m_memberElemLengthPrefixField->name()))) {
                m_generator.logger().error("Element length prefix and count prefix fields of \"" + m_dslObj.name() + "\" list must have different names.");
                return false;
            } 

            if ((m_memberLengthPrefixField) &&
                (comms::className(m_memberLengthPrefixField->name()) == comms::className(m_memberElemLengthPrefixField->name()))) {
                m_generator.logger().error("Element length prefix and list length prefix fields of \"" + m_dslObj.name() + "\" list must have different names.");
                return false;
            }                                

        } while (false);   

        do {
            if (!m_dslObj.hasTermSuffixField()) {
                break;
            }

            assert(!m_memberCountPrefixField);
            assert(!m_memberLengthPrefixField);
            assert(!m_dslObj.hasCountPrefixField());
            assert(!m_dslObj.hasLengthPrefixField());
            auto suffix = m_dslObj.termSuffixField();
            if (!suffix.externalRef().empty()) {
                m_externalTermSuffixField = m_generator.findField(suffix.externalRef());
                assert(m_externalTermSuffixField != nullptr);
                break;
            }

            m_memberTermSuffixField = Field::create(m_generator, suffix, m_parent);
            if (!m_memberTermSuffixField->prepare()) {
                return false;
            }

            if ((m_memberElementField) &&
                (comms::className(m_memberElementField->name()) == comms::className(m_memberTermSuffixField->name()))) {
                m_generator.logger().error("Termination suffix and element fields of \"" + m_dslObj.name() + "\" list must have different names.");
                return false;
            }            
        } while (false);                          

        return true;
    }

    Field* externalElementField()
    {
        return m_externalElementField;
    }

    const Field* externalElementField() const
    {
        return m_externalElementField;
    }

    Field* memberElementField()
    {
        return m_memberElementField.get();
    }

    const Field* memberElementField() const
    {
        return m_memberElementField.get();
    }

    Field* externalCountPrefixField()
    {
        return m_externalCountPrefixField;
    }

    const Field* externalCountPrefixField() const
    {
        return m_externalCountPrefixField;
    }

    Field* memberCountPrefixField()
    {
        return m_memberCountPrefixField.get();
    }

    const Field* memberCountPrefixField() const
    {
        return m_memberCountPrefixField.get();
    }

    Field* externalLengthPrefixField()
    {
        return m_externalLengthPrefixField;
    }

    const Field* externalLengthPrefixField() const
    {
        return m_externalLengthPrefixField;
    }

    Field* memberLengthPrefixField()
    {
        return m_memberLengthPrefixField.get();
    }

    const Field* memberLengthPrefixField() const
    {
        return m_memberLengthPrefixField.get();
    } 

    Field* externalElemLengthPrefixField()
    {
        return m_externalElemLengthPrefixField;
    }

    const Field* externalElemLengthPrefixField() const
    {
        return m_externalElemLengthPrefixField;
    }

    Field* memberElemLengthPrefixField()
    {
        return m_memberElemLengthPrefixField.get();
    }

    const Field* memberElemLengthPrefixField() const
    {
        return m_memberElemLengthPrefixField.get();
    }     

    Field* externalTermSuffixField()
    {
        return m_externalTermSuffixField;
    }

    const Field* externalTermSuffixField() const
    {
        return m_externalTermSuffixField;
    }             

    Field* memberTermSuffixField()
    {
        return m_memberTermSuffixField.get();
    }

    const Field* memberTermSuffixField() const
    {
        return m_memberTermSuffixField.get();
    }   

    void setReferenced()
    {
        Field::setFieldReferencedIfExists(m_externalElementField);
        Field::setFieldReferencedIfExists(m_memberElementField.get());
        Field::setFieldReferencedIfExists(m_externalCountPrefixField);
        Field::setFieldReferencedIfExists(m_memberCountPrefixField.get());
        Field::setFieldReferencedIfExists(m_externalLengthPrefixField);
        Field::setFieldReferencedIfExists(m_memberLengthPrefixField.get());    
        Field::setFieldReferencedIfExists(m_externalElemLengthPrefixField);
        Field::setFieldReferencedIfExists(m_memberElemLengthPrefixField.get());   
        Field::setFieldReferencedIfExists(m_externalTermSuffixField);
        Field::setFieldReferencedIfExists(m_memberTermSuffixField.get());                     
    }

private:
    Generator& m_generator;
    commsdsl::parse::ListField m_dslObj;
    Elem* m_parent = nullptr;
    Field* m_externalElementField = nullptr;
    FieldPtr m_memberElementField;
    Field* m_externalCountPrefixField = nullptr;
    FieldPtr m_memberCountPrefixField;    
    Field* m_externalLengthPrefixField = nullptr;
    FieldPtr m_memberLengthPrefixField;   
    Field* m_externalElemLengthPrefixField = nullptr;
    FieldPtr m_memberElemLengthPrefixField;        
    Field* m_externalTermSuffixField = nullptr;
    FieldPtr m_memberTermSuffixField = nullptr;
};    

ListField::ListField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<ListFieldImpl>(generator, listDslObj(), this))
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::List);
}

ListField::~ListField() = default;

Field* ListField::externalElementField()
{
    return m_impl->externalElementField();
}

const Field* ListField::externalElementField() const
{
    return m_impl->externalElementField();
}

Field* ListField::memberElementField()
{
    return m_impl->memberElementField();
}

const Field* ListField::memberElementField() const
{
    return m_impl->memberElementField();
}

Field* ListField::externalCountPrefixField()
{
    return m_impl->externalCountPrefixField();
}

const Field* ListField::externalCountPrefixField() const
{
    return m_impl->externalCountPrefixField();
}

Field* ListField::memberCountPrefixField()
{
    return m_impl->memberCountPrefixField();
}

const Field* ListField::memberCountPrefixField() const
{
    return m_impl->memberCountPrefixField();
}

Field* ListField::externalLengthPrefixField()
{
    return m_impl->externalLengthPrefixField();
}

const Field* ListField::externalLengthPrefixField() const
{
    return m_impl->externalLengthPrefixField();
}

Field* ListField::memberLengthPrefixField()
{
    return m_impl->memberLengthPrefixField();
}

const Field* ListField::memberLengthPrefixField() const
{
    return m_impl->memberLengthPrefixField();
} 

Field* ListField::externalElemLengthPrefixField()
{
    return m_impl->externalElemLengthPrefixField();
}

const Field* ListField::externalElemLengthPrefixField() const
{
    return m_impl->externalElemLengthPrefixField();
}

Field* ListField::memberElemLengthPrefixField()
{
    return m_impl->memberElemLengthPrefixField();
}

const Field* ListField::memberElemLengthPrefixField() const
{
    return m_impl->memberElemLengthPrefixField();
}

Field* ListField::externalTermSuffixField()
{
    return m_impl->externalTermSuffixField();
}

const Field* ListField::externalTermSuffixField() const
{
    return m_impl->externalTermSuffixField();
}

Field* ListField::memberTermSuffixField()
{
    return m_impl->memberTermSuffixField();
}

const Field* ListField::memberTermSuffixField() const
{
    return m_impl->memberTermSuffixField();
}

bool ListField::prepareImpl()
{
    return m_impl->prepare();
}

void ListField::setReferencedImpl()
{
    m_impl->setReferenced();
}

commsdsl::parse::ListField ListField::listDslObj() const
{
    return commsdsl::parse::ListField(dslObj());
}

} // namespace gen

} // namespace commsdsl
