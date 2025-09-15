//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

    static const CField* cCast(const commsdsl::gen::GenField* field);
    static CField* cCast(commsdsl::gen::GenField* field);    
    static CFieldsList cTransformFieldsList(const GenFieldsList& fields);

    bool cWrite() const;
    void cAddHeaderIncludes(CIncludesList& includes) const;
    void cAddSourceIncludes(CIncludesList& includes) const;
    std::string cStructName() const;
    std::string cHeaderCode() const;
    std::string cSourceCode() const;
    std::string cCommsType(bool appendOptions = true) const;
    bool cIsVersionOptional() const;
    void cAddSourceFiles(GenStringsList& sources) const;

    const GenField& cGenField() const
    {
        return m_genField;
    }    

protected:
    virtual void cAddHeaderIncludesImpl(CIncludesList& includes) const;
    virtual void cAddSourceIncludesImpl(CIncludesList& includes) const;
    virtual std::string cHeaderCodeImpl() const;
    virtual std::string cSourceCodeImpl() const;

private:
    bool cWriteHeaderInternal() const;
    bool cWriteSrcInternal() const;
    std::string cHeaderIncludesInternal() const;
    std::string cSourceIncludesInternal() const;
    std::string cHeaderLengthFunc() const;
    std::string cSourceLengthFunc() const;
    std::string cHeaderNameFunc() const;
    std::string cSourceNameFunc() const;
    std::string cHandleBrief() const;
    
    GenField& m_genField;
};

} // namespace commsdsl2c
