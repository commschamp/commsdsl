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
    m_toolsExternalField = dynamic_cast<ToolsQtField*>(m_layer.externalField());
    m_toolsMemberField = dynamic_cast<ToolsQtField*>(m_layer.memberField());
    return true;
}

ToolsQtLayer::IncludesList ToolsQtLayer::toolsSrcIncludes() const
{
    // TODO: remove
    IncludesList result;
    return result;
}

std::string ToolsQtLayer::toolsPropsFunc() const
{
    // TODO: remove
    return std::string();
}

std::string ToolsQtLayer::toolsFieldCommsScope() const
{
    if (m_toolsExternalField != nullptr) {
        return m_toolsExternalField->toolsCommsScope(toolExtraFieldTemplParamsImpl());
    }

    if (m_toolsMemberField != nullptr) {
        return m_toolsMemberField->toolsCommsScope();
    }

    auto* frameElem = m_layer.getParent();
    assert(frameElem != nullptr);
    auto& gen = static_cast<const ToolsQtGenerator&>(m_layer.generator());
    return 
        comms::scopeFor(*frameElem, gen) + 
        strings::layersSuffixStr() + ToolsQtDefaultOptions::toolsTemplParam(gen) + "::" +
        comms::className(m_layer.dslObj().name()) + "::Field";
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
        [](const ToolsQtField& f)
        {
            return static_cast<unsigned>(f.field().dslObj().minLength());
        };

    if (m_toolsExternalField !=  nullptr) {
        return calcFunc(*m_toolsExternalField);
    }

    if (m_toolsMemberField !=  nullptr) {
        return calcFunc(*m_toolsMemberField);
    }    

    assert(false); // should not happen;
    return 0U;
}

std::string ToolsQtLayer::toolExtraFieldTemplParamsImpl() const
{
    return strings::emptyString();
}

std::string ToolsQtLayer::toolsForcedSerHiddenStrImpl() const
{
    return "false";
}

ToolsQtLayer::StringsList ToolsQtLayer::toolsMsgFactoryExtraOptionsImpl() const
{
    return StringsList();
}

} // namespace commsdsl2tools_qt
