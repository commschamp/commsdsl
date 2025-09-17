//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CField.h"

#include "CGenerator.h"
#include "CInterface.h"
#include "CMessage.h"
#include "CProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

CField::CField(GenField& field) : 
    m_genField(field)
{
}

CField::~CField() = default;

const CField* CField::cCast(const commsdsl::gen::GenField* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    auto* cField = dynamic_cast<const CField*>(field);    
    assert(cField != nullptr);
    return cField;
}

CField* CField::cCast(commsdsl::gen::GenField* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    auto* cField = dynamic_cast<CField*>(field);    
    assert(cField != nullptr);
    return cField;
}

CField::CFieldsList CField::cTransformFieldsList(const GenFieldsList& fields)
{
    CFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* cField = 
            const_cast<CField*>(
                dynamic_cast<const CField*>(fPtr.get()));

        assert(cField != nullptr);
        result.push_back(cField);
    }

    return result;
}

bool CField::cWrite() const
{
    if (!comms::genIsGlobalField(m_genField)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    if (!m_genField.genIsReferenced()) {
        // Not referenced fields do not need to be written
        m_genField.genGenerator().genLogger().genDebug(
            "Skipping file generation for non-referenced field \"" + m_genField.genParseObj().parseExternalRef() + "\".");
        return true;
    }

    return 
        cWriteHeaderInternal() &&
        cWriteSrcInternal();    
}

void CField::cAddHeaderIncludes(CIncludesList& includes) const
{
    includes.push_back("<stddef.h>");
    return cAddHeaderIncludesImpl(includes);
}

void CField::cAddSourceIncludes(CIncludesList& includes) const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    auto* parent = m_genField.genGetParent();
    assert(parent != nullptr);

    if (parent->genElemType() != GenElem::GenType_Interface) {
        includes.push_back(CProtocolOptions::cRelHeaderPath(cGenerator));
    }

    if (comms::genIsGlobalField(m_genField) && m_genField.genIsReferenced()) {
        includes.push_back(comms::genRelHeaderPathFor(m_genField, cGenerator));
    }
    return cAddHeaderIncludesImpl(includes);
}

std::string CField::cStructName() const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    return cGenerator.cStructNameFor(m_genField);
}

std::string CField::cHeaderCode() const
{
    static const std::string Templ = 
        "#^#BRIEF#$#\n"
        "typedef struct #^#NAME#$#_ #^#NAME#$#;\n\n"
        "#^#CODE#$#\n"
        "#^#LENGTH_FUNC#$#\n"
        "#^#NAME_FUNC#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
        {"CODE", cHeaderCodeImpl()},
        {"BRIEF", cHandleBrief()},
        {"LENGTH_FUNC", cHeaderLengthFunc()},
        {"NAME_FUNC", cHeaderNameFunc()},
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceCode() const
{
    static const std::string Templ = 
        "using #^#NAME#$#__cpp = ::#^#COMMS_TYPE#$#;\n"
        "struct alignas(alignof(#^#NAME#$#__cpp)) #^#NAME#$#_ {};\n\n"
        "namespace\n"
        "{\n\n"
        "const #^#NAME#$#__cpp* fromHandle(const #^#NAME#$#* field)\n"
        "{\n"
        "    return reinterpret_cast<const #^#NAME#$#__cpp*>(field);\n"
        "}\n"
        // "const #^#NAME#$#* toHandle(const #^#NAME#$#__cpp* field)\n"
        // "{\n"
        // "    return reinterpret_cast<const #^#NAME#$#*>(field);\n"
        // "}\n"
        "\n"        
        "} // namespace\n\n"
        "#^#CODE#$#\n"
        "#^#LENGTH_FUNC#$#\n"
        "#^#NAME_FUNC#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
        {"CODE", cSourceCodeImpl()},
        {"LENGTH_FUNC", cSourceLengthFunc()},
        {"NAME_FUNC", cSourceNameFunc()},
        {"COMMS_TYPE", cCommsType()},
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cCommsType(bool appendOptions) const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    auto adjustType = 
        [this, &cGenerator](const std::string& type, bool withOpts)
        {
            auto str = type + strings::genFieldsSuffixStr();
            if (withOpts) {
                str += '<' + CProtocolOptions::cClassName(cGenerator) + '>';
            }
            str += "::";
            str += comms::genClassName(m_genField.genName());
            return str;
        };
        
    auto* parent = m_genField.genGetParent();
    assert(parent != nullptr);
    auto parentType = parent->genElemType();
    if (parentType == GenElem::GenType::GenType_Field) {
        return adjustType(CField::cCast(static_cast<const GenField*>(parent))->cCommsType(appendOptions), false);
    }

    if (parentType == GenElem::GenType::GenType_Message) {
        return adjustType(CMessage::cCast(static_cast<const commsdsl::gen::GenMessage*>(parent))->cCommsType(false), true);
    }

    if (parentType == GenElem::GenType::GenType_Interface) {
        return adjustType(CInterface::cCast(static_cast<const commsdsl::gen::GenInterface*>(parent))->cCommsType(), false);
    }    

    // TODO: Frame Layer
    assert (parentType == GenElem::GenType::GenType_Namespace);
    auto str = comms::genScopeFor(m_genField, cGenerator);
    if (appendOptions) {
        str += '<' + CProtocolOptions::cClassName(cGenerator) + '>';
    }
    return str;
}

bool CField::cIsVersionOptional() const
{
    return comms::genIsVersionOptionalField(m_genField, m_genField.genGenerator());
}

void CField::cAddSourceFiles(GenStringsList& sources) const
{
    if ((!comms::genIsGlobalField(m_genField)) || (!m_genField.genIsReferenced())) {
        return;
    }

    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    sources.push_back(cGenerator.cRelSourceFor(m_genField));
}

void CField::cAddHeaderIncludesImpl([[maybe_unused]] CIncludesList& includes) const
{
}

void CField::cAddSourceIncludesImpl([[maybe_unused]] CIncludesList& includes) const
{
}

std::string CField::cHeaderCodeImpl() const
{
    return strings::genEmptyString();
}

std::string CField::cSourceCodeImpl() const
{
    return strings::genEmptyString();
}

bool CField::cWriteHeaderInternal() const
{
    auto& generator = CGenerator::cCast(m_genField.genGenerator());
    auto filePath = generator.cAbsHeaderFor(m_genField);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.genLogger();
    logger.genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#CPP_GUARD_BEGIN#$#\n"
        "#^#CODE#$#\n"
        "#^#APPEND#$#\n"
        "#^#CPP_GUARD_END#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cHeaderIncludesInternal()},
        {"CODE", cHeaderCode()},
        {"APPEND", util::genReadFileContents(generator.cInputAbsHeaderFor(m_genField) + strings::genAppendFileSuffixStr())},
        {"CPP_GUARD_BEGIN", CGenerator::cCppGuardBegin()},
        {"CPP_GUARD_END", CGenerator::cCppGuardEnd()},
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

bool CField::cWriteSrcInternal() const
{
    auto& generator = CGenerator::cCast(m_genField.genGenerator());
    auto filePath = generator.cAbsSourceFor(m_genField);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.genLogger();
    logger.genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#^#INCLUDES#$#\n"
        "#^#CODE#$#\n"
        "#^#APPEND#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"HEADER", generator.cRelHeaderFor(m_genField)},
        {"INCLUDES", cSourceIncludesInternal()},
        {"CODE", cSourceCode()},
        {"APPEND", util::genReadFileContents(generator.cInputAbsSourceFor(m_genField) + strings::genAppendFileSuffixStr())},
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

std::string CField::cHeaderIncludesInternal() const
{
    CIncludesList includes;
    cAddHeaderIncludes(includes);
    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CField::cSourceIncludesInternal() const
{
    CIncludesList includes;
    cAddSourceIncludes(includes);
    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CField::cHeaderLengthFunc() const
{
    static const std::string Templ = 
        "/// @brief Retrieve serialization length of the @ref #^#NAME#$# field.\n"
        "size_t #^#NAME#$#_length(const #^#NAME#$#* field);\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceLengthFunc() const
{
    static const std::string Templ = 
        "size_t #^#NAME#$#_length(const #^#NAME#$#* field)\n"
        "{\n"
        "    return fromHandle(field)->length();\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cHeaderNameFunc() const
{
    static const std::string Templ = 
        "/// @brief Retrieve name of the @ref #^#NAME#$# field.\n"
        "const char* #^#NAME#$#_name(const #^#NAME#$#* field);\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceNameFunc() const
{
    static const std::string Templ = 
        "const char* #^#NAME#$#_name(const #^#NAME#$#* field)\n"
        "{\n"
        "    return fromHandle(field)->name();\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cStructName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cHandleBrief() const
{
    if (cIsVersionOptional()) {
        return "/// @brief Inner field of @ref " + cStructName() + " optional.";
    }

    return
        "/// @brief Definition of <b>\"" +
        util::genDisplayName(m_genField.genParseObj().parseDisplayName(), m_genField.genParseObj().parseName()) +
        "\"</b> field.";
}


} // namespace commsdsl2c
