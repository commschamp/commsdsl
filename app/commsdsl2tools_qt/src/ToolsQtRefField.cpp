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

#include "ToolsQtRefField.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtRefField::ToolsQtRefField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtRefField::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    assert(referencedField() != nullptr);
    m_toolsReferenceField = dynamic_cast<ToolsQtField*>(referencedField());
    assert(m_toolsReferenceField != nullptr);
    m_toolsReferenceField->toolsSetReferenced();
    return true;
}

bool ToolsQtRefField::writeImpl() const
{
    return toolsWrite();
}

ToolsQtRefField::IncludesList ToolsQtRefField::toolsExtraSrcIncludesImpl() const
{
    assert(m_toolsReferenceField != nullptr);
    return IncludesList{
        m_toolsReferenceField->toolsRelDeclHeaderFile()
    };
}

std::string ToolsQtRefField::toolsDefFuncBodyImpl() const
{
    static const std::string Templ =
        "using Field = ::#^#SCOPE#$#;\n"
        "auto props = #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$#, #^#SER_HIDDEN#$#);\n"
        "#^#EXTRA_PROPS#$#\n"
        "return props;";

    static const std::string VerOptTempl =
        "using Field = ::#^#SCOPE#$#Field;\n"
        "auto props = #^#PLUGIN_SCOPE#$#createProps_#^#REF_NAME#$#(#^#NAME_PROP#$#, #^#SER_HIDDEN#$#);\n"
        "#^#EXTRA_PROPS#$#\n"
        "using OptField = ::#^#SCOPE#$#;\n"
        "return\n"
        "    cc_tools_qt::property::field::ForField<OptField>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        .uncheckable()\n"
        "        .field(std::move(props))\n"
        "        .asMap();\n";

    bool verOptional = comms::isVersionOptionaField(*this, generator());
    auto* templ = &Templ;
    if (verOptional) {
        templ = &VerOptTempl;
    }

    auto& gen = generator();
    auto parent = getParent();
    while (parent != nullptr) {
        if (parent->elemType() == commsdsl::gen::Elem::Type::Type_Layer) {
            parent = parent->getParent();
        }

        if (parent->elemType() != commsdsl::gen::Elem::Type::Type_Field) {
            break;
        }

        if (comms::isGlobalField(*parent)) {
            break;
        }

        parent = parent->getParent();
    }

    assert(parent != nullptr);

    auto scope = toolsCommsScope();
    
    util::ReplacementMap repl = {
        {"SCOPE", scope},
        {"PLUGIN_SCOPE", gen.getTopNamespace() + "::" + comms::scopeFor(*referencedField(), gen, true, false) + "::"},
        {"REF_NAME", comms::accessName(referencedField()->dslObj().name())},
        {"NAME_PROP", comms::isGlobalField(*this) ? std::string("name") : std::string("Field::name()")},
        {"SER_HIDDEN", toolsIsPseudo() ? std::string("true") : std::string("serHidden")},
        {"EXTRA_PROPS", toolsExtraPropsInternal()}
    };

    return util::processTemplate(*templ, repl);
}

std::string ToolsQtRefField::toolsExtraPropsInternal() const
{
    util::StringsList updates;

    assert(m_toolsReferenceField != nullptr);
    bool displayReadOnly = dslObj().isDisplayReadOnly();
    if (displayReadOnly != m_toolsReferenceField->field().dslObj().isDisplayReadOnly()) {
        updates.push_back(".readOnly(" + util::boolToString(displayReadOnly) + ')');
    }

    bool displayHidden = dslObj().isDisplayHidden();
    if (displayHidden != m_toolsReferenceField->field().dslObj().isDisplayHidden()) {
        updates.push_back(".hidden(" + util::boolToString(displayHidden) + ')');
    }

    if (updates.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "auto extraProps =\n"
        "    cc_tools_qt::property::field::Common()\n"
        "        #^#UPDATES#$#;\n"
        "extraProps.setTo(props);";

    util::ReplacementMap repl = {
        {"UPDATES", util::strListToString(updates, "\n")}
    };
    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt
