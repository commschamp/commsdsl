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
#include "commsdsl/gen/util.h"

#include <cassert>
#include <algorithm>

namespace util = commsdsl::gen::util;

namespace commsdsl
{

namespace gen
{

class GenFieldImpl
{
public:
    using ParseField = GenField::ParseField;

    GenFieldImpl(GenGenerator& generator, const ParseField& parseObj) : 
        m_generator(generator),
        m_parseObj(parseObj)
    {
    }

    bool genIsPrepared()
    {
        return m_prepared;
    }

    void genSetPrepared()
    {
        m_prepared = true;
    }

    const ParseField& genParseObj() const
    {
        return m_parseObj;
    } 

    GenGenerator& genGenerator()
    {
        return m_generator;
    }

    const GenGenerator& genGenerator() const
    {
        return m_generator;
    }

    bool genIsReferenced() const
    {
        return m_referenced;
    }

    void genSetReferenced()
    {
        m_referenced = true;
    }

private:
    GenGenerator& m_generator;
    ParseField m_parseObj;
    bool m_prepared = false;
    bool m_referenced = false;
};    

GenField::GenField(GenGenerator& generator, const ParseField& parseObj, GenElem* parent) :
    Base(parent),
    m_impl(std::make_unique<GenFieldImpl>(generator, parseObj))
{
}

GenField::~GenField() = default;

GenField::GenPtr GenField::genCreate(GenGenerator& generator, ParseField parseobj, GenElem* parent)
{
    using CreateFunc = GenFieldPtr (GenGenerator::*)(ParseField parseobj, GenElem* parent);
    static const CreateFunc Map[] = {
        /* Int */ &GenGenerator::genCreateIntField,
        /* Enum */ &GenGenerator::genCreateEnumField,
        /* Set */ &GenGenerator::genCreateSetField,
        /* Float */ &GenGenerator::genCreateFloatField,
        /* Bitfield */ &GenGenerator::genCreateBitfieldField,
        /* Bundle */ &GenGenerator::genCreateBundleField,
        /* String */ &GenGenerator::genCreateStringField,
        /* Data */ &GenGenerator::genCreateDataField,
        /* List */ &GenGenerator::genCreateListField,
        /* Ref */ &GenGenerator::genCreateRefField,
        /* Optional */ &GenGenerator::genCreateOptionalField,
        /* Variant */ &GenGenerator::genCreateVariantField,
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(ParseField::ParseKind::NumOfValues), "Invalid map");

    auto idx = static_cast<std::size_t>(parseobj.parseKind());
    if (MapSize <= idx) {
        [[maybe_unused]] static constexpr bool Unexpected_kind = false;
        assert(Unexpected_kind);          
        return GenPtr();
    }

    auto func = Map[idx];
    assert(func != nullptr); // NYI
    return (generator.*func)(parseobj, parent);
}

bool GenField::genIsPrepared() const
{
    return m_impl->genIsPrepared();
}

const std::string& GenField::genDisplayName() const
{
    auto& obj = m_impl->genParseObj();
    return util::genDisplayName(obj.parseDisplayName(), obj.parseName());
}

bool GenField::genPrepare()
{
    if (m_impl->genIsPrepared()) {
        return true;
    }

    bool result = genPrepareImpl();
    if (result) {
        m_impl->genSetPrepared();
    }

    if (genParseObj().parseIsForceGen()) {
        genSetReferenced();
    }
    
    return result;
}

bool GenField::genWrite() const
{
    return genWriteImpl();
}

const GenField::ParseField& GenField::genParseObj() const
{
    return m_impl->genParseObj();
}

GenGenerator& GenField::genGenerator()
{
    return m_impl->genGenerator();
}

const GenGenerator& GenField::genGenerator() const
{
    return m_impl->genGenerator();
}

bool GenField::genIsReferenced() const
{
    return m_impl->genIsReferenced();
}

void GenField::genSetReferenced()
{
    m_impl->genSetReferenced();
    genSetReferencedImpl();
}

void GenField::genSetFieldReferencedIfExists(GenField* field)
{
    if (field != nullptr) {
        field->genSetReferenced();
    }
}

std::string GenField::genTemplateScopeOfComms(const std::string& protOptionsStr) const
{
    auto commsScope = comms::genScopeFor(*this, genGenerator());
    std::string optionsParams = "<" + protOptionsStr + ">";

    if (comms::genIsGlobalField(*this)) {
        return commsScope + optionsParams;
    }

    auto formScopeFunc = 
        [this, &commsScope, &optionsParams](const GenElem* parent, const std::string& suffix)
        {
            auto optLevelScope = comms::genScopeFor(*parent, genGenerator()) + suffix;
            assert(optLevelScope.size() < commsScope.size());
            assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
            
            return optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());
        };

    
    auto* parent = genGetParent();
    while (parent != nullptr)  {
        auto elemType = parent->genElemType();

        if (elemType == GenElem::GenType_Interface) {
            return commsScope;
        }        

        if ((elemType == GenElem::GenType_Field) && (comms::genIsGlobalField(*parent))) {
            return formScopeFunc(parent, strings::genMembersSuffixStr());
        }        

        if (elemType == GenElem::GenType_Message) {
            return formScopeFunc(parent, strings::genFieldsSuffixStr());
        }

        if (elemType == GenElem::GenType_Frame) {
            return formScopeFunc(parent, strings::genLayersSuffixStr());
        }        

        parent = parent->genGetParent();
    }

    assert(false); // Should not happen
    return commsScope;
}

GenField::GenFieldRefInfo GenField::genProcessInnerRef(const std::string& refStr) const
{
    if (refStr.empty()) {
        GenFieldRefInfo info;
        info.m_field = this;
        info.m_refType = FieldRefType_Field;
        return info;
    }

    return genProcessInnerRefImpl(refStr);
}

GenField::GenFieldRefInfo GenField::genProcessMemberRef(const GenFieldsList& fields, const std::string& refStr)
{
    GenFieldRefInfo info;
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
                return f->genName() == fieldName;
            });

    if (iter == fields.end()) {
        return info;
    }

    auto nextPos = refStr.size();
    if (dotPos < refStr.size()) {
        nextPos = dotPos + 1U;
    }

    return (*iter)->genProcessInnerRef(refStr.substr(nextPos));
}

const GenNamespace* GenField::genParentNamespace() const
{
    auto* parent = genGetParent();
    while (parent != nullptr) {
        if (parent->genElemType() == GenElem::GenType_Namespace) {
            return static_cast<const GenNamespace*>(parent);
        }

        parent = parent->genGetParent();
    }

    return static_cast<const GenNamespace*>(parent);
}

GenElem::GenType GenField::genElemTypeImpl() const
{
    return GenType_Field;
}

bool GenField::genPrepareImpl()
{
    return true;
}

bool GenField::genWriteImpl() const
{
    return true;
}

void GenField::genSetReferencedImpl()
{
}

GenField::GenFieldRefInfo GenField::genProcessInnerRefImpl([[maybe_unused]] const std::string& refStr) const
{
    assert(!refStr.empty());
    GenFieldRefInfo info;
    return info;
}

} // namespace gen

} // namespace commsdsl
