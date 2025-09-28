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

#include "SwigInterface.h"

#include "SwigComms.h"
#include "SwigDataBuf.h"
#include "SwigGenerator.h"
#include "SwigMsgHandler.h"
#include "SwigMsgId.h"
#include "SwigNamespace.h"

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

SwigInterface::SwigInterface(SwigGenerator& generator, ParseInterface parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

SwigInterface::~SwigInterface() = default;

void SwigInterface::swigAddCodeIncludes(GenStringsList& list) const
{
    if (!genIsReferenced()) {
        return;
    }

    list.push_back(comms::genRelHeaderPathFor(*this, genGenerator()));
}

void SwigInterface::swigAddCode(GenStringsList& list) const
{
    if (!genIsReferenced()) {
        return;
    }

    for (auto* f : m_swigFields) {
        f->swigAddCode(list);
    }

    auto& gen = SwigGenerator::swigCast(genGenerator());

    util::GenReplacementMap repl = {
        {"COMMS_CLASS", comms::genScopeFor(*this, gen)},
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"UINT8_T", gen.swigConvertCppType("std::uint8_t")},
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"MSG_HANDLER", SwigMsgHandler::swigClassName(gen)}
    };

    std::string publicCode = util::genReadFileContents(gen.swigInputCodePathFor(*this) + strings::genPublicFileSuffixStr());
    std::string protectedCode = util::genReadFileContents(gen.swigInputCodePathFor(*this) + strings::genProtectedFileSuffixStr());
    std::string privateCode = util::genReadFileContents(gen.swigInputCodePathFor(*this) + strings::genPrivateFileSuffixStr());

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

    std::string base;
    do {
        const std::string BaseTempl =
            "#^#COMMS_CLASS#$#<\n"
            "    comms::option::app::IdInfoInterface,\n"
            "    comms::option::app::ReadIterator<#^#DATA_BUF#$#::const_iterator>,\n"
            "    comms::option::app::WriteIterator<std::back_insert_iterator<#^#DATA_BUF#$#> >,\n"
            "    comms::option::app::ValidCheckInterface,\n"
            "    comms::option::app::LengthInfoInterface,\n"
            "    comms::option::app::RefreshInterface,\n"
            "    comms::option::app::NameInterface,\n"
            "    comms::option::app::Handler<#^#MSG_HANDLER#$#>\n"
            ">";

        base = util::genProcessTemplate(BaseTempl, repl);
    } while (false);

    const std::string Templ =
        "class #^#CLASS_NAME#$# : public\n"
        "    #^#BASE#$#\n"
        "{\n"
        "    using Base = \n"
        "        #^#BASE#$#;\n"
        "public:\n"
        "    #^#FIELDS#$#\n"
        "    #^#COMMON#$#\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n";

    repl.insert({
        {"BASE", std::move(base)},
        {"FIELDS", swigFieldsAccCodeInternal()},
        {"COMMON", swigCommonCodeInternal()},
        {"PUBLIC", std::move(publicCode)},
        {"PROTECTED", std::move(protectedCode)},
        {"PRIVATE", std::move(privateCode)}
    });

    list.push_back(util::genProcessTemplate(Templ, repl));
}

void SwigInterface::swigAddDef(GenStringsList& list) const
{
    if (!genIsReferenced()) {
        return;
    }

    for (auto* f : m_swigFields) {
        f->swigAddDef(list);
    }

    auto& gen = SwigGenerator::swigCast(genGenerator());
    list.push_back("%nodefaultctor " + gen.swigClassName(*this) + ";");
    list.push_back(SwigGenerator::swigDefInclude(comms::genRelHeaderPathFor(*this, genGenerator())));
}

bool SwigInterface::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    assert(genIsReferenced());

    m_swigFields = SwigField::swigTransformFieldsList(genFields());
    return true;
}

bool SwigInterface::genWriteImpl() const
{
    assert(genIsReferenced());

    auto filePath = comms::genHeaderPathFor(*this, genGenerator());
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!genGenerator().genCreateDirectory(dirPath)) {
        return false;
    }

    auto& logger = genGenerator().genLogger();
    logger.genInfo("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#FIELDS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", SwigGenerator::swigFileGeneratedComment()},
        {"FIELDS", swigFieldDeclsInternal()},
        {"DEF", swigClassDeclInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string SwigInterface::swigFieldDeclsInternal() const
{
    GenStringsList fields;
    fields.reserve(m_swigFields.size());

    for (auto* f : m_swigFields) {
        fields.push_back(f->swigClassDecl());
    }

    return util::genStrListToString(fields, "\n", "");
}

std::string SwigInterface::swigClassDeclInternal() const
{
    static const std::string Templ =
        "class #^#MSG_HANDLER#$#;\n\n"
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    virtual ~#^#CLASS_NAME#$#();\n\n"
        "    #^#FIELDS#$#\n"
        "    const char* name() const;\n"
        "    #^#MSG_ID#$# getId() const;\n"
        "    #^#ERR_STATUS#$# read(const #^#DATA_BUF#$#& buf);\n"
        "    #^#ERR_STATUS#$# write(#^#DATA_BUF#$#& buf) const;\n"
        "    bool refresh();\n"
        "    #^#SIZE_T#$# length() const;\n"
        "    bool valid() const;\n"
        "    void dispatch(#^#MSG_HANDLER#$#& handler);\n"
        "    #^#CUSTOM#$#\n"
        "protected:\n"
        "    #^#CLASS_NAME#$#();\n"
        "    #^#CLASS_NAME#$#(const #^#CLASS_NAME#$#& other);\n"
        "};\n";

    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto* parentNs = SwigNamespace::swigCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));

    auto& gen = SwigGenerator::swigCast(genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"FIELDS", swigFieldsAccDeclInternal()},
        {"CUSTOM", util::genReadFileContents(gen.swigInputCodePathFor(*this) + strings::genAppendFileSuffixStr())},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
        {"MSG_ID", parentNs->swigMsgIdClassName()},
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"MSG_HANDLER", SwigMsgHandler::swigClassName(gen)},
        {"ERR_STATUS", SwigComms::swigErrorStatusClassName(gen)}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigInterface::swigFieldsAccDeclInternal() const
{
    GenStringsList accFuncs;
    accFuncs.reserve(m_swigFields.size());

    auto& gen = SwigGenerator::swigCast(genGenerator());
    for (auto* f : m_swigFields) {
        static const std::string Templ = {
            "#^#CLASS_NAME#$#& transportField_#^#ACC_NAME#$#();\n"
        };

        util::GenReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(f->swigGenField())},
            {"ACC_NAME", comms::genAccessName(f->swigGenField().genParseObj().parseName())}
        };

        accFuncs.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(accFuncs, "\n", "");
}

std::string SwigInterface::swigFieldsAccCodeInternal() const
{
    GenStringsList accFuncs;
    accFuncs.reserve(m_swigFields.size());

    auto& gen = SwigGenerator::swigCast(genGenerator());
    for (auto* f : m_swigFields) {
        static const std::string Templ = {
            "#^#CLASS_NAME#$#& transportField_#^#ACC_NAME#$#()\n"
            "{\n"
            "    return static_cast<#^#CLASS_NAME#$#&>(Base::transportField_#^#ACC_NAME#$#());\n"
            "}\n"
            "const #^#CLASS_NAME#$#& transportField_#^#ACC_NAME#$#() const\n"
            "{\n"
            "    return static_cast<const #^#CLASS_NAME#$#&>(Base::transportField_#^#ACC_NAME#$#());\n"
            "}\n"
        };

        util::GenReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(f->swigGenField())},
            {"ACC_NAME", comms::genAccessName(f->swigGenField().genParseObj().parseName())}
        };

        accFuncs.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(accFuncs, "\n", "");
}

std::string SwigInterface::swigCommonCodeInternal() const
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
        "    buf.reserve(buf.size() + length());\n"
        "    auto iter = std::back_inserter(buf);\n"
        "    return Base::write(iter, buf.max_size() - buf.size());\n"
        "}\n"
    ;

    auto& gen = SwigGenerator::swigCast(genGenerator());
    util::GenReplacementMap repl = {
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"ERR_STATUS", SwigComms::swigErrorStatusClassName(gen)}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2swig
