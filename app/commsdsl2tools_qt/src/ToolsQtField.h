//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2tools_qt
{

class ToolsQtField
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;
    using ToolsQtFieldsList = std::vector<ToolsQtField*>;

    explicit ToolsQtField(commsdsl::gen::Field& field);
    virtual ~ToolsQtField();

    static ToolsQtFieldsList toolsTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields);

    const commsdsl::gen::Field& field() const
    {
        return m_field;
    }

    bool toolsWrite() const;

    void commsSetForcePseudo()
    {
        m_forcedPseudo = true;
    }

    void toolsSetReferenced()
    {
        m_referenced = true;
    }

    bool toolsIsPseudo() const;

    IncludesList toolsHeaderIncludes() const;
    IncludesList toolsSrcIncludes() const;
    std::string toolsDeclSig() const;
    std::string toolsDefFunc() const;
    std::string toolsDefMembers() const;
    std::string toolsCommsScope() const;

    std::string relDeclHeaderFile() const;
    std::string relDefSrcFile() const;

protected:
    virtual IncludesList toolsExtraSrcIncludesImpl() const;    
    virtual std::string toolsDefFuncBodyImpl() const;
    virtual std::string toolsExtraPropsImpl() const;
    virtual std::string toolsDefMembersImpl() const;

private:
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSrcInternal() const;
    std::string toolsDeclSigInternal(bool defaultSerHidden = true) const;
    std::string toolsRelPathInternal() const;
    std::string toolsSerHiddenParamInternal() const;
    std::string toolsDefAnonimousInternal() const;

    commsdsl::gen::Field& m_field;
    bool m_forcedPseudo = false;
    bool m_referenced = false;
};

} // namespace commsdsl2tools_qt
