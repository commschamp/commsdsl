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

    if (m_toolsExternalField != nullptr) {
        m_toolsExternalField->toolsSetReferenced();
    }
    return true;
}

ToolsQtLayer::IncludesList ToolsQtLayer::toolsSrcIncludes() const
{
    IncludesList result;

    auto addIncsFunc = 
        [&result](const ToolsQtField* f)
        {
            if (f == nullptr) {
                return;
            }

            if (comms::isGlobalField(f->field())) {
                result.push_back(f->toolsRelDeclHeaderFile());
            }
            auto incs = f->toolsSrcIncludes();
            result.reserve(result.size() + incs.size());
            std::move(incs.begin(), incs.end(), std::back_inserter(result));
        };

    addIncsFunc(m_toolsExternalField);
    addIncsFunc(m_toolsMemberField);
    return result;
}

std::string ToolsQtLayer::toolsPropsFunc() const
{
    std::string fieldFunc;
    do {
        if (m_toolsMemberField != nullptr) {
            fieldFunc = m_toolsMemberField->toolsDefMembers();
            if (!fieldFunc.empty()) {
                fieldFunc.push_back('\n');
            }

            fieldFunc.append(m_toolsMemberField->toolsDefFunc());
            break;
        }

        if (m_toolsExternalField != nullptr) {
            static const std::string FieldTempl =
                "static QVariantMap createProps_#^#NAME#$#(bool serHidden)\n"
                "{\n"
                "    return #^#TOP_NS#$#::#^#SCOPE#$#::createProps_#^#NAME#$#(\"#^#DISP_NAME#$#\", serHidden);\n"
                "}\n";        

            auto fieldDslObj = m_toolsExternalField->field().dslObj();
            util::ReplacementMap fieldRepl = {
                {"TOP_NS", m_layer.generator().getTopNamespace()},
                {"SCOPE", comms::scopeFor(m_toolsExternalField->field(), m_layer.generator(), true, false)},
                {"NAME", comms::accessName(fieldDslObj.name())},
                {"DISP_NAME", util::displayName(fieldDslObj.displayName(), fieldDslObj.name())}
            };
            fieldFunc = util::processTemplate(FieldTempl, fieldRepl);
            break;
        }

        static const std::string FieldTempl =
            "static QVariantMap createProps_#^#NAME#$#(bool serHidden)\n"
            "{\n"
            "    return cc_tools_qt::property::field::ArrayList().name(\"#^#DISP_NAME#$#\").serialisedHidden(serHidden).asMap();\n"
            "}\n";    

        util::ReplacementMap fieldRepl = {
            {"NAME", comms::accessName(m_layer.dslObj().name())},
            {"DISP_NAME", m_layer.dslObj().name()}
        };

        fieldFunc = util::processTemplate(FieldTempl, fieldRepl);
    } while (false);

    static const std::string Templ = 
        "struct #^#NAME#$#Layer\n"
        "{\n"
        "    #^#FUNC#$#\n"
        "}; // struct #^#NAME#$#Layer\n";

    util::ReplacementMap repl = {
        {"NAME", comms::className(m_layer.dslObj().name())},
        {"FUNC", std::move(fieldFunc)}
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtLayer::toolsCreatePropsInvocation() const
{
    static const std::string Templ = 
        "#^#NAME#$#Layer::createProps_#^#FIELD_NAME#$#(false)";

    std::string fieldName;
    do {
        if (m_toolsExternalField != nullptr) {
            fieldName = m_toolsExternalField->field().dslObj().name();
            break;
        }

        if (m_toolsMemberField != nullptr) {
            fieldName = m_toolsMemberField->field().dslObj().name();
            break;
        }

        fieldName = m_layer.dslObj().name();
    } while (false);
    
    util::ReplacementMap repl = {
        {"NAME", comms::className(m_layer.dslObj().name())},
        {"FIELD_NAME", comms::accessName(fieldName)}
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtLayer::toolsFieldCommsScope() const
{
    if (m_toolsExternalField != nullptr) {
        return m_toolsExternalField->toolsCommsScope();
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

} // namespace commsdsl2tools_qt
