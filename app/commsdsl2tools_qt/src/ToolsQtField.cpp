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

#include "ToolsQtField.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtField::ToolsQtField(commsdsl::gen::Field& field) :
    m_field(field)
{
    static_cast<void>(m_field);
}

ToolsQtField::~ToolsQtField() = default;

ToolsQtField::ToolsQtFieldsList ToolsQtField::toolsTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields)
{
    ToolsQtFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* toolsField = 
            const_cast<ToolsQtField*>(
                dynamic_cast<const ToolsQtField*>(fPtr.get()));

        assert(toolsField != nullptr);
        result.push_back(toolsField);
    }

    return result;
}

bool ToolsQtField::toolsWrite() const
{
    if (!comms::isGlobalField(m_field)) { 
        return true;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_referenced)) {
        // Not referenced fields do not need to be written
        return true;
    }

    return toolsWriteHeaderInternal() && toolsWriteSrcInternal();
}

bool ToolsQtField::toolsIsPseudo() const
{
    if (m_forcedPseudo || m_field.dslObj().isPseudo()) {
        return true;
    }

    auto* parent = m_field.getParent();
    while (parent != nullptr) {
        if (parent->elemType() != commsdsl::gen::Elem::Type_Field) {
            break;
        }

        auto* parentField = dynamic_cast<const ToolsQtField*>(parent);
        assert(parentField != nullptr);
        if (parentField->toolsIsPseudo()) {
            return true;
        }

        parent = parent->getParent();
    }

    return false;
}

ToolsQtField::IncludesList ToolsQtField::toolsHeaderIncludes() const
{
    IncludesList incs = {
        "<QtCore/QVariantMap>"
    };

    return incs;
}

ToolsQtField::IncludesList ToolsQtField::toolsSrcIncludes() const
{
    IncludesList incs = {
        "cc_tools_qt/property/field.h"
    };

    if (comms::isGlobalField(m_field)) {
        incs.push_back(comms::relHeaderPathFor(m_field, m_field.generator()));
    }

    auto extra = toolsExtraSrcIncludesImpl();
    incs.reserve(incs.size() + extra.size());
    std::move(extra.begin(), extra.end(), std::back_inserter(incs));

    std::sort(incs.begin(), incs.end());
    incs.erase(
        std::unique(incs.begin(), incs.end()),
        incs.end());

    return incs;
}

std::string ToolsQtField::toolsDeclSig() const
{
    return toolsDeclSigInternal();
}

std::string ToolsQtField::toolsDefFunc() const
{
    static const std::string Templ = 
        "#^#DECL#$#\n"
        "{\n"
        "    #^#SER_HIDDEN_CAST#$#\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"DECL", toolsDeclSigInternal(false)},
        {"SER_HIDDEN_CAST", "static_cast<void>(serHidden);"},
        {"BODY", toolsDefFuncBodyImpl()},
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtField::toolsDefMembers() const
{
    auto body = toolsDefMembersImpl();
    if (body.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "struct #^#NAME#$##^#SUFFIX#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}; // struct #^#NAME#$##^#SUFFIX#$#\n";

    util::ReplacementMap repl = {
        {"NAME", comms::className(m_field.dslObj().name())},
        {"SUFFIX", strings::membersSuffixStr()},
        {"BODY", std::move(body)},
    };

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtField::toolsCommsScope() const
{
    auto parent = m_field.getParent();
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

    auto& generator = m_field.generator();
    auto scope = comms::scopeFor(m_field, generator);
    bool globalField = comms::isGlobalField(m_field);
    do {
        if (globalField) {
            scope += "<>";
            break;
        }

        auto parentScope = comms::scopeFor(*parent, generator);

        if ((parent->elemType() == commsdsl::gen::Elem::Type::Type_Message) ||
            (parent->elemType() == commsdsl::gen::Elem::Type::Type_Interface)) {
            parentScope += strings::fieldsSuffixStr();
        }    
        else {
            parentScope += strings::membersSuffixStr();
        }

        scope = parentScope + "<>" + scope.substr(parentScope.size());

    } while (false);

    return scope;
}

std::string ToolsQtField::relDeclHeaderFile() const
{
    return toolsRelPathInternal() + strings::cppHeaderSuffixStr();
}

std::string ToolsQtField::relDefSrcFile() const
{
    return toolsRelPathInternal() + strings::cppSourceSuffixStr();
}

ToolsQtField::IncludesList ToolsQtField::toolsExtraSrcIncludesImpl() const
{
    return IncludesList();
}

std::string ToolsQtField::toolsDefFuncBodyImpl() const
{
    static const std::string FieldDefTempl =
        "    cc_tools_qt::property::field::ForField<Field>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        #^#SER_HIDDEN#$#\n"
        "        #^#READ_ONLY#$#\n"          
        "        #^#HIDDEN#$#\n"
        "        #^#PROPERTIES#$#\n"
        "        .asMap();\n";

    static const std::string Templ =
        "using Field = #^#SCOPE#$#;\n"
        "return\n" + 
        FieldDefTempl;

    static const std::string VerOptTempl =
        "using Field = #^#SCOPE#$#Field;\n"
        "using OptField = #^#SCOPE#$#;\n"
        "auto props =\n" + 
        FieldDefTempl + "\n"
        "return\n"
        "    cc::property::field::ForField<OptField>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        .uncheckable()\n"
        "        .field(std::move(props))\n"
        "        .asMap();\n";

    auto& generator = m_field.generator();
    bool verOptional = comms::isVersionOptionaField(m_field, generator);
    auto* templ = &Templ;
    if (verOptional) {
        templ = &VerOptTempl;
    }

    auto scope = toolsCommsScope();
    auto dslObj = m_field.dslObj();

    util::ReplacementMap repl = {
        {"SCOPE", scope},
        {"SER_HIDDEN", toolsSerHiddenParamInternal()},
        {"READ_ONLY", dslObj.isDisplayReadOnly() ? std::string(".readOnly()") : strings::emptyString()},
        {"HIDDEN", dslObj.isDisplayHidden() ? std::string(".hidden()") : strings::emptyString()},
        {"PROPERTIES", toolsExtraPropsImpl()}
    };

    if (comms::isGlobalField(m_field)) {
        repl["NAME_PROP"] = "name";
    }
    else {
        repl["NAME_PROP"] = "Field::name()";
    }
    return util::processTemplate(*templ, repl);
}

std::string ToolsQtField::toolsExtraPropsImpl() const
{
    return strings::emptyString();
}

std::string ToolsQtField::toolsDefMembersImpl() const
{
    return strings::emptyString();
}

bool ToolsQtField::toolsWriteHeaderInternal() const
{
    auto& generator = m_field.generator();
    auto filePath = m_field.generator().getOutputDir() + '/' + relDeclHeaderFile();

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }

    auto includes = toolsHeaderIncludes();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DECL#$#;\n\n"
        "#^#NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
        {"DECL", toolsDeclSig()},
    };
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

bool ToolsQtField::toolsWriteSrcInternal() const
{
    auto& generator = m_field.generator();
    auto filePath = m_field.generator().getOutputDir() + '/' + relDefSrcFile();

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    auto includes = toolsSrcIncludes();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#include \"#^#NAME#$#.h\"\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#ANONIMOUS#$#\n\n"
        "#^#DEF#$#\n\n"
        "#^#NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"NAME", comms::className(m_field.dslObj().name())},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
        {"DEF", toolsDefFunc()},
        {"ANONIMOUS", toolsDefAnonimousInternal()}
    };

    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

std::string ToolsQtField::toolsDeclSigInternal(bool defaultSerHidden) const
{
    static const std::string Templ = 
        "QVariantMap createProps_#^#NAME#$#(#^#NAME_PARAM#$#bool serHidden#^#DEF_VALUE#$#)";

    util::ReplacementMap repl = {
        {"NAME", comms::accessName(m_field.dslObj().name())}
    };

    if (defaultSerHidden) {
        repl["DEF_VALUE"] = " = false";
    }

    if (comms::isGlobalField(m_field)) {
        repl["NAME_PARAM"] = "const char* name, ";
    }

    return util::processTemplate(Templ, repl);
}

std::string ToolsQtField::toolsRelPathInternal() const
{
    auto scope = comms::scopeFor(m_field, m_field.generator(), false);
    return util::strReplace(scope, "::", "/");
}

std::string ToolsQtField::toolsSerHiddenParamInternal() const
{
    if (toolsIsPseudo()) {
        return ".serialisedHidden(true)";
    }

    return ".serialisedHidden(serHidden)";
}

std::string ToolsQtField::toolsDefAnonimousInternal() const
{
    auto body = toolsDefMembers();
    if (body.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "namespace\n"
        "{\n\n"
        "#^#BODY#$#\n"
        "} // namespace";

    util::ReplacementMap repl = {
        {"BODY", std::move(body)}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2tools_qt
