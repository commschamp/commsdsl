//
// Copyright 2022 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenOptionalField.h"
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

EmscriptenField* EmscriptenField::cast(commsdsl::gen::Field* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    auto* emscriptenField = dynamic_cast<EmscriptenField*>(field);    
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
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    return generator.emscriptenRelHeaderFor(m_field);
}

bool EmscriptenField::emscriptenIsVersionOptional() const
{
    return comms::isVersionOptionalField(m_field, m_field.generator());
}

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
        "#^#EXTRA#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"MEMBERS", emscriptenHeaderMembersInternal()},
        {"EXTRA", emscriptenHeaderExtraCodePrefixImpl()},
        {"DEF", emscriptenHeaderClassInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenTemplateScope() const
{
    auto& gen = EmscriptenGenerator::cast(m_field.generator());
    return m_field.templateScopeOfComms(EmscriptenProtocolOptions::emscriptenClassName(gen));
}

std::string EmscriptenField::emscriptenSourceCode() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#EXTRA#$#\n"
        "#^#BIND#$#\n";       

    util::ReplacementMap repl = {
        {"MEMBERS", emscriptenSourceMembersInternal()},
        {"EXTRA", emscriptenSourceExtraCodeInternal()},
        {"BIND", emscriptenSourceBindInternal()},
    };

    return util::processTemplate(Templ, repl);
}

void EmscriptenField::emscriptenHeaderAddExtraIncludes(StringsList& incs) const
{
    for (auto* m : m_members) {
        m->emscriptenHeaderAddExtraIncludes(incs);
    }

    emscriptenHeaderAddExtraIncludesImpl(incs);
}

void EmscriptenField::emscriptenAddSourceFiles(StringsList& sources) const
{
    if ((!comms::isGlobalField(m_field)) || (!m_field.isReferenced())) {
        return;
    }

    auto& gen = EmscriptenGenerator::cast(m_field.generator());
    sources.push_back(gen.emscriptenRelSourceFor(m_field));
}

void EmscriptenField::emscriptenHeaderAddExtraIncludesImpl([[maybe_unused]] StringsList& incs) const
{
}

std::string EmscriptenField::emscriptenHeaderExtraCodePrefixImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenHeaderValueAccImpl() const
{
    return emscriptenHeaderValueAccByRef();
}

std::string EmscriptenField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenSourceExtraCodeImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenSourceBindValueAccImpl() const
{
    return emscriptenSourceBindValueAcc();
}

std::string EmscriptenField::emscriptenSourceBindFuncsImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenSourceBindExtraImpl() const
{
    return strings::emptyString();
}

void EmscriptenField::emscriptenAssignMembers(const commsdsl::gen::Field::FieldsList& fields)
{
    m_members = emscriptenTransformFieldsList(fields);
}

void EmscriptenField::emscriptenAddMember(commsdsl::gen::Field* field)
{
    if (field == nullptr) {
        return;
    }

    auto* emscriptenField = cast(field);
    assert(emscriptenField != nullptr);
    m_members.push_back(emscriptenField);
}

std::string EmscriptenField::emscriptenHeaderValueAccByRef()
{
    static const std::string Templ = 
        "const ValueType& getValue() const\n"
        "{\n"
        "    return Base::getValue();\n"
        "}\n\n"
        "void setValue(const ValueType& val)\n"
        "{\n"
        "    Base::setValue(val);\n"
        "}\n";        

    return Templ;
}

std::string EmscriptenField::emscriptenHeaderValueAccByValue()
{
    static const std::string Templ = 
        "ValueType getValue() const\n"
        "{\n"
        "    return Base::getValue();\n"
        "}\n\n"
        "void setValue(ValueType val)\n"
        "{\n"
        "    Base::setValue(val);\n"
        "}\n";        

    return Templ;
}

std::string EmscriptenField::emscriptenHeaderValueAccLengthField()
{
    static const std::string Templ = 
        "std::size_t getValue() const\n"
        "{\n"
        "    return Base::getValue();\n"
        "}\n\n"
        "void setValue(std::size_t val)\n"
        "{\n"
        "    Base::setValue(val);\n"
        "}\n";        

    return Templ;
}

std::string EmscriptenField::emscriptenHeaderValueAccByPointer()
{
    static const std::string Templ = 
        "const ValueType* getValue() const\n"
        "{\n"
        "    return &Base::getValue();\n"
        "}\n\n"
        "void setValue(const ValueType& val)\n"
        "{\n"
        "    Base::setValue(val);\n"
        "}\n";        

    return Templ;
}

std::string EmscriptenField::emscriptenHeaderValueStorageAccByPointer()
{
    static const std::string Templ = 
        "ValueType* value()\n"
        "{\n"
        "    return &Base::value();\n"
        "}\n";

    return Templ;
}

std::string EmscriptenField::emscriptenSourceBindValueAcc() const
{
    static const std::string Templ = 
        ".property(\"value\", &#^#CLASS_NAME#$#::getValue, &#^#CLASS_NAME#$#::setValue)\n"
        ".function(\"getValue\", &#^#CLASS_NAME#$#::getValue)\n"
        ".function(\"setValue\", &#^#CLASS_NAME#$#::setValue)"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenSourceBindValueAccByPointer() const
{
    static const std::string Templ = 
        ".function(\"getValue\", &#^#CLASS_NAME#$#::getValue, emscripten::allow_raw_pointers())\n"
        ".function(\"setValue\", &#^#CLASS_NAME#$#::setValue)"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenSourceBindValueStorageAccByPointer() const
{
    static const std::string Templ = 
        ".function(\"value\", &#^#CLASS_NAME#$#::value, emscripten::allow_raw_pointers())"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenBindClassName(bool checkVersionOptional) const
{
    auto& gen = EmscriptenGenerator::cast(m_field.generator());
    auto result = gen.emscriptenClassName(m_field);

    if (checkVersionOptional && emscriptenIsVersionOptional()) {
        result.append(strings::versionOptionalFieldSuffixStr());
    }    

    return result;
}

std::string EmscriptenField::emscriptenMembersAccessFuncs() const
{
    auto& gen = EmscriptenGenerator::cast(m_field.generator());
    util::StringsList fields;
    for (auto* f : emscriptenMembers()) {
        static const std::string Templ = 
            "using Base::field_#^#NAME#$#;\n"
            "#^#FIELD_CLASS#$#* field_#^#NAME#$#_()\n"
            "{\n"
            "    return static_cast<#^#FIELD_CLASS#$#*>(&field_#^#NAME#$#());\n"
            "}\n";

        util::ReplacementMap repl = {
            {"FIELD_CLASS", gen.emscriptenClassName(f->field())},
            {"NAME", comms::accessName(f->field().dslObj().name())},
        };

        fields.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(fields, "\n", "");
}

std::string EmscriptenField::emscriptenMembersBindFuncs() const
{
    util::StringsList fields;

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
    };

    for (auto* f : emscriptenMembers()) {
        static const std::string Templ = 
            ".function(\"field_#^#NAME#$#\", &#^#CLASS_NAME#$#::field_#^#NAME#$#_, emscripten::allow_raw_pointers())";

        repl["NAME"] = comms::accessName(f->field().dslObj().name());
        fields.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(fields, "\n", "");
}

bool EmscriptenField::emscriptenWriteHeaderInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    auto filePath = generator.emscriptenAbsHeaderFor(m_field);
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
        {"APPEND", util::readFileContents(generator.emspriptenInputAbsHeaderFor(m_field) + strings::appendFileSuffixStr())}
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

bool EmscriptenField::emscriptenWriteSrcInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    auto filePath = generator.emscriptenAbsSourceFor(m_field);
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
        "#^#INCLUDES#$#\n"
        "#^#CODE#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"INCLUDES", emscriptenSourceIncludesInternal()},
        {"CODE", emscriptenSourceCode()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

std::string EmscriptenField::emscriptenHeaderIncludesInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    StringsList includes = {
        "<iterator>",
        comms::relHeaderPathFor(m_field, generator),
        EmscriptenDataBuf::emscriptenRelHeader(generator)
    };

    EmscriptenProtocolOptions::emscriptenAddInclude(generator, includes);

    emscriptenHeaderAddExtraIncludes(includes);

    for (auto* m : m_members) {
        m->emscriptenHeaderAddExtraIncludes(includes);
    }

    comms::prepareIncludeStatement(includes);
    auto result = util::strListToString(includes, "\n", "\n");
    result.append(util::readFileContents(generator.emspriptenInputAbsHeaderFor(m_field) + strings::incFileSuffixStr()));
    return result;
}

std::string EmscriptenField::emscriptenHeaderClassInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());

    std::string publicCode;
    std::string protectedCode;
    std::string privateCode;
    if (comms::isGlobalField(m_field)) {
        auto inputCodePrefix = generator.emspriptenInputAbsHeaderFor(m_field);
        publicCode = util::readFileContents(inputCodePrefix + strings::publicFileSuffixStr());
        protectedCode = util::readFileContents(inputCodePrefix + strings::protectedFileSuffixStr());
        privateCode = util::readFileContents(inputCodePrefix + strings::privateFileSuffixStr());
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
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public #^#COMMS_CLASS#$##^#SUFFIX#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$##^#SUFFIX#$#;\n"
        "public:\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#() = default;\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#(const #^#CLASS_NAME#$##^#SUFFIX#$#&) = default;\n"
        "    ~#^#CLASS_NAME#$##^#SUFFIX#$#() = default;\n\n"
        "    #^#VALUE_ACC#$#\n"
        "    #^#EXTRA#$#\n"
        "    #^#COMMON#$#\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n\n"
        "inline bool eq_#^#CLASS_NAME#$##^#SUFFIX#$#(const #^#CLASS_NAME#$##^#SUFFIX#$#& first, const #^#CLASS_NAME#$##^#SUFFIX#$#& second)\n"
        "{\n"
        "    return first == second;\n"
        "}\n\n"
        "inline bool lt_#^#CLASS_NAME#$##^#SUFFIX#$#(const #^#CLASS_NAME#$##^#SUFFIX#$#& first, const #^#CLASS_NAME#$##^#SUFFIX#$#& second)\n"
        "{\n"
        "    return first < second;\n"
        "}\n"        
        ;

    util::ReplacementMap repl = {
        {"COMMS_CLASS", emscriptenTemplateScope()},
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"COMMON", emscriptenHeaderCommonPublicFuncsInternal()},
        {"VALUE_ACC", emscriptenHeaderValueAccImpl()},
        {"EXTRA", emscriptenHeaderExtraPublicFuncsImpl()},
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
        "    using Base = #^#COMMS_CLASS#$#;\n\n"
        "public:\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#* field()\n"
        "    {\n"
        "        return static_cast<#^#CLASS_NAME#$##^#SUFFIX#$#*>(&(Base::field()));\n"
        "    }\n\n"
        "    #^#COMMON#$#\n"
        "    #^#COMMON_OPTIONAL#$#\n"
        "};\n\n"
        "inline bool eq_#^#CLASS_NAME#$#(const #^#CLASS_NAME#$#& first, const #^#CLASS_NAME#$#& second)\n"
        "{\n"
        "    return first == second;\n"
        "}\n\n"
        "inline bool lt_#^#CLASS_NAME#$#(const #^#CLASS_NAME#$#& first, const #^#CLASS_NAME#$#& second)\n"
        "{\n"
        "    return first < second;\n"
        "}\n"        
        ;

    repl["COMMON_OPTIONAL"] = EmscriptenOptionalField::emscriptenHeaderCommonModeFuncs();

    return util::processTemplate(OptTempl, repl);
}

std::string EmscriptenField::emscriptenHeaderCommonPublicFuncsInternal() const
{
    static const std::string Templ = 
        "comms::ErrorStatus readDataBuf(const #^#DATA_BUF#$#& buf)\n"
        "{\n"
        "    auto iter = buf.begin();\n"
        "    return Base::read(iter, buf.size());\n"
        "}\n\n"   
        "comms::ErrorStatus readJsArray(const emscripten::val& jsArray)\n"
        "{\n"
        "    auto dataBuf = #^#JS_ARRAY_FUNC#$#(jsArray);\n"
        "    return readDataBuf(dataBuf);\n"
        "}\n\n"              
        "comms::ErrorStatus writeDataBuf(#^#DATA_BUF#$#& buf) const\n"
        "{\n"
        "    auto iter = std::back_inserter(buf);\n"
        "    return Base::write(iter, buf.max_size());\n"
        "}\n\n"        
        "bool refresh()\n"
        "{\n"
        "    return Base::refresh();\n"
        "}\n\n"
        "std::size_t length() const\n"
        "{\n"
        "    return Base::length();\n"
        "}\n\n"
        "bool valid() const\n"
        "{\n"
        "    return Base::valid();\n"
        "}\n\n"
        "std::string name() const\n"
        "{\n"
        "    return std::string(Base::name());\n"
        "}\n"
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(generator)},
        {"JS_ARRAY_FUNC", EmscriptenDataBuf::emscriptenJsArrayToDataBufFuncName()},
    };
    
    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenSourceIncludesInternal() const
{
    StringsList includes = {
        "<emscripten/bind.h>",
        emscriptenRelHeaderPath()
    };

    comms::prepareIncludeStatement(includes);
    return util::strListToString(includes, "\n", "\n");
}

std::string EmscriptenField::emscriptenSourceBindInternal() const
{
    static const std::string Templ = 
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$##^#SUFFIX#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$##^#SUFFIX#$#>(\"#^#CLASS_NAME#$##^#SUFFIX#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$##^#SUFFIX#$#&>()\n"
        "        #^#VALUE_ACC#$#\n"
        "        #^#FUNCS#$#\n"
        "        #^#COMMON#$#\n"
        "        #^#CUSTOM#$#\n"
        "        ;\n"
        "    emscripten::function(\"eq_#^#CLASS_NAME#$##^#SUFFIX#$#\", &eq_#^#CLASS_NAME#$##^#SUFFIX#$#);\n"
        "    emscripten::function(\"lt_#^#CLASS_NAME#$##^#SUFFIX#$#\", &lt_#^#CLASS_NAME#$##^#SUFFIX#$#);\n"
        "    #^#VECTOR#$#\n"
        "    #^#EXTRA#$#\n"
        "}\n"
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"VALUE_ACC", emscriptenSourceBindValueAccImpl()},
        {"FUNCS", emscriptenSourceBindFuncsImpl()},
        {"COMMON", emscriptenSourceBindCommonInternal()},
        {"CUSTOM", util::readFileContents(generator.emspriptenInputAbsSourceFor(m_field) + strings::bindFileSuffixStr())},
        {"VECTOR", emscriptenSourceRegisterVectorInternal()},
        {"EXTRA", emscriptenSourceBindExtraImpl()},
    };

    if (!emscriptenIsVersionOptional()) {
        return util::processTemplate(Templ, repl);
    }

    repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();

    static const std::string OptTempl = 
        "#^#FIELD#$#\n"
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$#&>()\n"
        "        .function(\"field\", &#^#CLASS_NAME#$#::field, emscripten::allow_raw_pointers())\n"
        "        #^#COMMON#$#\n"
        "        .function(\"getMode\", &#^#CLASS_NAME#$#::getMode)\n"
        "        .function(\"setMode\", &#^#CLASS_NAME#$#::setMode)\n"
        "        .function(\"isTentative\", &#^#CLASS_NAME#$#::isTentative)\n"
        "        .function(\"setTentative\", &#^#CLASS_NAME#$#::setTentative)\n"
        "        .function(\"doesExist\", &#^#CLASS_NAME#$#::doesExist)\n"
        "        .function(\"setExists\", &#^#CLASS_NAME#$#::setExists)\n"
        "        .function(\"isMissing\", &#^#CLASS_NAME#$#::isMissing)\n"
        "        .function(\"setMissing\", &#^#CLASS_NAME#$#::setMissing)\n"
        "        ;\n"
        "    emscripten::function(\"eq_#^#CLASS_NAME#$#\", &eq_#^#CLASS_NAME#$#);\n"
        "    emscripten::function(\"lt_#^#CLASS_NAME#$#\", &lt_#^#CLASS_NAME#$#);\n"
        "}\n"; 

    util::ReplacementMap optRepl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"COMMON", emscriptenSourceBindCommonInternal(true)},
        {"FIELD", util::processTemplate(Templ, repl)}
    };

    return util::processTemplate(OptTempl, optRepl);
}

std::string EmscriptenField::emscriptenSourceBindCommonInternal(bool skipVersionOptCheck) const
{
    static const std::string Templ = 
        ".function(\"readDataBuf\", &#^#CLASS_NAME#$#::readDataBuf)\n"
        ".function(\"readJsArray\", &#^#CLASS_NAME#$#::readJsArray)\n"
        ".function(\"writeDataBuf\", &#^#CLASS_NAME#$#::writeDataBuf)\n"
        ".function(\"refresh\", &#^#CLASS_NAME#$#::refresh)\n"
        ".function(\"length\", &#^#CLASS_NAME#$#::length)\n"
        ".function(\"valid\", &#^#CLASS_NAME#$#::valid)"
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
    };

    if ((!skipVersionOptCheck) && (emscriptenIsVersionOptional())) {
        repl["CLASS_NAME"].append(strings::versionOptionalFieldSuffixStr());
    }

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenHeaderMembersInternal() const
{
    util::StringsList members;
    for (auto* m : m_members) {
        auto str = m->emscriptenHeaderClass();
        if (!str.empty()) {
            members.push_back(std::move(str));
        }
    }

    return util::strListToString(members, "\n", "");
}

std::string EmscriptenField::emscriptenSourceMembersInternal() const
{
    util::StringsList members;
    for (auto* m : m_members) {
        auto str = m->emscriptenSourceCode();
        if (!str.empty()) {
            members.push_back(std::move(str));
        }
    }

    return util::strListToString(members, "\n", "");    
}

std::string EmscriptenField::emscriptenSourceRegisterVectorInternal() const
{
    if (!m_listElement) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "emscripten::class_<std::vector<#^#CLASS_NAME#$#> >(\"#^#CLASS_NAME#$#_Vector\")\n" 
        "    .constructor<>()\n"
        "    .constructor<const std::vector<#^#CLASS_NAME#$#>&>()\n"
        "    .function(\"resize\", &#^#CLASS_NAME#$#_Vector_resize)\n"
        "    .function(\"size\", &#^#CLASS_NAME#$#_Vector_size)\n"
        "    .function(\"at\", &#^#CLASS_NAME#$#_Vector_at, emscripten::allow_raw_pointers());";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenSourceExtraVectorFuncsInternal() const
{
    if (!m_listElement) {
        return strings::emptyString();
    }
        
    static const std::string Templ = 
        "void #^#CLASS_NAME#$#_Vector_resize(std::vector<#^#CLASS_NAME#$#>& vec, std::size_t count)\n"
        "{\n"
        "    vec.resize(count);\n"
        "}\n\n"
        "std::size_t #^#CLASS_NAME#$#_Vector_size(const std::vector<#^#CLASS_NAME#$#>& vec)\n"
        "{\n"
        "    return vec.size();\n"
        "}\n\n"           
        "#^#CLASS_NAME#$#* #^#CLASS_NAME#$#_Vector_at(std::vector<#^#CLASS_NAME#$#>& vec, std::size_t idx)\n"
        "{\n"
        "    return &vec.at(idx);\n"
        "}\n"        
    ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
    };

    return util::processTemplate(Templ, repl);        
}

std::string EmscriptenField::emscriptenSourceExtraCodeInternal() const
{
    auto str = emscriptenSourceExtraVectorFuncsInternal();
    if (!str.empty()) {
        str += "\n";
    }
    str += emscriptenSourceExtraCodeImpl();
    return str;
}


} // namespace commsdsl2emscripten
