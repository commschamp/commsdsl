//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtField.h"

#include "commsdsl/gen/ListField.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtListField final : public commsdsl::gen::ListField, public ToolsQtField
{
    using Base = commsdsl::gen::ListField;
    using ToolsBase = ToolsQtField;
public:
    explicit ToolsQtListField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;   

    // ToolsBase overrides 
    virtual IncludesList toolsExtraSrcIncludesImpl() const override;
    virtual std::string toolsExtraPropsImpl() const override;
    virtual std::string toolsDefMembersImpl() const override;

private:
    std::string toolsPrefixNameInternal() const;

    ToolsQtField* m_toolsMemberElementField = nullptr;
    ToolsQtField* m_toolsExternalElementField = nullptr;
};

} // namespace commsdsl2tools_qt
