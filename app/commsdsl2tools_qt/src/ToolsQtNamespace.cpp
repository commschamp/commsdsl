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

#include "ToolsQtNamespace.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtFrame.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtInterface.h"
#include "ToolsQtMessage.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtNamespace::ToolsQtNamespace(ToolsQtGenerator& generator, commsdsl::parse::ParseNamespace dslObj, commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent),
    m_factory(generator, *this)
{
}

ToolsQtNamespace::StringsList ToolsQtNamespace::toolsSourceFiles(const ToolsQtInterface& interface) const
{
    StringsList result;

    auto addToResult =
        [&result](StringsList&& list)
        {
            result.reserve(result.size() + list.size());
            std::move(list.begin(), list.end(), std::back_inserter(result));
        };

    for (auto& mPtr : genMessages()) {
        assert(mPtr);
        auto* toolsMessage = static_cast<const ToolsQtMessage*>(mPtr.get());
        assert(toolsMessage != nullptr);
        addToResult(toolsMessage->toolsSourceFiles(interface));
    }    

    if (genHasFramesRecursive() &&
        genHasMessagesRecursive()) {
        addToResult(m_factory.toolsSourceFiles(interface));
    }

    return result;
}

std::string ToolsQtNamespace::toolsFactoryRelHeaderPath(const commsdsl::gen::GenInterface& iFace) const
{
    return m_factory.toolsRelHeaderPath(iFace);
}

std::string ToolsQtNamespace::toolsFactoryClassScope(const commsdsl::gen::GenInterface& iFace) const
{
    return m_factory.toolsClassScope(iFace);
}

bool ToolsQtNamespace::genWriteImpl() const
{
    if ((!genHasFramesRecursive()) ||
        (!genHasMessagesRecursive())) {
        return true;
    }

    return m_factory.toolsWrite();
}

} // namespace commsdsl2tools_qt
