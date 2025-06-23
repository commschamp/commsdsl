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
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/gen/GenElem.h"

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

    static Ptr create(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent = nullptr);    

    bool isPrepared() const;
    bool prepare();
    bool write() const;

    const commsdsl::parse::ParseField& dslObj() const;    

    GenGenerator& generator();
    const GenGenerator& generator() const;

    bool isReferenced() const;
    void setReferenced();

    static void setFieldReferencedIfExists(GenField* field);

    std::string templateScopeOfComms(const std::string& protOptionsStr) const;

    FieldRefInfo processInnerRef(const std::string& refStr) const;
    static FieldRefInfo processMemberRef(const FieldsList& fields, const std::string& refStr);

    const GenNamespace* parentNamespace() const;

protected:    
    GenField(GenGenerator& generator, const commsdsl::parse::ParseField& dslObj, GenElem* parent = nullptr);

    virtual Type elemTypeImpl() const override final;
    virtual bool prepareImpl();
    virtual bool writeImpl() const;
    virtual void setReferencedImpl();
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const;


private:
    std::unique_ptr<GenFieldImpl> m_impl;
};

using FieldPtr = GenField::Ptr;

} // namespace gen

} // namespace commsdsl
