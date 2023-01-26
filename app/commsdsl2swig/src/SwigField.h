//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2swig
{

class SwigField
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;
    using SwigFieldsList = std::vector<SwigField*>;

    explicit SwigField(commsdsl::gen::Field& field);
    virtual ~SwigField();

    static const SwigField* cast(const commsdsl::gen::Field* field);
    static SwigField* cast(commsdsl::gen::Field* field);

    static SwigFieldsList swigTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields);

    commsdsl::gen::Field& field()
    {
        return m_field;
    }

    const commsdsl::gen::Field& field() const
    {
        return m_field;
    }

    std::string swigRelHeaderPath() const;

    bool swigIsVersionOptional() const;
    std::string swigClassDecl() const;
    std::string swigPublicDecl() const;
    std::string swigExtraPublicFuncsCode() const;
    
    void swigAddCodeIncludes(StringsList& list) const; 
    void swigAddCode(StringsList& list) const; 
    void swigAddDef(StringsList& list) const;

    std::string swigTemplateScope() const;

    bool swigWrite() const;

    void swigSetListElement()
    {
        m_listElement = true;
    }

protected:
    virtual std::string swigMembersDeclImpl() const;
    virtual std::string swigValueTypeDeclImpl() const;
    virtual std::string swigValueAccDeclImpl() const;
    virtual std::string swigExtraPublicFuncsDeclImpl() const;
    virtual std::string swigExtraPublicFuncsCodeImpl() const;
    virtual std::string swigPublicDeclImpl() const;
    virtual void swigAddDefImpl(StringsList& list) const;
    virtual void swigAddMembersCodeImpl(StringsList& list) const;

    std::string swigCommonPublicFuncsDecl() const;
    std::string swigCommonPublicFuncsCode() const;

    std::string swigSemanticTypeLengthValueAccDecl() const;
    std::string swigSemanticTypeLengthValueAccCode() const;
    
private:
    std::string swigClassDeclInternal() const;
    std::string swigOptionalDeclInternal() const;
    std::string swigClassCodeInternal() const;
    std::string swigComparisonRenameInternal() const;
    void swigAddVectorTemplateInternal(StringsList& list) const;

    commsdsl::gen::Field& m_field;
    mutable bool m_defAdded = false;
    mutable bool m_codeAdded = false;
    bool m_listElement = false;
};

} // namespace commsdsl2swig
