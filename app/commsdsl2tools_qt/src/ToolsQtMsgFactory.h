//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include <string>

#include "commsdsl/gen/util.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtMsgFactory
{
public:
    using StringsList = commsdsl::gen::util::StringsList;

    static bool write(ToolsQtGenerator& generator);
    static std::string toolsRelHeaderPath(const ToolsQtGenerator& generator);
    static StringsList toolsSourceFiles(const ToolsQtGenerator& generator); 
    static std::string toolsClassScope(const ToolsQtGenerator& generator);

private:
    explicit ToolsQtMsgFactory(ToolsQtGenerator& generator) : m_generator(generator) {}

    bool toolsWriteInternal() const;
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSourceInternal() const;
    bool toolsHasUniqueIdsInternal() const;
    bool toolsIsGeneratedInternal() const;
    bool toolsHasSourceInternal() const;
    std::string toolsHeaderCodeInternal() const;
    std::string toolsHeaderSingleInterfaceCodeInternal() const;
    std::string toolsHeaderMultipleInterfacesCodeInternal() const;
    std::string toolsSourceCodeInternal() const;
    

    ToolsQtGenerator& m_generator;
};

} // namespace commsdsl2tools_qt