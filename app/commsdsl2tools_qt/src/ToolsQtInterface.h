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

#include "ToolsQtField.h"

#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtInterface final : public commsdsl::gen::Interface
{
    using Base = commsdsl::gen::Interface;
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;

    using ToolsQtFieldsList = ToolsQtField::ToolsQtFieldsList;

    explicit ToolsQtInterface(ToolsQtGenerator& generator, commsdsl::parse::Interface dslObj, commsdsl::gen::Elem* parent);

    std::string toolsHeaderFilePath() const;

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() override;    

private:
    bool toolsWriteHeaderInternal();
    bool toolsWriteSrcInternal();
    std::string toolsHeaderCodeInternal() const;
    std::string toolsSrcCodeInternal() const;
    const std::string& toolsNameInternal() const;

    ToolsQtFieldsList m_fields;
};

} // namespace commsdsl2tools_qt
