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

#include "EmscriptenField.h"

// #include "EmscriptenComms.h"
#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
// #include "EmscriptenOptionalField.h"
#include "EmscriptenProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <algorithm>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;


namespace commsdsl2emscripten
{

EmscriptenField::EmscriptenField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

EmscriptenField::~EmscriptenField() = default;

const EmscriptenField* EmscriptenField::cast(const commsdsl::gen::Field* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    auto* emscriptenField = dynamic_cast<const EmscriptenField*>(field);    
    assert(emscriptenField != nullptr);
    return emscriptenField;
}

EmscriptenField::EmscriptenFieldsList EmscriptenField::emscriptenTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields)
{
    EmscriptenFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* emscriptenField = 
            const_cast<EmscriptenField*>(
                dynamic_cast<const EmscriptenField*>(fPtr.get()));

        assert(emscriptenField != nullptr);
        result.push_back(emscriptenField);
    }

    return result;    
}

std::string EmscriptenField::emscriptenRelHeaderPath() const
{
    return comms::relHeaderPathFor(m_field, m_field.generator());
}

bool EmscriptenField::emscriptenIsVersionOptional() const
{
    return comms::isVersionOptionaField(m_field, m_field.generator());
}

// std::string EmscriptenField::emscriptenClassDecl() const
// {
//     static const std::string Templ = 
//         "#^#MEMBERS#$#\n"
//         "#^#DEF#$#\n"
//         "#^#OPTIONAL#$#\n"
//     ;

//     util::ReplacementMap repl = {
//         {"MEMBERS", emscriptenMembersDeclImpl()},
//         {"DEF", emscriptenClassDeclInternal()},
//         {"OPTIONAL", emscriptenOptionalDeclInternal()},
//     };

//     return util::processTemplate(Templ, repl);
// }

// std::string EmscriptenField::emscriptenPublicDecl() const
// {
//     return emscriptenPublicDeclImpl();
// }

// std::string EmscriptenField::emscriptenExtraPublicFuncsCode() const
// {
//     return emscriptenExtraPublicFuncsCodeImpl();
// }

// std::string EmscriptenField::emscriptenTemplateScope() const
// {
//     auto& gen = EmscriptenGenerator::cast(m_field.generator());
//     auto commsScope = comms::scopeFor(m_field, gen);
//     std::string optionsParams = "<" + EmscriptenProtocolOptions::emscriptenClassName(gen) + ">";

//     if (comms::isGlobalField(m_field)) {
//         return commsScope + optionsParams;
//     }

//     using Elem = commsdsl::gen::Elem;

//     auto formScopeFunc = 
//         [&commsScope, &gen, &optionsParams](const Elem* parent, const std::string& suffix)
//         {
//             auto optLevelScope = comms::scopeFor(*parent, gen) + suffix;
//             assert(optLevelScope.size() < commsScope.size());
//             assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
            
//             return optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());
//         };

    
//     Elem* parent = m_field.getParent();
//     while (parent != nullptr)  {
//         auto elemType = parent->elemType();

//         if (elemType == Elem::Type_Interface) {
//             return commsScope;
//         }        

//         if ((elemType == Elem::Type_Field) && (comms::isGlobalField(*parent))) {
//             return formScopeFunc(parent, strings::membersSuffixStr());
//         }        

//         if (elemType == Elem::Type_Message) {
//             return formScopeFunc(parent, strings::fieldsSuffixStr());
//         }

//         if (elemType == Elem::Type_Frame) {
//             return formScopeFunc(parent, strings::layersSuffixStr());
//         }        

//         parent = parent->getParent();
//     }

//     assert(false); // Should not happen
//     return commsScope;
// }

bool EmscriptenField::emscriptenWrite() const
{
    if (!comms::isGlobalField(m_field)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    if (!m_field.isReferenced()) {
        // Code for not referenced does not exist
        return true;
    }

    return 
        emscriptenWriteHeaderInternal() &&
        emscriptenWriteSrcInternal();
}

std::string EmscriptenField::emscriptenHeaderClass() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#DEF#$#\n"
        "#^#OPTIONAL#$#\n"
    ;

    util::ReplacementMap repl = {
        {"MEMBERS", emscriptenHeaderMembersImpl()},
        {"DEF", emscriptenHeaderClassInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenTemplateScope() const
{
    auto& gen = EmscriptenGenerator::cast(m_field.generator());
    auto commsScope = comms::scopeFor(m_field, gen);
    std::string optionsParams = "<" + EmscriptenProtocolOptions::emscriptenClassName(gen) + ">";

    if (comms::isGlobalField(m_field)) {
        return commsScope + optionsParams;
    }

    using Elem = commsdsl::gen::Elem;

    auto formScopeFunc = 
        [&commsScope, &gen, &optionsParams](const Elem* parent, const std::string& suffix)
        {
            auto optLevelScope = comms::scopeFor(*parent, gen) + suffix;
            assert(optLevelScope.size() < commsScope.size());
            assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
            
            return optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());
        };

    
    Elem* parent = m_field.getParent();
    while (parent != nullptr)  {
        auto elemType = parent->elemType();

        if (elemType == Elem::Type_Interface) {
            return commsScope;
        }        

        if ((elemType == Elem::Type_Field) && (comms::isGlobalField(*parent))) {
            return formScopeFunc(parent, strings::membersSuffixStr());
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

std::string EmscriptenField::emscriptenHeaderMembersImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenHeaderCommonPublicFuncs() const
{
    static const std::string Templ = 
        "comms::ErrorStatus read(const #^#DATA_BUF#$#& buf);\n"
        "comms::ErrorStatus write(#^#DATA_BUF#$#& buf) const;\n"
        "bool refresh();\n"
        "std::size_t length();\n"
        "bool valid() const;\n"
        "const char* name() const;"
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(generator)}
    };
    
    return util::processTemplate(Templ, repl);
}

bool EmscriptenField::emscriptenWriteHeaderInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
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
        "#^#GENERATED#$#\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#CLASS#$#\n"
        "#^#APPEND#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"INCLUDES", emscriptenHeaderIncludesInternal()},
        {"CLASS", emscriptenHeaderClass()},
        {"APPEND", util::readFileContents(comms::inputCodePathFor(m_field, generator) + strings::appendFileSuffixStr())}
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

bool EmscriptenField::emscriptenWriteSrcInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    auto filePath = comms::sourcePathFor(m_field, generator);
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
        // TODO
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        // TODO
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

std::string EmscriptenField::emscriptenHeaderIncludesInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    StringsList includes = {
        comms::relHeaderPathFor(m_field, generator),
        EmscriptenDataBuf::emscriptenRelHeader(generator)
    };

    EmscriptenProtocolOptions::emscriptenAddInclude(generator, includes);

    comms::prepareIncludeStatement(includes);
    auto result = util::strListToString(includes, "\n", "\n");
    result.append(util::readFileContents(comms::inputCodePathFor(m_field, generator) + strings::incFileSuffixStr()));
    return result;
}

std::string EmscriptenField::emscriptenHeaderClassInternal() const
{
    auto& gen = EmscriptenGenerator::cast(m_field.generator());

    auto inputCodePrefix = comms::inputCodePathFor(m_field, gen);
    std::string publicCode = util::readFileContents(inputCodePrefix + strings::publicFileSuffixStr());
    std::string protectedCode = util::readFileContents(inputCodePrefix + strings::protectedFileSuffixStr());
    std::string privateCode = util::readFileContents(inputCodePrefix + strings::privateFileSuffixStr());
    std::string extraFuncs = emscriptenHeaderExtraPublicFuncsImpl();

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
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public #^#COMMS_CLASS#$##^#SUFFIX#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$##^#SUFFIX#$#;\n"
        "public:\n"
        "    #^#EXTRA#$#\n"
        "    #^#COMMON#$#\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n";

    util::ReplacementMap repl = {
        {"COMMS_CLASS", emscriptenTemplateScope()},
        {"CLASS_NAME", gen.emscriptenClassName(m_field)},
        {"COMMON", emscriptenHeaderCommonPublicFuncs()},
        {"EXTRA", std::move(extraFuncs)},
        {"PUBLIC", std::move(publicCode)},
        {"PROTECTED", std::move(protectedCode)},
        {"PRIVATE", std::move(privateCode)}
    };

    if (!emscriptenIsVersionOptional()) {
        return util::processTemplate(Templ, repl);
    }

    repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();
    repl["FIELD"] = util::processTemplate(Templ, repl);

    static const std::string OptTempl = 
        "#^#FIELD#$#\n"
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#;\n"
        "public:\n"
        "    #^#COMMON#$#\n"
        "};\n";

    return util::processTemplate(OptTempl, repl);
}


} // namespace commsdsl2emscripten
