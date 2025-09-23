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
        cWriteSrcInternal() &&
        cWriteCommsHeaderInternal();    
}

std::string CField::cRelHeader() const
{
    assert(comms::genIsGlobalField(m_genField));
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    return cGenerator.cRelHeaderFor(m_genField);
}

void CField::cAddHeaderIncludes(CIncludesList& includes) const
{
    includes.push_back("<stddef.h>");
    return cAddHeaderIncludesImpl(includes);
}

void CField::cAddSourceIncludes(CIncludesList& includes) const
{
    if (comms::genIsGlobalField(m_genField) && m_genField.genIsReferenced()) {
        includes.push_back(cRelCommsDefHeader());
    }

    return cAddSourceIncludesImpl(includes);
}

void CField::cAddCommsHeaderIncludes(CIncludesList& includes) const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    auto* parent = m_genField.genGetParent();
    assert(parent != nullptr);    

    if (parent->genElemType() != GenElem::GenType_Interface) {
        includes.push_back(CProtocolOptions::cRelHeaderPath(cGenerator));
    }

    if (comms::genIsGlobalField(m_genField) && m_genField.genIsReferenced()) {
        includes.push_back(comms::genRelHeaderPathFor(m_genField, cGenerator));
        includes.push_back(cRelHeader());
    }
    
    return cAddCommsHeaderIncludesImpl(includes);
}

std::string CField::cName(bool forceOptional) const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    auto str = cGenerator.cNameFor(m_genField);
    if (forceOptional) {
        str += strings::genVersionOptionalFieldSuffixStr();
    }
    return str;
}

std::string CField::cCommsTypeName(bool forceOptional) const
{
    return cName(forceOptional) + strings::genCommsNameSuffixStr();
}

std::string CField::cHeaderCode() const
{
    auto doCode = 
        [this](bool forceOptional)
        {
            static const std::string Templ = 
                "#^#BRIEF#$#\n"
                "typedef struct #^#NAME#$##^#SUFFIX#$#_ #^#NAME#$##^#SUFFIX#$#;\n\n"
                "#^#CODE#$#\n"
                "#^#LENGTH_FUNC#$#\n"
                "#^#NAME_FUNC#$#\n"
                ;

            util::GenReplacementMap repl = {
                {"NAME", cName()},
                {"BRIEF", cHandleBriefInternal(forceOptional)},
                {"LENGTH_FUNC", cHeaderLengthFuncInternal(forceOptional)},
                {"NAME_FUNC", cHeaderNameFuncInternal(forceOptional)},
            };

            if (forceOptional) {
                repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
            }

            if (cIsVersionOptional() && (!forceOptional)) {
                repl["CODE"] = cHeaderOptionalCodeInternal();
            }     
            else {
                repl["CODE"] = cHeaderCodeImpl();
            }       

            return util::genProcessTemplate(Templ, repl);
        };

    if (!cIsVersionOptional()) {
        return doCode(false);
    }

    static const std::string Templ = 
        "#^#INNER#$#\n"
        "#^#WRAP#$#\n";
    
    util::GenReplacementMap repl = {
        {"INNER", doCode(true)},
        {"WRAP", doCode(false)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceCode() const
{
    auto doCode = 
        [this](bool forceOptional)
        {    
            static const std::string Templ = 
                "#^#CODE#$#\n"
                "#^#LENGTH_FUNC#$#\n"
                "#^#NAME_FUNC#$#\n"
                ;

            util::GenReplacementMap repl = {
                {"CODE", cSourceCodeImpl()},
                {"LENGTH_FUNC", cSourceLengthFuncInternal(forceOptional)},
                {"NAME_FUNC", cSourceNameFuncInternal(forceOptional)},
            };

            if (cIsVersionOptional() && (!forceOptional)) {
                repl["CODE"] = cSourceOptionalCodeInternal();
            }     
            else {
                repl["CODE"] = cSourceCodeImpl();
            }             

            return util::genProcessTemplate(Templ, repl);
        };

    if (!cIsVersionOptional()) {
        return doCode(false);
    }

    static const std::string Templ = 
        "#^#INNER#$#\n"
        "#^#WRAP#$#\n";
    
    util::GenReplacementMap repl = {
        {"INNER", doCode(true)},
        {"WRAP", doCode(false)},
    };

    return util::genProcessTemplate(Templ, repl);        
}

std::string CField::cCommsHeaderCode() const
{
    auto doCode = 
        [this](bool forceOptional)
        {    
            static const std::string Templ = 
                "using #^#COMMS_NAME#$# = ::#^#COMMS_TYPE#$#;\n"
                "struct alignas(alignof(#^#COMMS_NAME#$#)) #^#NAME#$#_ {};\n\n"
                "inline const #^#COMMS_NAME#$#* from#^#CONV_SUFFIX#$#(const #^#NAME#$#* from)\n"
                "{\n"
                "    return reinterpret_cast<const #^#COMMS_NAME#$#*>(from);\n"
                "}\n\n"
                "inline #^#COMMS_NAME#$#* from#^#CONV_SUFFIX#$#(#^#NAME#$#* from)\n"
                "{\n"
                "    return reinterpret_cast<#^#COMMS_NAME#$#*>(from);\n"
                "}\n\n"        
                "inline const #^#NAME#$#* to#^#CONV_SUFFIX#$#(const #^#COMMS_NAME#$#* from)\n"
                "{\n"
                "    return reinterpret_cast<const #^#NAME#$#*>(from);\n"
                "}\n\n"
                "inline #^#NAME#$#* to#^#CONV_SUFFIX#$#(#^#COMMS_NAME#$#* from)\n"
                "{\n"
                "    return reinterpret_cast<#^#NAME#$#*>(from);\n"
                "}\n"        
                ;

            util::GenReplacementMap repl = {
                {"NAME", cName(forceOptional)},
                {"COMMS_NAME", cCommsTypeName(forceOptional)},
                {"COMMS_TYPE", cCommsType(true, forceOptional)},
                {"CONV_SUFFIX", cConversionSuffix()},
            };

            return util::genProcessTemplate(Templ, repl);   
        };     

    if (!cIsVersionOptional()) {
        return doCode(false);
    }

    static const std::string Templ = 
        "#^#INNER#$#\n"
        "#^#WRAP#$#\n";
    
    util::GenReplacementMap repl = {
        {"INNER", doCode(true)},
        {"WRAP", doCode(false)},
    };

    return util::genProcessTemplate(Templ, repl);    
}

std::string CField::cCommsType(bool appendOptions, bool forceOptional) const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    auto adjustType = 
        [this, &cGenerator, forceOptional](const std::string& type, bool withOpts)
        {
            auto str = type + strings::genFieldsSuffixStr();
            if (withOpts) {
                str += '<' + CProtocolOptions::cClassName(cGenerator) + '>';
            }
            str += "::";
            str += comms::genClassName(m_genField.genName());

            if (forceOptional) {
                str += strings::genVersionOptionalFieldSuffixStr();
            }

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

std::string CField::cRelCommsDefHeader() const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    return cGenerator.cRelCommsHeaderFor(m_genField);
}

const std::string& CField::cConversionSuffix() const
{
    auto* parent = m_genField.genGetParent();
    assert(parent != nullptr);    

    if (parent->genElemType() == GenElem::GenType_Interface) {
        static const std::string Str = "InterfaceFieldHandle";
        return Str;
    }

    if (parent->genElemType() == GenElem::GenType_Message) {
        static const std::string Str = "MessageFieldHandle";
        return Str;
    }   
    
    if (parent->genElemType() == GenElem::GenType_Layer) {
        static const std::string Str = "LayerFieldHandle";
        return Str;
    }        

    static const std::string Str = "FieldHandle";
    return Str;    
}

void CField::cAddHeaderIncludesImpl([[maybe_unused]] CIncludesList& includes) const
{
}

void CField::cAddSourceIncludesImpl([[maybe_unused]] CIncludesList& includes) const
{
}

void CField::cAddCommsHeaderIncludesImpl([[maybe_unused]] CIncludesList& includes) const
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

bool CField::cWriteCommsHeaderInternal() const
{
    auto& generator = CGenerator::cCast(m_genField.genGenerator());
    auto filePath = generator.cAbsCommsHeaderFor(m_genField);
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
        "#^#CODE#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cCommsHeaderIncludesInternal()},
        {"CODE", cCommsHeaderCode()},
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

std::string CField::cHeaderLengthFuncInternal(bool forcedOptional) const
{
    static const std::string Templ = 
        "/// @brief Retrieve serialization length of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_length(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
    };

    if (forcedOptional) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceLengthFuncInternal(bool forcedOptional) const
{
    static const std::string Templ = 
        "size_t #^#NAME#$##^#SUFFIX#$#_length(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->length();\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"CONV_SUFFIX", cConversionSuffix()},
    };

    if (forcedOptional) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }    

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cHeaderNameFuncInternal(bool forcedOptional) const
{
    static const std::string Templ = 
        "/// @brief Retrieve name of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "const char* #^#NAME#$##^#SUFFIX#$#_name(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
    };

    if (forcedOptional) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }    

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceNameFuncInternal(bool forcedOptional) const
{
    static const std::string Templ = 
        "const char* #^#NAME#$##^#SUFFIX#$#_name(const #^#NAME#$##^#SUFFX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->name();\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"CONV_SUFFIX", cConversionSuffix()},
    };

    if (forcedOptional) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }       

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cHandleBriefInternal(bool forcedOptional) const
{
    if (forcedOptional) {
        return "/// @brief Inner field of @ref " + cName() + " optional.";
    }

    return
        "/// @brief Definition of <b>\"" +
        util::genDisplayName(m_genField.genParseObj().parseDisplayName(), m_genField.genParseObj().parseName()) +
        "\"</b> field.";
}

std::string CField::cCommsHeaderIncludesInternal() const
{
    CIncludesList includes;
    cAddCommsHeaderIncludes(includes);
    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CField::cHeaderOptionalCodeInternal() const
{
    static const std::string Templ = 
        "/// @brief Obtain access to inner non-optional field.\n"
        "#^#NAME#$##^#SUFFIX#$#* #^#NAME#$#_field(#^#NAME#$#* field);\n"
        "\n"
        "/// @brief Check the version dependent field exists.\n"
        "bool #^#NAME#$#_doesExist(const #^#NAME#$#* field);\n"
        "\n"
        "/// @brief Force the version dependent field into existance\n"
        "void #^#NAME#$#_setExists(#^#NAME#$#* field);\n"
        "\n"
        "/// @brief Check the version dependent field is missing.\n"
        "bool #^#NAME#$#_isMissing(const #^#NAME#$#* field);\n"
        "\n"
        "/// @brief Force the version dependent field to be missing\n"
        "void #^#NAME#$#_setMissing(#^#NAME#$#* field);\n"
        "\n"
        "/// @brief Check the version dependent field is tentative.\n"
        "bool #^#NAME#$#_isTentative(const #^#NAME#$#* field);\n"
        "\n"
        "/// @brief Force the version dependent field to be tenative\n"
        "void #^#NAME#$#_setTentative(#^#NAME#$#* field);\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"SUFFIX", strings::genVersionOptionalFieldSuffixStr()}
    };

    return util::genProcessTemplate(Templ, repl);        
}

std::string CField::cSourceOptionalCodeInternal() const
{
    static const std::string Templ = 
        "#^#NAME#$##^#SUFFIX#$#* #^#NAME#$#_field(#^#NAME#$#* field)\n"
        "{\n"
        "    return to#^#CONV_SUFFIX#$#(&(from#^#CONV_SUFFIX#$#(field)->field()));\n"
        "}\n"
        "\n"
        "bool #^#NAME#$#_doesExist(const #^#NAME#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->doesExist();\n"
        "}\n"        
        "\n"
        "void #^#NAME#$#_setExists(#^#NAME#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->setExists();\n"
        "}\n"         
        "\n"
        "bool #^#NAME#$#_isMissing(const #^#NAME#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->isMissing();\n"
        "}\n"        
        "\n"
        "void #^#NAME#$#_setMissing(#^#NAME#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->setMissing();\n"
        "}\n"         
        "\n"
        "bool #^#NAME#$#_isTenative(const #^#NAME#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->isTentative();\n"
        "}\n"        
        "\n"
        "void #^#NAME#$#_setTenative(#^#NAME#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->setTentative();\n"
        "}\n"  
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"SUFFIX", strings::genVersionOptionalFieldSuffixStr()},
        {"CONV_SUFFIX", cConversionSuffix()},
    };

    return util::genProcessTemplate(Templ, repl);        
}

} // namespace commsdsl2c
