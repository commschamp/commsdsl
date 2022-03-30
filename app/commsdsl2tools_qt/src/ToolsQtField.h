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

    explicit ToolsQtField(commsdsl::gen::Field& field);
    virtual ~ToolsQtField();

    bool toolsWrite() const;

    void toolsSetReferenced()
    {
        m_referenced = true;
    }

    IncludesList toolsHeaderIncludes() const;
    std::string toolsDeclSig() const;

private:
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSrcInternal() const;

    commsdsl::gen::Field& m_field;
    bool m_referenced = true; // TODO: make false as default
};

} // namespace commsdsl2tools_qt
