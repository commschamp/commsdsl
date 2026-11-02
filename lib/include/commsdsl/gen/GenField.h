//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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
    using GenPtr = std::unique_ptr<GenField>;
    using GenFieldsList = std::vector<GenPtr>;
    using ParseField = commsdsl::parse::ParseField;

    enum GenFieldRefType
    {
        FieldRefType_Invalid,
        FieldRefType_Field,
        FieldRefType_InnerValue,
        FieldRefType_ValuesLimit
    };

    struct GenFieldRefInfo
    {
        const GenField* m_field = nullptr;
        std::string m_valueName;
        GenFieldRefType m_refType = FieldRefType_Invalid;
    };

    virtual ~GenField();

    [[nodiscard]] static GenPtr genCreate(GenGenerator& generator, ParseField parseObj, GenElem* parent = nullptr);

    [[nodiscard]] const std::string& genDisplayName() const;

    [[nodiscard]] bool genIsPrepared() const;
    [[nodiscard]] bool genPrepare();
    [[nodiscard]] bool genWrite() const;

    [[nodiscard]] const ParseField& genParseObj() const;

    [[nodiscard]] GenGenerator& genGenerator();
    [[nodiscard]] const GenGenerator& genGenerator() const;

    [[nodiscard]] bool genIsReferenced() const;
    void genSetReferenced(bool referenced = true);

    static void genSetFieldReferencedIfExists(GenField* field, bool referenced = true);

    [[nodiscard]] std::string genTemplateScopeOfComms(const std::string& protOptionsStr) const;

    [[nodiscard]] GenFieldRefInfo genProcessInnerRef(const std::string& refStr) const;
    [[nodiscard]] static GenFieldRefInfo genProcessMemberRef(const GenFieldsList& fields, const std::string& refStr);

    [[nodiscard]] const GenNamespace* genParentNamespace() const;
    [[nodiscard]] const GenFieldsList& genMembers() const;

protected:
    GenField(GenGenerator& generator, const ParseField& parseObj, GenElem* parent = nullptr);

    [[nodiscard]] virtual GenType genElemTypeImpl() const override final;
    [[nodiscard]] virtual bool genPrepareImpl();
    [[nodiscard]] virtual bool genWriteImpl() const;
    virtual void genSetReferencedImpl(bool referenced);
    [[nodiscard]] virtual GenFieldRefInfo genProcessInnerRefImpl(const std::string& refStr) const;
    [[nodiscard]] virtual const GenFieldsList& genMembersImpl() const;

private:
    std::unique_ptr<GenFieldImpl> m_impl;
};

using GenFieldPtr = GenField::GenPtr;

} // namespace gen

} // namespace commsdsl
