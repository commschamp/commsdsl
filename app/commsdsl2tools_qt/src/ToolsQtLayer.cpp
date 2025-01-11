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

#include "ToolsQtLayer.h"

#include "ToolsQtDefaultOptions.h"
#include "ToolsQtGenerator.h"

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

ToolsQtLayer::ToolsQtLayer(commsdsl::gen::Layer& layer) :
    m_layer(layer)
{
}

bool ToolsQtLayer::prepare()
{
    return true;
}

std::string ToolsQtLayer::toolsMsgFactoryOptions() const
{
    util::StringsList extraOpts = toolsMsgFactoryExtraOptionsImpl();
    if (extraOpts.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "using #^#NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#OPTS#$#,\n"
        "        typename #^#DEFAULT_OPTS#$#::#^#SCOPE#$#\n"
        "    >;\n";    

    auto& gen = ToolsQtGenerator::cast(m_layer.generator());

    util::ReplacementMap repl = {
        {"NAME", comms::className(m_layer.dslObj().name())},
        {"OPTS", util::strListToString(extraOpts, ",\n", "")},
        {"DEFAULT_OPTS", ToolsQtDefaultOptions::toolsClassScope(gen)},
        {"SCOPE", comms::scopeFor(m_layer, gen, gen.toolsHasMainNamespaceInOptions())}
    };

    return util::processTemplate(Templ, repl);
}

unsigned ToolsQtLayer::toolsMinFieldLength() const
{
    auto calcFunc = 
        [](const commsdsl::gen::Field& f)
        {
            return static_cast<unsigned>(f.dslObj().minLength());
        };

    auto* externalField = m_layer.externalField();
    if (externalField !=  nullptr) {
        return calcFunc(*externalField);
    }

    auto* memberField = m_layer.memberField();
    if (memberField !=  nullptr) {
        return calcFunc(*memberField);
    }    

    assert(false); // should not happen;
    return 0U;
}

ToolsQtLayer::StringsList ToolsQtLayer::toolsMsgFactoryExtraOptionsImpl() const
{
    return StringsList();
}

} // namespace commsdsl2tools_qt
