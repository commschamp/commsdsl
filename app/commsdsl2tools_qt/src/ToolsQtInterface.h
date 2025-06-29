//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtInterface final : public commsdsl::gen::GenInterface
{
    using Base = commsdsl::gen::GenInterface;
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;

    explicit ToolsQtInterface(ToolsQtGenerator& generator, commsdsl::parse::ParseInterface dslObj, commsdsl::gen::GenElem* parent);

    std::string toolsClassScope() const;
    std::string toolsHeaderFilePath() const;
    StringsList toolsSourceFiles() const;

    static const ToolsQtInterface* cast(const commsdsl::gen::GenInterface* val)
    {
        return static_cast<const ToolsQtInterface*>(val);
    }

    static const ToolsQtInterface& cast(const commsdsl::gen::GenInterface& val)
    {
        return static_cast<const ToolsQtInterface&>(val);
    }    

protected:
    virtual bool genWriteImpl() const override;    

private:
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSrcInternal() const;
    std::string toolsHeaderCodeInternal() const;
    std::string toolsSrcCodeInternal() const;
    const std::string& toolsNameInternal() const;
    std::string toolsRelFilePath() const;
};

} // namespace commsdsl2tools_qt
