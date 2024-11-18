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

#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtMsgFactory
{
public:
    using StringsList = commsdsl::gen::util::StringsList;

    static bool write(ToolsQtGenerator& generator);
    static std::string toolsRelHeaderPath(const ToolsQtGenerator& generator, const commsdsl::gen::Interface& iFace);
    static StringsList toolsSourceFiles(const ToolsQtGenerator& generator, const commsdsl::gen::Interface& iFace); 
    static std::string toolsClassScope(const ToolsQtGenerator& generator);

private:
    explicit ToolsQtMsgFactory(const ToolsQtGenerator& generator) : m_generator(generator) {}

    std::string toolsRelPathInternal(const commsdsl::gen::Interface& iFace) const;
    bool toolsWriteInternal() const;
    bool toolsWriteHeaderInternal() const;
    bool toolsWriteSourceInternal() const;
    std::string toolsHeaderCodeInternal() const;
    std::string toolsSourceCodeInternal() const;
    std::string toolsSourceIncludesInternal(const commsdsl::gen::Interface& iFace) const;

    const ToolsQtGenerator& m_generator;
};

} // namespace commsdsl2tools_qt