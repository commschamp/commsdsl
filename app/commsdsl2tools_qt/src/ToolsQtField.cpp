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

#include <cassert>
#include <fstream>

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

bool ToolsQtField::toolsWrite() const
{
    auto* parent = m_field.getParent();
    if (parent == nullptr) {
        assert(false); // Should not happen
        return false;
    } 

    auto type = parent->elemType();
    if (type != commsdsl::gen::Elem::Type::Type_Namespace)
    {
        // Skip write for non-global fields,
        // The code generation will be driven by other means
        return true;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_referenced)) {
        // Not referenced fields do not need to be written
        return true;
    }

    return toolsWriteHeaderInternal() && toolsWriteSrcInternal();
}

ToolsQtField::IncludesList ToolsQtField::toolsHeaderIncludes() const
{
    IncludesList incs = {
        "<QtCore/QVariantMap>"
    };

    return incs;
}

std::string ToolsQtField::toolsDeclSig() const
{
    static const std::string Templ = 
        "QVariantMap createProps_#^#NAME#$#(const char* name, bool serHidden = false)";

    util::ReplacementMap repl = {
        {"NAME", comms::accessName(m_field.dslObj().name())}
    };

    return util::processTemplate(Templ, repl);
}

bool ToolsQtField::toolsWriteHeaderInternal() const
{
    auto& generator = m_field.generator();
    auto filePath = comms::headerPathFor(m_field, generator);

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
        "#^#EXTRA_INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DECL#$#;\n"
        "#^#NS_END#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", ToolsQtGenerator::fileGeneratedComment()},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"EXTRA_INCLUDES", util::readFileContents(comms::inputCodePathFor(m_field, generator) + strings::incFileSuffixStr())},
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
    // TODO: 
    return true;
}


} // namespace commsdsl2tools_qt
