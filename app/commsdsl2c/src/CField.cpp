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

#include "CErrorStatus.h"
#include "CGenerator.h"
#include "CInterface.h"
#include "CLayer.h"
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
    includes.push_back("<stdint.h>");
    includes.push_back(CErrorStatus::cRelHeader(CGenerator::cCast(m_genField.genGenerator())));
    return cAddHeaderIncludesImpl(includes);
}

void CField::cAddSourceIncludes(CIncludesList& includes) const
{
    if (comms::genIsGlobalField(m_genField) && m_genField.genIsReferenced()) {
        includes.push_back(cRelCommsHeader());
    }

    return cAddSourceIncludesImpl(includes);
}

void CField::cAddCommsHeaderIncludes(CIncludesList& includes) const
{
    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    auto* parent = m_genField.genGetParent();
    assert(parent != nullptr);

    if (parent->genElemType() != GenElem::GenType_Interface) {
        includes.push_back(CProtocolOptions::cRelHeader(cGenerator));
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
                "#^#COMMON_FUNC#$#\n"
                ;

            util::GenReplacementMap repl = {
                {"NAME", cName()},
                {"BRIEF", cHandleBriefInternal(forceOptional)},
                {"COMMON_FUNC", cHeaderCommonFuncsInternal(forceOptional)},
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
                "#^#COMMON_FUNCS#$#\n"
                ;

            util::GenReplacementMap repl = {
                {"CODE", cSourceCodeImpl()},
                {"COMMON_FUNCS", cSourceCommonFuncsInternal(forceOptional)},
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
                {"COMMS_TYPE", cCommsType(forceOptional)},
                {"CONV_SUFFIX", cConversionSuffix()},
            };

            return util::genProcessTemplate(Templ, repl);
        };

    static const std::string Templ =
        "#^#IMPL#$#\n"
        "#^#INNER#$#\n"
        "#^#WRAP#$#\n";

    if (!cIsVersionOptional()) {
        util::GenReplacementMap repl = {
            {"IMPL", cCommsHeaderCodeImpl()},
            {"INNER", doCode(false)},
        };

        return util::genProcessTemplate(Templ, repl);
    }

    util::GenReplacementMap repl = {
        {"IMPL", cCommsHeaderCodeImpl()},
        {"INNER", doCode(true)},
        {"WRAP", doCode(false)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cCommsType(bool forceOptional) const
{
    GenElem* optParent = &m_genField;
    bool needOptions = true;
    std::string memberSuffix;
    while (optParent != nullptr) {
        auto* nextParent = optParent->genGetParent();
        assert(nextParent != nullptr);
        auto nextParentType = nextParent->genElemType();

        if (nextParentType == GenElem::GenType::GenType_Namespace) {
            // Global field
            break;
        }

        optParent = nextParent;

        if ((nextParentType == GenElem::GenType::GenType_Field) ||
            (nextParentType == GenElem::GenType::GenType_Layer)) {
            memberSuffix = strings::genMembersSuffixStr();
            continue;
        }

        if (nextParentType == GenElem::GenType::GenType_Interface) {
            needOptions = false;
            memberSuffix = strings::genFieldsSuffixStr();
            break;
        }

        if (nextParentType == GenElem::GenType::GenType_Message) {
            memberSuffix = strings::genFieldsSuffixStr();
            break;
        }

        if (nextParentType == GenElem::GenType::GenType_Frame) {
            memberSuffix = strings::genLayersSuffixStr();
            break;
        }

        [[maybe_unused]] static const bool Should_not_reach = true;
        assert(Should_not_reach);
    }

    auto& cGenerator = CGenerator::cCast(m_genField.genGenerator());
    auto scope = comms::genScopeFor(m_genField, cGenerator);
    auto optScope = comms::genScopeFor(*optParent, cGenerator) + memberSuffix;

    auto result = optScope;
    if (needOptions) {
        result += '<' + CProtocolOptions::cName(cGenerator) + '>';
    }
    result += scope.substr(optScope.size());

    if (forceOptional) {
        result += strings::genVersionOptionalFieldSuffixStr();
    }

    return result;
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

std::string CField::cRelCommsHeader() const
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

    // if (parent->genElemType() == GenElem::GenType_Field) {
    //     static const std::string Str = "MemberFieldHandle";
    //     return Str;
    // }

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

std::string CField::cCommsHeaderCodeImpl() const
{
    return strings::genEmptyString();
}

std::string CField::cHeaderCommonValueAccessFuncs() const
{
    static const std::string Templ =
        "/// @brief Get value of the @ref #^#NAME#$# field.\n"
        "#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# #^#NAME#$##^#SUFFIX#$#_getValue(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "#^#SET_FUNC#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"VALUE_TYPE", strings::genValueTypeStr()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (!m_genField.genParseObj().parseIsFixedValue()) {
        static const std::string SetTempl =
            "/// @brief Set value of the @ref #^#NAME#$# field.\n"
            "void #^#NAME#$##^#SUFFIX#$#_setValue(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# value);"
        ;

        repl["SET_FUNC"] = util::genProcessTemplate(SetTempl, repl);
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceCommonValueAccessFuncs() const
{
    static const std::string Templ =
        "#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# #^#NAME#$##^#SUFFIX#$#_getValue(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return static_cast<#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$#>(from#^#CONV_SUFFIX#$#(field)->getValue());\n"
        "}\n"
        "\n"
        "#^#SET_FUNC#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"VALUE_TYPE", strings::genValueTypeStr()},
        {"CONV_SUFFIX", cConversionSuffix()}
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (!m_genField.genParseObj().parseIsFixedValue()) {
        static const std::string SetTempl =
            "void #^#NAME#$##^#SUFFIX#$#_setValue(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# value)\n"
            "{\n"
            "    from#^#CONV_SUFFIX#$#(field)->setValue(value);\n"
            "}"
            ;

        repl["SET_FUNC"] = util::genProcessTemplate(SetTempl, repl);
    }

    return util::genProcessTemplate(Templ, repl);
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

std::string CField::cHeaderCommonFuncsInternal(bool forcedOptional) const
{
    static const std::string Templ =
        "/// @brief Read the field's value from input buffer.\n"
        "/// @param[in, out] field Handle of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "/// @param[in, out] iter Pointer to bufer iterator.\n"
        "/// @param[in] bufLen Remaining bytes in the input buffer.\n"
        "/// @post The iterator is advanced by amount of consumed bytes.\n"
        "/// @return Status of the read operation.\n"
        "#^#ERROR_STATUS#$# #^#NAME#$##^#SUFFIX#$#_read(#^#NAME#$##^#SUFFIX#$#* field, const uint8_t** iter, size_t bufLen);\n"
        "\n"
        "/// @brief Write the field's value into output buffer.\n"
        "/// @param[in] field Handle of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "/// @param[in, out] iter Pointer to bufer iterator.\n"
        "/// @param[in] bufLen Available bytes in the output buffer.\n"
        "/// @post The iterator is advanced by amount of written bytes.\n"
        "/// @return Status of the write operation.\n"
        "#^#ERROR_STATUS#$# #^#NAME#$##^#SUFFIX#$#_write(const #^#NAME#$##^#SUFFIX#$#* field, uint8_t** iter, size_t bufLen);\n"
        "\n"
        "/// @brief Retrieve serialization length of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_length(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Retrieve name of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "const char* #^#NAME#$##^#SUFFIX#$#_name(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Check the stored value of the @ref #^#NAME#$##^#SUFFIX#$# field if valid.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_valid(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"ERROR_STATUS", CErrorStatus::cName(CGenerator::cCast(m_genField.genGenerator()))},
    };

    if (forcedOptional) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CField::cSourceCommonFuncsInternal(bool forcedOptional) const
{
    static const std::string Templ =
        "#^#ERROR_STATUS#$# #^#NAME#$##^#SUFFIX#$#_read(#^#NAME#$##^#SUFFIX#$#* field, const uint8_t** iter, size_t bufLen)\n"
        "{\n"
        "    return static_cast<#^#ERROR_STATUS#$#>(from#^#CONV_SUFFIX#$#(field)->read(*iter, bufLen));\n"
        "}\n"
        "\n"
        "#^#ERROR_STATUS#$# #^#NAME#$##^#SUFFIX#$#_write(const #^#NAME#$##^#SUFFIX#$#* field, uint8_t** iter, size_t bufLen)\n"
        "{\n"
        "    return static_cast<#^#ERROR_STATUS#$#>(from#^#CONV_SUFFIX#$#(field)->write(*iter, bufLen));\n"
        "}\n"
        "\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_length(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->length();\n"
        "}\n"
        "\n"
        "const char* #^#NAME#$##^#SUFFIX#$#_name(const #^#NAME#$##^#SUFFX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->name();\n"
        "}\n"
        "\n"
        "bool #^#NAME#$##^#SUFFIX#$#_valid(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->valid();\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"CONV_SUFFIX", cConversionSuffix()},
        {"ERROR_STATUS", CErrorStatus::cName(CGenerator::cCast(m_genField.genGenerator()))},
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

    auto prefix =
        "/// @brief Definition of <b>\"" +
        util::genDisplayName(m_genField.genParseObj().parseDisplayName(), m_genField.genParseObj().parseName()) + "\"</b>";

    auto* parent = m_genField.genGetParent();
    assert(parent != nullptr);
    auto parentElemType = parent->genElemType();

    if (parentElemType == GenElem::GenType_Namespace) {
        return prefix + " global field.";
    }

    std::string parentName;
    if (parentElemType == GenElem::GenType_Interface) {
        parentName = CInterface::cCast(static_cast<const commsdsl::gen::GenInterface*>(parent))->cName();
    }
    else if (parentElemType == GenElem::GenType_Message) {
        parentName = CMessage::cCast(static_cast<const commsdsl::gen::GenMessage*>(parent))->cName();
    }
    else if (parentElemType == GenElem::GenType_Field) {
        parentName = CField::cCast(static_cast<const commsdsl::gen::GenField*>(parent))->cName();
    }
    else if (parentElemType == GenElem::GenType_Layer) {
        parentName = CLayer::cCast(static_cast<const commsdsl::gen::GenLayer*>(parent))->cName();
    }    

    assert(!parentName.empty());
    return prefix + " member field of @ref " + parentName + '.';
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
        "/// @brief Obtain access to inner non-optional field of @ref #^#NAME#$#.\n"
        "#^#NAME#$##^#SUFFIX#$#* #^#NAME#$#_field(#^#NAME#$#* field);\n"
        "\n"
        "/// @brief Check the version dependent field @ref #^#NAME#$# exists.\n"
        "bool #^#NAME#$#_doesExist(const #^#NAME#$#* field);\n"
        "\n"
        "/// @brief Force the version dependent field @ref #^#NAME#$# into existance\n"
        "void #^#NAME#$#_setExists(#^#NAME#$#* field);\n"
        "\n"
        "/// @brief Check the version dependent field @ref #^#NAME#$# is missing.\n"
        "bool #^#NAME#$#_isMissing(const #^#NAME#$#* field);\n"
        "\n"
        "/// @brief Force the version dependent field @ref #^#NAME#$# to be missing\n"
        "void #^#NAME#$#_setMissing(#^#NAME#$#* field);\n"
        "\n"
        "/// @brief Check the version dependent field @ref #^#NAME#$# is tentative.\n"
        "bool #^#NAME#$#_isTentative(const #^#NAME#$#* field);\n"
        "\n"
        "/// @brief Force the version dependent field @ref #^#NAME#$# to be tenative\n"
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
