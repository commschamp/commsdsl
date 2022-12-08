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
// #include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
// #include "EmscriptenOptionalField.h"
// #include "EmscriptenProtocolOptions.h"

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

} // namespace commsdsl2emscripten
