//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "SwigField.h"

#include "SwigComms.h"
#include "SwigDataBuf.h"
#include "SwigGenerator.h"
#include "SwigOptionalField.h"
#include "SwigProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;


namespace commsdsl2swig
{

SwigField::SwigField(commsdsl::gen::GenField& field) :
    m_field(field)
{
}

SwigField::~SwigField() = default;

const SwigField* SwigField::cast(const commsdsl::gen::GenField* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    auto* swigField = dynamic_cast<const SwigField*>(field);    
    assert(swigField != nullptr);
    return swigField;
}

SwigField* SwigField::cast(commsdsl::gen::GenField* field)
{
    return const_cast<SwigField*>(cast(static_cast<const commsdsl::gen::GenField*>(field)));
}

SwigField::SwigFieldsList SwigField::swigTransformFieldsList(const commsdsl::gen::GenField::GenFieldsList& fields)
{
    SwigFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* swigField = 
            const_cast<SwigField*>(
                dynamic_cast<const SwigField*>(fPtr.get()));

        assert(swigField != nullptr);
        result.push_back(swigField);
    }

    return result;    
}

std::string SwigField::swigRelHeaderPath() const
{
    return comms::genRelHeaderPathFor(m_field, m_field.genGenerator());
}

bool SwigField::swigIsVersionOptional() const
{
    return comms::genIsVersionOptionalField(m_field, m_field.genGenerator());
}

std::string SwigField::swigClassDecl() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#DEF#$#\n"
        "#^#OPTIONAL#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"MEMBERS", swigMembersDeclImpl()},
        {"DEF", swigClassDeclInternal()},
        {"OPTIONAL", swigOptionalDeclInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigField::swigPublicDecl() const
{
    return swigPublicDeclImpl();
}

std::string SwigField::swigExtraPublicFuncsCode() const
{
    return swigExtraPublicFuncsCodeImpl();
}

void SwigField::swigAddCodeIncludes(StringsList& list) const
{
    if (!comms::genIsGlobalField(m_field)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return;
    }

    if (!m_field.genIsReferenced()) {
        // Code for not referenced does not exist
        return;
    }

    list.push_back(comms::genRelHeaderPathFor(m_field, m_field.genGenerator()));
}

void SwigField::swigAddCode(StringsList& list) const
{
    if (m_codeAdded) {
        return;
    }

    m_codeAdded = true;

    if (comms::genIsGlobalField(m_field) && (!m_field.genIsReferenced())) {
        return;
    }

    swigAddMembersCodeImpl(list);
    list.push_back(swigClassCodeInternal());
}

void SwigField::swigAddDef(StringsList& list) const
{
    if (m_defAdded) {
        return;
    }

    m_defAdded = true;

    bool global = comms::genIsGlobalField(m_field);
    if (global && (!m_field.genIsReferenced())) {
        // Code for not referenced does not exist
        return;
    }

    swigAddVectorTemplateInternal(list);

    swigAddDefImpl(list);

    list.push_back(swigComparisonRenameInternal());

    if (!global) {
        return;
    }

    list.push_back(SwigGenerator::swigDefInclude(comms::genRelHeaderPathFor(m_field, m_field.genGenerator())));
}

std::string SwigField::swigTemplateScope() const
{
    auto& gen = SwigGenerator::cast(m_field.genGenerator());
    return m_field.genTemplateScopeOfComms(SwigProtocolOptions::swigClassName(gen));
}

bool SwigField::swigWrite() const
{
    if (!comms::genIsGlobalField(m_field)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    if (!m_field.genIsReferenced()) {
        // Code for not referenced does not exist
        return true;
    }

    auto& generator = m_field.genGenerator();
    auto filePath = comms::genHeaderPathFor(m_field, generator);
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
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#DEF#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"DEF", swigClassDecl()},
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

std::string SwigField::swigMembersDeclImpl() const
{
    return strings::genEmptyString();
}

std::string SwigField::swigValueTypeDeclImpl() const
{
    return strings::genEmptyString();
}

std::string SwigField::swigValueAccDeclImpl() const
{
    std::string result = 
        "const ValueType& getValue() const;\n";

    if (!m_field.genParseObj().parseIsFixedValue()) {
        result += "void setValue(const ValueType&);\n";
    }

    return result;
}

std::string SwigField::swigExtraPublicFuncsDeclImpl() const
{
    return strings::genEmptyString();
}

std::string SwigField::swigExtraPublicFuncsCodeImpl() const
{
    return strings::genEmptyString();
}

std::string SwigField::swigPublicDeclImpl() const
{
    static const std::string Templ = 
        "#^#VALUE_TYPE#$#\n"
        "#^#VALUE_ACC#$#\n"
        "#^#COMMON_FUNCS#$#\n"
        "#^#EXTRA#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"VALUE_TYPE", swigValueTypeDeclImpl()},
        {"VALUE_ACC", swigValueAccDeclImpl()},
        {"COMMON_FUNCS", swigCommonPublicFuncsDecl()},
        {"EXTRA", swigExtraPublicFuncsDeclImpl()},
    };            

    return util::genProcessTemplate(Templ, repl);
}

void SwigField::swigAddDefImpl([[maybe_unused]] StringsList& list) const
{
}

void SwigField::swigAddMembersCodeImpl([[maybe_unused]] StringsList& list) const
{
}

std::string SwigField::swigCommonPublicFuncsDecl() const
{
    static const std::string Templ = 
        "static const char* name();\n"
        "#^#ERR_STATUS#$# read(const #^#DATA_BUF#$#& buf);\n"
        "#^#ERR_STATUS#$# write(#^#DATA_BUF#$#& buf) const;\n"
        "bool refresh();\n"
        "#^#SIZE_T#$# length() const;\n"
        "bool valid() const;\n"
    ;

    auto& gen = SwigGenerator::cast(m_field.genGenerator());
    util::GenReplacementMap repl = {
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
        {"ERR_STATUS", SwigComms::swigErrorStatusClassName(gen)}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigField::swigCommonPublicFuncsCode() const
{
    static const std::string Templ = 
        "using Base::read;\n"
        "#^#ERR_STATUS#$# read(const #^#DATA_BUF#$#& buf)\n"
        "{\n"
        "    auto iter = buf.begin();\n"
        "    return Base::read(iter, buf.size());\n"
        "}\n\n"
        "using Base::write;\n"
        "#^#ERR_STATUS#$# write(#^#DATA_BUF#$#& buf) const\n"
        "{\n"
        "    auto iter = std::back_inserter(buf);\n"
        "    return Base::write(iter, buf.max_size() - buf.size());\n"
        "}\n"
    ;

    auto& gen = SwigGenerator::cast(m_field.genGenerator());
    util::GenReplacementMap repl = {
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"ERR_STATUS", SwigComms::swigErrorStatusClassName(gen)}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigField::swigSemanticTypeLengthValueAccDecl() const
{
    std::string templ = 
        "#^#SIZE_T#$# getValue() const;\n"
        ;

    if (!m_field.genParseObj().parseIsFixedValue()) {
        templ += "void setValue(#^#SIZE_T#$# val);\n";
    }

    util::GenReplacementMap repl = {
        {"SIZE_T", SwigGenerator::cast(m_field.genGenerator()).swigConvertCppType("std::size_t")},
    };

    return util::genProcessTemplate(templ, repl);
}

std::string SwigField::swigSemanticTypeLengthValueAccCode() const
{
    std::string templ = 
        "#^#SIZE_T#$# getValue() const\n"
        "{\n"
        "    return static_cast<#^#SIZE_T#$#>(Base::getValue());\n"
        "}\n";

    if (!m_field.genParseObj().parseIsFixedValue()) {
        templ += 
            "\n"
            "void setValue(#^#SIZE_T#$# val)\n"
            "{\n"
            "    Base::setValue(val);\n"
            "}\n";        
    }

    util::GenReplacementMap repl = {
        {"SIZE_T", SwigGenerator::cast(m_field.genGenerator()).swigConvertCppType("std::size_t")},
    };

    return util::genProcessTemplate(templ, repl);
}

std::string SwigField::swigClassDeclInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$##^#SUFFIX#$#\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#();\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#(const #^#CLASS_NAME#$##^#SUFFIX#$#&);\n\n"
        "    #^#PUBLIC#$#\n"
        "    #^#CUSTOM#$#\n"
        "};\n\n"
        "// Equality comparison operator is renamed as \"eq_#^#CLASS_NAME#$##^#SUFFIX#$#()\" function by SWIG\n"
        "bool operator==(const #^#CLASS_NAME#$##^#SUFFIX#$#& first, const #^#CLASS_NAME#$##^#SUFFIX#$#& second);\n\n"
        "// Order comparison operator is renamed as \"lt_#^#CLASS_NAME#$##^#SUFFIX#$#()\" function by SWIG\n"
        "bool operator<(const #^#CLASS_NAME#$##^#SUFFIX#$#& first, const #^#CLASS_NAME#$##^#SUFFIX#$#& second);\n"
        ;

    auto& generator = SwigGenerator::cast(m_field.genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", generator.swigClassName(m_field)},
        {"PUBLIC", swigPublicDeclImpl()},
    };

    if (swigIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (comms::genIsGlobalField(m_field)) {
        repl["CUSTOM"] = 
            util::genReadFileContents(generator.swigInputCodePathFor(m_field) + strings::genPublicFileSuffixStr());
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigField::swigOptionalDeclInternal() const
{
    if (!swigIsVersionOptional()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#\n"
        "{\n"
        "    #^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&);\n\n"
        "   #^#COMMON_FUNCS#$#\n"
        "   #^#OPTIONAL_FUNCS#$#\n"
        "};\n\n"
        "// Equality comparison operator is renamed as \"eq_#^#CLASS_NAME#$#()\" function by SWIG\n"
        "bool operator==(const #^#CLASS_NAME#$#& first, const #^#CLASS_NAME#$#& second);\n\n"
        "// Order comparison operator is renamed as \"lt_#^#CLASS_NAME#$#\"() function by SWIG\n"
        "bool operator<(const #^#CLASS_NAME#$#& first, const #^#CLASS_NAME#$#& second);\n"
        ;     

    auto& gen = SwigGenerator::cast(m_field.genGenerator());
    auto className = gen.swigClassName(m_field);
    util::GenReplacementMap repl = {
        {"CLASS_NAME", className},
        {"COMMON_FUNCS", swigCommonPublicFuncsDecl()},
        {"OPTIONAL_FUNCS", SwigOptionalField::swigDeclFuncs(gen, className + strings::genVersionOptionalFieldSuffixStr())},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigField::swigClassCodeInternal() const
{
    auto& gen = SwigGenerator::cast(m_field.genGenerator());

    std::string publicCode = util::genReadFileContents(gen.swigInputCodePathFor(m_field) + strings::genPublicFileSuffixStr());
    std::string protectedCode = util::genReadFileContents(gen.swigInputCodePathFor(m_field) + strings::genProtectedFileSuffixStr());
    std::string privateCode = util::genReadFileContents(gen.swigInputCodePathFor(m_field) + strings::genPrivateFileSuffixStr());
    std::string extraFuncs = swigExtraPublicFuncsCodeImpl();

    if (!protectedCode.empty()) {
        static const std::string TemplTmp = 
            "protected:\n"
            "    #^#CODE#$#\n";

        util::GenReplacementMap replTmp = {
            {"CODE", std::move(protectedCode)}
        };

        protectedCode = util::genProcessTemplate(TemplTmp, replTmp);
    }

    if (!privateCode.empty()) {
        static const std::string TemplTmp = 
            "private:\n"
            "    #^#CODE#$#\n";

        util::GenReplacementMap replTmp = {
            {"CODE", std::move(privateCode)}
        };

        privateCode = util::genProcessTemplate(TemplTmp, replTmp);
    }    

    static const std::string Templ = 
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public #^#COMMS_CLASS#$##^#SUFFIX#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$##^#SUFFIX#$#;\n"
        "public:\n"
        "    #^#EXTRA#$#\n"
        "    #^#COMMON#$#\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n\n"
        "bool operator==(const #^#CLASS_NAME#$##^#SUFFIX#$#& first, const #^#CLASS_NAME#$##^#SUFFIX#$#& second)\n"
        "{\n"
        "    return static_cast<const #^#COMMS_CLASS#$##^#SUFFIX#$#&>(first) == static_cast<const #^#COMMS_CLASS#$##^#SUFFIX#$#&>(second);\n"
        "}\n\n"
        "bool operator<(const #^#CLASS_NAME#$##^#SUFFIX#$#& first, const #^#CLASS_NAME#$##^#SUFFIX#$#& second)\n"
        "{\n"
        "    return static_cast<const #^#COMMS_CLASS#$##^#SUFFIX#$#&>(first) < static_cast<const #^#COMMS_CLASS#$##^#SUFFIX#$#&>(second);\n"
        "}\n"        
        ;

    util::GenReplacementMap repl = {
        {"COMMS_CLASS", swigTemplateScope()},
        {"CLASS_NAME", gen.swigClassName(m_field)},
        {"COMMON", swigCommonPublicFuncsCode()},
        {"EXTRA", std::move(extraFuncs)},
        {"PUBLIC", std::move(publicCode)},
        {"PROTECTED", std::move(protectedCode)},
        {"PRIVATE", std::move(privateCode)}
    };

    if (!swigIsVersionOptional()) {
        return util::genProcessTemplate(Templ, repl);
    }

    repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    repl["FIELD"] = util::genProcessTemplate(Templ, repl);

    static const std::string OptTempl = 
        "#^#FIELD#$#\n"
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#;\n"
        "public:\n"
        "    #^#COMMON#$#\n"
        "};\n";

    return util::genProcessTemplate(OptTempl, repl);
}

std::string SwigField::swigComparisonRenameInternal() const
{
    static const std::string Templ = 
        "%rename(eq_#^#CLASS_NAME#$##^#SUFFIX#$#) operator==(const #^#CLASS_NAME#$##^#SUFFIX#$#&, const #^#CLASS_NAME#$##^#SUFFIX#$#&);\n"
        "%rename(lt_#^#CLASS_NAME#$##^#SUFFIX#$#) operator<(const #^#CLASS_NAME#$##^#SUFFIX#$#&, const #^#CLASS_NAME#$##^#SUFFIX#$#&);";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", SwigGenerator::cast(m_field.genGenerator()).swigClassName(m_field)},
    };

    if (!swigIsVersionOptional()) {
        return util::genProcessTemplate(Templ, repl);
    }    

    auto noSuffix = util::genProcessTemplate(Templ, repl);
    repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    return util::genProcessTemplate(Templ, repl) + '\n' + noSuffix;
}

void SwigField::swigAddVectorTemplateInternal(StringsList& list) const
{
    if (!m_listElement) {
        return;
    }

    static const std::string Templ = 
        "%template(#^#CLASS_NAME#$#_Vector) std::vector<#^#CLASS_NAME#$#>;";

    auto& gen = SwigGenerator::cast(m_field.genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(m_field)},
    };    

    list.push_back(util::genProcessTemplate(Templ, repl));
}

} // namespace commsdsl2swig
