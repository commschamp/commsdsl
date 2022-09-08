//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "SwigGenerator.h"
#include "SwigOptionalField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <algorithm>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;


namespace commsdsl2swig
{

SwigField::SwigField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

SwigField::~SwigField() = default;

const SwigField* SwigField::cast(const commsdsl::gen::Field* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    auto* swigField = dynamic_cast<const SwigField*>(field);    
    assert(swigField != nullptr);
    return swigField;
}

SwigField::SwigFieldsList SwigField::swigTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields)
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
    return comms::relHeaderPathFor(m_field, m_field.generator());
}

bool SwigField::swigIsVersionOptional() const
{
    return comms::isVersionOptionaField(m_field, m_field.generator());
}

std::string SwigField::swigClassDecl() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#DEF#$#\n"
        "#^#OPTIONAL#$#\n"
    ;

    util::ReplacementMap repl = {
        {"MEMBERS", swigMembersDeclImpl()},
        {"DEF", swigClassDeclInternal()},
        {"OPTIONAL", swigOptionalDeclInternal()},
    };

    return util::processTemplate(Templ, repl);
}

void SwigField::swigAddCodeIncludes(StringsList& list) const
{
    if (!comms::isGlobalField(m_field)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_field.isReferenced())) {
        // Code for not referenced does not exist
        return;
    }

    list.push_back(comms::relHeaderPathFor(m_field, m_field.generator()));
}

void SwigField::swigAddCode(StringsList& list) const
{
    if (m_codeAdded) {
        return;
    }

    m_codeAdded = true;

    if (comms::isGlobalField(m_field) && (!m_field.isReferenced()) && (!m_field.dslObj().isForceGen())) {
        return;
    }

    swigAddCodeImpl(list);
    list.push_back(swigClassCodeInternal());
}

void SwigField::swigAddDef(StringsList& list) const
{
    swigAddDefImpl(list);
    
    if (!comms::isGlobalField(m_field)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_field.isReferenced())) {
        // Code for not referenced does not exist
        return;
    }

    list.push_back(SwigGenerator::swigDefInclude(comms::relHeaderPathFor(m_field, m_field.generator())));
}

bool SwigField::swigWrite() const
{
    if (!comms::isGlobalField(m_field)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_field.isReferenced())) {
        // Code for not referenced does not exist
        return true;
    }

    auto& generator = m_field.generator();
    auto filePath = comms::headerPathFor(m_field, generator);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"DEF", swigClassDecl()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();    
}

std::string SwigField::swigBaseClassDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigField::swigMembersDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigField::swigValueTypeDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigField::swigValueAccDeclImpl() const
{
    return std::string(
        "const ValueType& getValue() const;\n"
        "void setValue(const ValueType&);\n"
    );
}

std::string SwigField::swigExtraPublicFuncsDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigField::swigExtraPublicFuncsCodeImpl() const
{
    return strings::emptyString();
}

std::string SwigField::swigCommonPublicFuncsDeclImpl() const
{
    return swigCommonPublicFuncsDecl();
}

void SwigField::swigAddDefImpl(StringsList& list) const
{
    static_cast<void>(list);
}

void SwigField::swigAddCodeImpl(StringsList& list) const
{
    static_cast<void>(list);
}

std::string SwigField::swigCommonPublicFuncsDecl() const
{
    static const std::string Templ = 
        "static const char* name();\n"
        "comms_ErrorStatus read(const #^#UINT8_T#$#*& iter, #^#SIZE_T#$# len);\n"
        "comms_ErrorStatus write(#^#UINT8_T#$#*& iter, #^#SIZE_T#$# len) const;\n"
        "bool refresh();\n"
        "#^#SIZE_T#$# length() const;\n"
        "bool valid() const;\n"
    ;

    auto& gen = SwigGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"UINT8_T", gen.swigConvertCppType("std::uint8_t")},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigField::swigClassDeclInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$##^#SUFFIX#$##^#PUBLIC#$##^#BASE#$#\n"
        "{\n"
        "public:\n"
        "    #^#VALUE_TYPE#$#\n"
        "    #^#VALUE_ACC#$#\n"
        "    #^#COMMON_FUNCS#$#\n"
        "    #^#EXTRA#$#\n"
        "    #^#CUSTOM#$#\n"
        "};\n";

    auto& generator = SwigGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.swigClassName(m_field)},
        {"VALUE_TYPE", swigValueTypeDeclImpl()},
        {"VALUE_ACC", swigValueAccDeclImpl()},
        {"COMMON_FUNCS", swigCommonPublicFuncsDeclImpl()},
        {"EXTRA", swigExtraPublicFuncsDeclImpl()},
        {"BASE", swigBaseClassDeclImpl()}
    };

    if (swigIsVersionOptional()) {
        repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();
    }

    if (comms::isGlobalField(m_field)) {
        repl["CUSTOM"] = 
            util::readFileContents(generator.swigInputCodePathFor(m_field) + strings::publicFileSuffixStr());  
    }

    if (!repl["BASE"].empty()) {
        repl["PUBLIC"] = " : public ";
    }

    return util::processTemplate(Templ, repl);
}

std::string SwigField::swigOptionalDeclInternal() const
{
    if (!swigIsVersionOptional()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#\n"
        "{\n"
        "   #^#COMMON_FUNCS#$#\n"
        "   #^#OPTIONAL_FUNCS#$#\n"
        "};\n";     

    auto& generator = SwigGenerator::cast(m_field.generator());
    auto className = generator.swigClassName(m_field);
    util::ReplacementMap repl = {
        {"CLASS_NAME", className},
        {"COMMON_FUNCS", swigCommonPublicFuncsDecl()},
        {"OPTIONAL_FUNCS", SwigOptionalField::swigDefFuncs(className + strings::versionOptionalFieldSuffixStr())},
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigField::swigClassCodeInternal() const
{
    auto& gen = SwigGenerator::cast(m_field.generator());

    util::ReplacementMap repl = {
        {"COMMS_CLASS", swigTemplateScopeInternal()},
        {"CLASS_NAME", gen.swigClassName(m_field)}
    };


    std::string publicCode = util::readFileContents(gen.swigInputCodePathFor(m_field) + strings::publicFileSuffixStr());
    std::string protectedCode = util::readFileContents(gen.swigInputCodePathFor(m_field) + strings::protectedFileSuffixStr());
    std::string privateCode = util::readFileContents(gen.swigInputCodePathFor(m_field) + strings::privateFileSuffixStr());
    std::string extraFuncs = swigExtraPublicFuncsCodeImpl();

    if (publicCode.empty() && protectedCode.empty() && privateCode.empty() && extraFuncs.empty()) {
        static const std::string Templ = 
            "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$# {};\n";

        return util::processTemplate(Templ, repl);
    }

    if (!protectedCode.empty()) {
        static const std::string TemplTmp = 
            "protected:\n"
            "    #^#CODE#$#\n";

        util::ReplacementMap replTmp = {
            {"CODE", std::move(protectedCode)}
        };

        protectedCode = util::processTemplate(TemplTmp, replTmp);
    }

    if (!privateCode.empty()) {
        static const std::string TemplTmp = 
            "private:\n"
            "    #^#CODE#$#\n";

        util::ReplacementMap replTmp = {
            {"CODE", std::move(privateCode)}
        };

        privateCode = util::processTemplate(TemplTmp, replTmp);
    }    

    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#;\n"
        "public:\n"
        "    #^#EXTRA#$#\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n";

    repl.insert({
        {"EXTRA", std::move(extraFuncs)},
        {"PUBLIC", std::move(publicCode)},
        {"PROTECTED", std::move(protectedCode)},
        {"PRIVATE", std::move(privateCode)}
    });

    return util::processTemplate(Templ, repl);    
}

std::string SwigField::swigTemplateScopeInternal() const
{
    static const std::string TemplParams = "<>";

    auto& gen = SwigGenerator::cast(m_field.generator());
    auto commsScope = comms::scopeFor(m_field, gen);

    if (comms::isGlobalField(m_field)) {
        return commsScope + TemplParams;
    }

    using Elem = commsdsl::gen::Elem;

    auto formScopeFunc = 
        [&commsScope, &gen](const Elem* parent, const std::string& suffix)
        {
            auto optLevelScope = comms::scopeFor(*parent, gen) + suffix;
            assert(optLevelScope.size() < commsScope.size());
            assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
            
            return optLevelScope + TemplParams + commsScope.substr(optLevelScope.size());
        };

    
    Elem* parent = m_field.getParent();
    while (parent != nullptr)  {
        auto elemType = parent->elemType();

        if (elemType == Elem::Type_Interface) {
            return commsScope;
        }        

        if (elemType == Elem::Type_Message) {

            return formScopeFunc(parent, strings::fieldsSuffixStr());
        }

        if (elemType == Elem::Type_Frame) {
            return formScopeFunc(parent, strings::layersSuffixStr());
        }        

        parent = parent->getParent();
    }

    assert(false); // Should not happen
    return commsScope;
}

} // namespace commsdsl2swig
