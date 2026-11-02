//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2c
{

class CField
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenElem = commsdsl::gen::GenElem;
    using GenField = commsdsl::gen::GenField;
    using GenFieldsList = GenField::GenFieldsList;

    using CIncludesList = GenStringsList;
    using CFieldsList = std::vector<CField*>;

    explicit CField(GenField& field);
    virtual ~CField();

    [[nodiscard]] static const CField* cCast(const commsdsl::gen::GenField* field);
    [[nodiscard]] static CField* cCast(commsdsl::gen::GenField* field);
    [[nodiscard]] static CFieldsList cTransformFieldsList(const GenFieldsList& fields);

    [[nodiscard]] bool cWrite() const;
    [[nodiscard]] std::string cRelHeader() const;
    void cAddHeaderIncludes(CIncludesList& includes) const;
    void cAddSourceIncludes(CIncludesList& includes) const;
    void cAddCommsHeaderIncludes(CIncludesList& includes) const;
    [[nodiscard]] std::string cName(bool forceOptional = false) const;
    [[nodiscard]] std::string cCommsTypeName(bool forceOptional = false) const;
    [[nodiscard]] std::string cHeaderCode() const;
    [[nodiscard]] std::string cSourceCode() const;
    [[nodiscard]] std::string cCommsHeaderCode() const;
    [[nodiscard]] std::string cCommsType(bool forceOptional = false) const;
    [[nodiscard]] bool cIsVersionOptional() const;
    void cAddSourceFiles(GenStringsList& sources) const;
    [[nodiscard]] std::string cRelCommsHeader() const;
    [[nodiscard]] std::string cFrameValueDef(const std::string& name) const;
    [[nodiscard]] std::string cFrameValueAssign(const std::string& valueAccess, const std::string& fieldAccess) const;

    [[nodiscard]] const GenField& cGenField() const
    {
        return m_genField;
    }

    [[nodiscard]] const std::string& cConversionSuffix() const;

protected:
    virtual void cAddHeaderIncludesImpl(CIncludesList& includes) const;
    virtual void cAddSourceIncludesImpl(CIncludesList& includes) const;
    virtual void cAddCommsHeaderIncludesImpl(CIncludesList& includes) const;
    [[nodiscard]] virtual std::string cHeaderCodeImpl() const;
    [[nodiscard]] virtual std::string cSourceCodeImpl() const;
    [[nodiscard]] virtual std::string cCommsHeaderCodeImpl() const;
    [[nodiscard]] virtual std::string cFrameValueDefImpl(const std::string& name) const;
    [[nodiscard]] virtual std::string cFrameValueAssignImpl(const std::string& valueAccess, const std::string& fieldAccess) const;

    [[nodiscard]] std::string cHeaderCommonValueAccessFuncs() const;
    [[nodiscard]] std::string cSourceCommonValueAccessFuncs() const;
    [[nodiscard]] std::string cCommonFrameValueDef(const std::string& typeStr, const std::string& name) const;
    [[nodiscard]] std::string cCommonFrameValueAssign(const std::string& valueAccess, const std::string& fieldAccess) const;

private:
    [[nodiscard]] bool cWriteHeaderInternal() const;
    [[nodiscard]] bool cWriteSrcInternal() const;
    [[nodiscard]] bool cWriteCommsHeaderInternal() const;
    [[nodiscard]] std::string cHeaderIncludesInternal() const;
    [[nodiscard]] std::string cSourceIncludesInternal() const;
    [[nodiscard]] std::string cHeaderCommonFuncsInternal(bool forcedOptional) const;
    [[nodiscard]] std::string cSourceCommonFuncsInternal(bool forcedOptional) const;
    [[nodiscard]] std::string cHandleBriefInternal(bool forcedOptional) const;
    [[nodiscard]] std::string cCommsHeaderIncludesInternal() const;
    [[nodiscard]] std::string cHeaderOptionalCodeInternal() const;
    [[nodiscard]] std::string cSourceOptionalCodeInternal() const;

    GenField& m_genField;
};

} // namespace commsdsl2c
