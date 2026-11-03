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

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2swig
{

class SwigField
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using SwigFieldsList = std::vector<SwigField*>;

    explicit SwigField(commsdsl::gen::GenField& field);
    virtual ~SwigField();

    [[nodiscard]] static const SwigField* swigCast(const commsdsl::gen::GenField* field);
    [[nodiscard]] static SwigField* swigCast(commsdsl::gen::GenField* field);

    [[nodiscard]] static SwigFieldsList swigTransformFieldsList(const commsdsl::gen::GenField::GenFieldsList& fields);

    [[nodiscard]] commsdsl::gen::GenField& swigGenField()
    {
        return m_genField;
    }

    [[nodiscard]] const commsdsl::gen::GenField& swigGenField() const
    {
        return m_genField;
    }

    [[nodiscard]] std::string swigRelHeaderPath() const;

    [[nodiscard]] bool swigIsVersionOptional() const;
    [[nodiscard]] std::string swigClassDecl() const;
    [[nodiscard]] std::string swigPublicDecl() const;
    [[nodiscard]] std::string swigExtraPublicFuncsCode() const;

    void swigAddCodeIncludes(GenStringsList& list) const;
    void swigAddCode(GenStringsList& list) const;
    void swigAddDef(GenStringsList& list) const;

    [[nodiscard]] std::string swigTemplateScope() const;

    [[nodiscard]] bool swigWrite() const;

    void swigSetListElement()
    {
        m_listElement = true;
    }

protected:
    [[nodiscard]] virtual std::string swigMembersDeclImpl() const;
    [[nodiscard]] virtual std::string swigValueTypeDeclImpl() const;
    [[nodiscard]] virtual std::string swigValueAccDeclImpl() const;
    [[nodiscard]] virtual std::string swigExtraPublicFuncsDeclImpl() const;
    [[nodiscard]] virtual std::string swigExtraPublicFuncsCodeImpl() const;
    [[nodiscard]] virtual std::string swigPublicDeclImpl() const;
    virtual void swigAddDefImpl(GenStringsList& list) const;
    virtual void swigAddMembersCodeImpl(GenStringsList& list) const;

    [[nodiscard]] std::string swigCommonPublicFuncsDecl() const;
    [[nodiscard]] std::string swigCommonPublicFuncsCode() const;

    [[nodiscard]] std::string swigSemanticTypeLengthValueAccDecl() const;
    [[nodiscard]] std::string swigSemanticTypeLengthValueAccCode() const;

private:
    [[nodiscard]] std::string swigClassDeclInternal() const;
    [[nodiscard]] std::string swigOptionalDeclInternal() const;
    [[nodiscard]] std::string swigClassCodeInternal() const;
    [[nodiscard]] std::string swigComparisonRenameInternal() const;
    void swigAddVectorTemplateInternal(GenStringsList& list) const;

    commsdsl::gen::GenField& m_genField;
    mutable bool m_defAdded = false;
    mutable bool m_codeAdded = false;
    bool m_listElement = false;
};

} // namespace commsdsl2swig
