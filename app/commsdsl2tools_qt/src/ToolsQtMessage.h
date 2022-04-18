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
#include "ToolsQtInterface.h"

#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtMessage final : public commsdsl::gen::Message
{
    using Base = commsdsl::gen::Message;
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;

    using ToolsQtFieldsList = ToolsQtField::ToolsQtFieldsList;

    explicit ToolsQtMessage(ToolsQtGenerator& generator, commsdsl::parse::Message dslObj, commsdsl::gen::Elem* parent);

    StringsList toolsSourceFiles() const;

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() override;    

private:
    enum CodeType : unsigned
    {
        CodeType_MultipleInterfaces,
        CodeType_SinglePimplInterface,
        CodeType_SingleInterfaceWithFields,
        CodeType_NumOfValues
    };

    bool toolsWriteHeaderInternal();
    bool toolsWriteSrcInternal();
    std::string toolsRelPathInternal() const;
    IncludesList toolsHeaderIncludesInternal() const;
    IncludesList toolsHeaderIncludesMultipleInterfacesInternal() const;
    IncludesList toolsHeaderIncludesSinglePimplInterfaceInternal() const;
    IncludesList toolsHeaderIncludesSingleInterfaceWithFieldsInternal() const;
    std::string toolsHeaderCodeInternal() const;
    IncludesList toolsSrcIncludesInternal() const;
    IncludesList toolsSrcIncludesMultipleInterfacesInternal() const;
    IncludesList toolsSrcIncludesSinglePimplInterfaceInternal() const;
    IncludesList toolsSrcIncludesSingleInterfaceWithFieldsInternal() const;
    std::string toolsSrcCodeInternal() const;

    CodeType toolCodeType() const;
    ToolsQtFieldsList m_fields;
    bool m_exists = true;
};

} // namespace commsdsl2tools_qt
