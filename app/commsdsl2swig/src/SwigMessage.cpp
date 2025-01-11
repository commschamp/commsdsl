//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "SwigMessage.h"

#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigMsgHandler.h"
#include "SwigMsgId.h"
#include "SwigProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{


SwigMessage::SwigMessage(SwigGenerator& generator, commsdsl::parse::Message dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

SwigMessage::~SwigMessage() = default;

void SwigMessage::swigAddCodeIncludes(StringsList& list) const
{
    if (!isReferenced()) {
        return;
    }

    list.push_back(comms::relHeaderPathFor(*this, generator()));
}

void SwigMessage::swigAddCode(StringsList& list) const
{
    if (!isReferenced()) {
        return;
    }

    for (auto* f : m_swigFields) {
        f->swigAddCode(list);
    }

    auto& gen = SwigGenerator::cast(generator());
    auto* mainInterface = gen.swigMainInterface();
    assert(mainInterface != nullptr);

    util::ReplacementMap repl = {
        {"COMMS_CLASS", comms::scopeFor(*this, gen)},
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*mainInterface)},
        {"MSG_HANDLER", SwigMsgHandler::swigClassName(gen)}
    };

    if (SwigProtocolOptions::swigIsDefined(gen)) {
        repl["OPTS"] = ", " + SwigProtocolOptions::swigClassName(gen);
    }

    std::string publicCode = util::readFileContents(gen.swigInputCodePathFor(*this) + strings::publicFileSuffixStr());
    std::string protectedCode = util::readFileContents(gen.swigInputCodePathFor(*this) + strings::protectedFileSuffixStr());
    std::string privateCode = util::readFileContents(gen.swigInputCodePathFor(*this) + strings::privateFileSuffixStr());

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
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#<#^#INTERFACE#$##^#OPTS#$#>\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#<#^#INTERFACE#$##^#OPTS#$#>;\n"
        "public:\n"
        "    #^#FIELDS#$#\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n";

    repl.insert({
        {"FIELDS", swigFieldsAccCodeInternal()},
        {"PUBLIC", std::move(publicCode)},
        {"PROTECTED", std::move(protectedCode)},
        {"PRIVATE", std::move(privateCode)}
    });

    list.push_back(util::processTemplate(Templ, repl));    
}

void SwigMessage::swigAddDef(StringsList& list) const
{
    if (!isReferenced()) {
        return;
    }

    for (auto* f : m_swigFields) {
        f->swigAddDef(list);
    }

    static const std::string Templ = 
        "%rename(eq_#^#CLASS_NAME#$#) operator==(const #^#CLASS_NAME#$#&, const #^#CLASS_NAME#$#&);";

    util::ReplacementMap repl = {
        {"CLASS_NAME", SwigGenerator::cast(generator()).swigClassName(*this)},
    };

    list.push_back(util::processTemplate(Templ, repl));

    list.push_back(SwigGenerator::swigDefInclude(comms::relHeaderPathFor(*this, generator())));
}

bool SwigMessage::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_swigFields = SwigField::swigTransformFieldsList(fields());
    return true;
}

bool SwigMessage::writeImpl() const
{
    assert(isReferenced());
    auto filePath = comms::headerPathFor(*this, generator());
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator().createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator().logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#FIELDS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"FIELDS", swigFieldDefsInternal()},
        {"DEF", swigClassDeclInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

std::string SwigMessage::swigFieldDefsInternal() const
{
    StringsList fields;
    fields.reserve(m_swigFields.size());

    for (auto* f : m_swigFields) {
        fields.push_back(f->swigClassDecl());
    }

    return util::strListToString(fields, "\n", "");
}

std::string SwigMessage::swigClassDeclInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$# : public #^#INTERFACE#$#\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#&);\n\n"
        "    #^#FIELDS#$#\n"
        "    #^#CUSTOM#$#\n"
        "};\n\n"
        "// Equality comparison operator is renamed as \"eq_#^#CLASS_NAME#$#()\" function by SWIG\n"
        "bool operator==(const #^#CLASS_NAME#$#& first, const #^#CLASS_NAME#$#& second);\n";

    auto& gen = SwigGenerator::cast(generator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*iFace)},
        {"FIELDS", swigFieldsAccDeclInternal()},
        {"CUSTOM", util::readFileContents(gen.swigInputCodePathFor(*this) + strings::publicFileSuffixStr())},
    };

    return util::processTemplate(Templ, repl);    
}

std::string SwigMessage::swigFieldsAccDeclInternal() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigFields.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* f : m_swigFields) {
        static const std::string Templ = {
            "#^#CLASS_NAME#$#& field_#^#ACC_NAME#$#();\n"
        };

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(f->field())},
            {"ACC_NAME", comms::accessName(f->field().dslObj().name())}
        };

        accFuncs.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(accFuncs, "\n", "");
}

std::string SwigMessage::swigFieldsAccCodeInternal() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigFields.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* f : m_swigFields) {
        static const std::string Templ = {
            "#^#CLASS_NAME#$#& field_#^#ACC_NAME#$#()\n"
            "{\n"
            "    return static_cast<#^#CLASS_NAME#$#&>(Base::field_#^#ACC_NAME#$#());\n"
            "}\n"
            "const #^#CLASS_NAME#$#& field_#^#ACC_NAME#$#() const\n"
            "{\n"
            "    return static_cast<const #^#CLASS_NAME#$#&>(Base::field_#^#ACC_NAME#$#());\n"
            "}\n"            
        };

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(f->field())},
            {"ACC_NAME", comms::accessName(f->field().dslObj().name())}
        };

        accFuncs.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(accFuncs, "\n", "");
}

} // namespace commsdsl2swig
