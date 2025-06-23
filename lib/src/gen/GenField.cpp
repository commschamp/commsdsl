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

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class GenFieldImpl
{
public:
    GenFieldImpl(GenGenerator& generator, const commsdsl::parse::ParseField& dslObj) : 
        m_generator(generator),
        m_dslObj(dslObj)
    {
    }

    bool isPrepared()
    {
        return m_prepared;
    }

    void setPrepared()
    {
        m_prepared = true;
    }

    const commsdsl::parse::ParseField& dslObj() const
    {
        return m_dslObj;
    } 

    GenGenerator& generator()
    {
        return m_generator;
    }

    const GenGenerator& generator() const
    {
        return m_generator;
    }

    bool isReferenced() const
    {
        return m_referenced;
    }

    void setReferenced()
    {
        m_referenced = true;
    }

private:
    GenGenerator& m_generator;
    commsdsl::parse::ParseField m_dslObj;
    bool m_prepared = false;
    bool m_referenced = false;
};    

GenField::GenField(GenGenerator& generator, const commsdsl::parse::ParseField& dslObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenFieldImpl>(generator, dslObj))
{
}

GenField::~GenField() = default;

GenField::Ptr GenField::create(GenGenerator& generator, commsdsl::parse::ParseField dslobj, GenElem* parent)
{
    using CreateFunc = FieldPtr (GenGenerator::*)(commsdsl::parse::ParseField dslobj, GenElem* parent);
    static const CreateFunc Map[] = {
        /* Int */ &GenGenerator::createIntField,
        /* Enum */ &GenGenerator::createEnumField,
        /* Set */ &GenGenerator::createSetField,
        /* Float */ &GenGenerator::createFloatField,
        /* Bitfield */ &GenGenerator::createBitfieldField,
        /* Bundle */ &GenGenerator::createBundleField,
        /* String */ &GenGenerator::createStringField,
        /* Data */ &GenGenerator::createDataField,
        /* List */ &GenGenerator::createListField,
        /* Ref */ &GenGenerator::createRefField,
        /* Optional */ &GenGenerator::createOptionalField,
        /* Variant */ &GenGenerator::createVariantField,
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(commsdsl::parse::ParseField::Kind::NumOfValues), "Invalid map");

    auto idx = static_cast<std::size_t>(dslobj.kind());
    if (MapSize <= idx) {
        [[maybe_unused]] static constexpr bool Unexpected_kind = false;
        assert(Unexpected_kind);          
        return Ptr();
    }

    auto func = Map[idx];
    assert(func != nullptr); // NYI
    return (generator.*func)(dslobj, parent);
}

bool GenField::isPrepared() const
{
    return m_impl->isPrepared();
}

bool GenField::prepare()
{
    if (m_impl->isPrepared()) {
        return true;
    }

    bool result = prepareImpl();
    if (result) {
        m_impl->setPrepared();
    }

    if (dslObj().isForceGen()) {
        setReferenced();
    }
    
    return result;
}

bool GenField::write() const
{
    return writeImpl();
}

const commsdsl::parse::ParseField& GenField::dslObj() const
{
    return m_impl->dslObj();
}

GenGenerator& GenField::generator()
{
    return m_impl->generator();
}

const GenGenerator& GenField::generator() const
{
    return m_impl->generator();
}

bool GenField::isReferenced() const
{
    return m_impl->isReferenced();
}

void GenField::setReferenced()
{
    m_impl->setReferenced();
    setReferencedImpl();
}

void GenField::setFieldReferencedIfExists(GenField* field)
{
    if (field != nullptr) {
        field->setReferenced();
    }
}

std::string GenField::templateScopeOfComms(const std::string& protOptionsStr) const
{
    auto commsScope = comms::scopeFor(*this, generator());
    std::string optionsParams = "<" + protOptionsStr + ">";

    if (comms::isGlobalField(*this)) {
        return commsScope + optionsParams;
    }

    auto formScopeFunc = 
        [this, &commsScope, &optionsParams](const GenElem* parent, const std::string& suffix)
        {
            auto optLevelScope = comms::scopeFor(*parent, generator()) + suffix;
            assert(optLevelScope.size() < commsScope.size());
            assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
            
            return optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());
        };

    
    auto* parent = getParent();
    while (parent != nullptr)  {
        auto elemType = parent->elemType();

        if (elemType == GenElem::Type_Interface) {
            return commsScope;
        }        

        if ((elemType == GenElem::Type_Field) && (comms::isGlobalField(*parent))) {
            return formScopeFunc(parent, strings::membersSuffixStr());
        }        

        if (elemType == GenElem::Type_Message) {
            return formScopeFunc(parent, strings::fieldsSuffixStr());
        }

        if (elemType == GenElem::Type_Frame) {
            return formScopeFunc(parent, strings::layersSuffixStr());
        }        

        parent = parent->getParent();
    }

    assert(false); // Should not happen
    return commsScope;
}

GenField::FieldRefInfo GenField::processInnerRef(const std::string& refStr) const
{
    if (refStr.empty()) {
        FieldRefInfo info;
        info.m_field = this;
        info.m_refType = FieldRefType_Field;
        return info;
    }

    return processInnerRefImpl(refStr);
}

GenField::FieldRefInfo GenField::processMemberRef(const FieldsList& fields, const std::string& refStr)
{
    FieldRefInfo info;
    auto dotPos = refStr.find_first_of('.');
    std::string fieldName(refStr, 0, dotPos);
    if (fieldName.empty()) {
        return info;
    }

    auto iter =
        std::find_if(
            fields.begin(), fields.end(),
            [&fieldName](auto& f)
            {
                return f->name() == fieldName;
            });

    if (iter == fields.end()) {
        return info;
    }

    auto nextPos = refStr.size();
    if (dotPos < refStr.size()) {
        nextPos = dotPos + 1U;
    }

    return (*iter)->processInnerRef(refStr.substr(nextPos));
}

const GenNamespace* GenField::parentNamespace() const
{
    auto* parent = getParent();
    while (parent != nullptr) {
        if (parent->elemType() == GenElem::Type_Namespace) {
            return static_cast<const GenNamespace*>(parent);
        }

        parent = parent->getParent();
    }

    return static_cast<const GenNamespace*>(parent);
}

GenElem::Type GenField::elemTypeImpl() const
{
    return Type_Field;
}

bool GenField::prepareImpl()
{
    return true;
}

bool GenField::writeImpl() const
{
    return true;
}

void GenField::setReferencedImpl()
{
}

GenField::FieldRefInfo GenField::processInnerRefImpl([[maybe_unused]] const std::string& refStr) const
{
    assert(!refStr.empty());
    FieldRefInfo info;
    return info;
}

} // namespace gen

} // namespace commsdsl
