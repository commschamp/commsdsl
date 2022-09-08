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

#include "ToolsQtNamespace.h"

#include "ToolsQtField.h"
#include "ToolsQtFrame.h"
#include "ToolsQtGenerator.h"
#include "ToolsQtInterface.h"
#include "ToolsQtMessage.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace commsdsl2tools_qt
{

ToolsQtNamespace::ToolsQtNamespace(ToolsQtGenerator& generator, commsdsl::parse::Namespace dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

ToolsQtNamespace::StringsList ToolsQtNamespace::toolsSourceFiles() const
{
    StringsList result;

    auto addToResult =
        [&result](StringsList&& list)
        {
            result.reserve(result.size() + list.size());
            std::move(list.begin(), list.end(), std::back_inserter(result));
        };

    // for (auto& iPtr : interfaces()) {
    //     assert(iPtr);
    //     auto* toolsInterface = static_cast<const ToolsQtInterface*>(iPtr.get());
    //     assert(toolsInterface != nullptr);
    //     addToResult(toolsInterface->toolsSourceFiles());
    // }            

    for (auto& fPtr : fields()) {
        assert(fPtr);
        auto* toolsField = dynamic_cast<const ToolsQtField*>(fPtr.get());
        assert(toolsField != nullptr);
        addToResult(toolsField->toolsSourceFiles());
    }

    for (auto& mPtr : messages()) {
        assert(mPtr);
        auto* toolsMessage = static_cast<const ToolsQtMessage*>(mPtr.get());
        assert(toolsMessage != nullptr);
        addToResult(toolsMessage->toolsSourceFiles());
    }    

    for (auto& fPtr : frames()) {
        assert(fPtr);
        auto* toolsFrame = static_cast<const ToolsQtFrame*>(fPtr.get());
        assert(toolsFrame != nullptr);
        addToResult(toolsFrame->toolsSourceFiles());
    }       

    return result;
}

} // namespace commsdsl2tools_qt
