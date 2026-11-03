//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtInterface final : public commsdsl::gen::GenInterface
{
    using GenBase = commsdsl::gen::GenInterface;

public:
    using ParseInterface = commsdsl::parse::ParseInterface;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit ToolsQtInterface(ToolsQtGenerator& generator, ParseInterface parseObj, GenElem* parent);

    [[nodiscard]] std::string toolsClassScope() const;
    [[nodiscard]] std::string toolsHeaderFilePath() const;
    [[nodiscard]] GenStringsList toolsSourceFiles() const;

    [[nodiscard]] static const ToolsQtInterface* toolsCast(const commsdsl::gen::GenInterface* obj)
    {
        return static_cast<const ToolsQtInterface*>(obj);
    }

    [[nodiscard]] static const ToolsQtInterface& toolsCast(const commsdsl::gen::GenInterface& obj)
    {
        return static_cast<const ToolsQtInterface&>(obj);
    }

protected:
    [[nodiscard]] virtual bool genWriteImpl() const override;

private:
    [[nodiscard]] bool toolsWriteHeaderInternal() const;
    [[nodiscard]] bool toolsWriteSrcInternal() const;
    [[nodiscard]] std::string toolsHeaderCodeInternal() const;
    [[nodiscard]] std::string toolsSrcCodeInternal() const;
    [[nodiscard]] const std::string& toolsNameInternal() const;
    [[nodiscard]] std::string toolsRelFilePath() const;
};

} // namespace commsdsl2tools_qt
