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

#pragma once

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/gen/GenElem.h"
#include "commsdsl/parse/ParseField.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenFieldImpl;
class GenGenerator;
class GenNamespace;

class COMMSDSL_API GenField : public GenElem
{
    using Base = GenElem;
public:
    using Ptr = std::unique_ptr<GenField>;
    using FieldsList = std::vector<Ptr>;
    using ParseField = commsdsl::parse::ParseField;

    enum FieldRefType
    {
        FieldRefType_Invalid,
        FieldRefType_Field,
        FieldRefType_InnerValue,
        FieldRefType_ValuesLimit
    };

    struct FieldRefInfo
    {
        const GenField* m_field = nullptr;
        std::string m_valueName;
        FieldRefType m_refType = FieldRefType_Invalid;
    };    

    virtual ~GenField();

    static Ptr genCreate(GenGenerator& generator, ParseField parseObj, GenElem* parent = nullptr);    

    bool genIsPrepared() const;
    bool genPrepare();
    bool genWrite() const;

    const ParseField& genParseObj() const;    

    GenGenerator& genGenerator();
    const GenGenerator& genGenerator() const;

    bool genIsReferenced() const;
    void genSetReferenced();

    static void genSetFieldReferencedIfExists(GenField* field);

    std::string genTemplateScopeOfComms(const std::string& protOptionsStr) const;

    FieldRefInfo genProcessInnerRef(const std::string& refStr) const;
    static FieldRefInfo genProcessMemberRef(const FieldsList& fields, const std::string& refStr);

    const GenNamespace* genParentNamespace() const;

protected:    
    GenField(GenGenerator& generator, const ParseField& parseObj, GenElem* parent = nullptr);

    virtual Type genElemTypeImpl() const override final;
    virtual bool genPrepareImpl();
    virtual bool genWriteImpl() const;
    virtual void genSetReferencedImpl();
    virtual FieldRefInfo genProcessInnerRefImpl(const std::string& refStr) const;


private:
    std::unique_ptr<GenFieldImpl> m_impl;
};

using GenFieldPtr = GenField::Ptr;

} // namespace gen

} // namespace commsdsl
