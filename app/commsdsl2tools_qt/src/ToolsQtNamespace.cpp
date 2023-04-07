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

#include "ToolsQtNamespace.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtField.h"
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

    return result;
}

std::string ToolsQtNamespace::toolsMsgFactoryOptions() const
{
    util::StringsList elems;
    auto addStrFunc = 
        [&elems](std::string&& str)
        {
            if (!str.empty()) {
                elems.push_back(std::move(str));
            }
        };

    auto& subNsList = namespaces();
    for (auto& nsPtr : subNsList) {
        addStrFunc(ToolsQtNamespace::cast(nsPtr.get())->toolsMsgFactoryOptions());
    }

    for (auto& fPtr : frames()) {
        assert(fPtr);
        addStrFunc(ToolsQtFrame::cast(fPtr.get())->toolsMsgFactoryOptions());
    }   

    if (elems.empty()) {
        return strings::emptyString();
    }    

    auto& gen = ToolsQtGenerator::cast(generator());
    auto nsName = dslObj().name();
    bool hasMainNs = gen.toolsHasMainNamespaceInOptions();
    if (nsName.empty() && (!hasMainNs)) {
        return util::strListToString(elems, "\n", "");
    }

    if (nsName.empty()) {
        nsName = gen.currentSchema().mainNamespace();
    }

    static const std::string Templ = 
        "struct #^#NAME#$# : public #^#DEFAULT_OPTS#$#::#^#NS#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}; // struct #^#NAME#$#\n";    

    util::ReplacementMap repl = {
        {"NAME", nsName},
        {"BODY", util::strListToString(elems, "\n", "")},
        {"DEFAULT_OPTS", ToolsQtDefaultOptions::toolsScope(gen)},
        {"NS", comms::scopeFor(*this, generator(), hasMainNs)},
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt
