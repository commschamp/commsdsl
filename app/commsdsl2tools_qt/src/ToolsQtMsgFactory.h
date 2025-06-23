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

#include <string>

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtNamespace;

class ToolsQtMsgFactory
{
public:
    using StringsList = commsdsl::gen::util::StringsList;

    ToolsQtMsgFactory(const ToolsQtGenerator& generator, const ToolsQtNamespace& parent);
    bool toolsWrite() const;
    std::string toolsRelHeaderPath(const commsdsl::gen::GenInterface& iFace) const;
    StringsList toolsSourceFiles(const commsdsl::gen::GenInterface& iFace) const; 
    std::string toolsClassScope(const commsdsl::gen::GenInterface& iFace) const;

private:
    std::string toolsRelPathInternal(const commsdsl::gen::GenInterface& iFace) const;
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSourceInternal() const;
    std::string toolsHeaderCodeInternal() const;
    std::string toolsSourceCodeInternal(const commsdsl::gen::GenInterface& iFace) const;
    std::string toolsSourceIncludesInternal(const commsdsl::gen::GenInterface& iFace) const;

    const ToolsQtGenerator& m_generator;
    const ToolsQtNamespace& m_parent;
};

} // namespace commsdsl2tools_qt