//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "SwigFrame.h"

#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigLayer.h"

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

SwigFrame::SwigFrame(SwigGenerator& generator, commsdsl::parse::Frame dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

SwigFrame::~SwigFrame() = default;

void SwigFrame::swigAddCodeIncludes(StringsList& list) const
{
    list.push_back(comms::relHeaderPathFor(*this, generator()));
}

void SwigFrame::swigAddCode(StringsList& list) const
{
    static_cast<void>(list); // TODO
}

void SwigFrame::swigAddDef(StringsList& list) const
{
    for (auto& l : layers()) {
        auto* swigLayer = SwigLayer::cast(l.get());
        assert(swigLayer != nullptr);

        swigLayer->swigAddDef(list);
    }

    static const std::string Templ = 
        "%feature(\"director\") #^#CLASS_NAME#$#_Handler;";

    auto& gen = SwigGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
    };

    list.push_back(util::processTemplate(Templ, repl));
    list.push_back(SwigGenerator::swigDefInclude(comms::relHeaderPathFor(*this, generator())));
}

bool SwigFrame::writeImpl() const
{
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
        "#^#LAYERS#$#\n"
        "#^#HANDLER#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"LAYERS", swigLayerDeclsInternal()},
        {"HANDLER", swigHandlerDeclInternal()},
        {"DEF", swigClassDeclInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string SwigFrame::swigLayerDeclsInternal() const
{
    util::StringsList elems;
    for (auto& l : layers()) {
        auto* swigLayer = SwigLayer::cast(l.get());
        assert(swigLayer != nullptr);

        auto code = swigLayer->swigDeclCode();
        if (!code.empty()) {
            elems.push_back(std::move(code));
        }
    }
    return util::strListToString(elems, "\n", "");
}

std::string SwigFrame::swigHandlerDeclInternal() const
{
    auto& gen = SwigGenerator::cast(generator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);

    auto allMessages = gen.getAllMessagesIdSorted();
    util::StringsList handleFuncs;
    handleFuncs.reserve(allMessages.size());

    for (auto* m : allMessages) {
        static const std::string Templ = 
            "virtual handle_#^#MESSAGE#$#(#^#MESSAGE#$#& msg);\n";

        util::ReplacementMap repl = {
            {"MESSAGE", gen.swigClassName(*m)}
        };

        handleFuncs.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "class #^#CLASS_NAME#$#_Handler\n"
        "{\n"
        "public:\n"
        "     #^#HANDLE_FUNCS#$#\n"
        "     virtual handle_#^#INTERFACE#$#(#^#INTERFACE#$#& msg);\n"
        "};\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*iFace)},
        {"HANDLE_FUNCS", util::strListToString(handleFuncs, "", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigFrame::swigClassDeclInternal() const
{
    static const std::string Templ =
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    #^#LAYERS#$#\n\n"
        "    #^#SIZE_T#$# processInputData(const std::vector<#^#UINT8_T#$#>& buf, #^#CLASS_NAME#$#_Handler& handler);\n"
        "    std::vector<#^#UINT8_T#$#> writeMessage(const #^#INTERFACE#$#& msg);\n"
        "    #^#CUSTOM#$#\n"
        "};\n";    

    auto& gen = SwigGenerator::cast(generator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*iFace)},
        {"LAYERS", swigLayersAccessInternal()},
        {"CUSTOM", util::readFileContents(gen.swigInputCodePathFor(*this) + strings::appendFileSuffixStr())},
        {"UINT8_T", gen.swigConvertCppType("std::uint8_t")},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
    };

    return util::processTemplate(Templ, repl);            
}

std::string SwigFrame::swigLayersAccessInternal() const
{
    auto& gen = SwigGenerator::cast(generator());
    util::StringsList elems;
    for (auto& l : layers()) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#& layer_#^#ACC_NAME#$#();\n";

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(*l)},
            {"ACC_NAME", comms::accessName(l->dslObj().name())}
        };

        elems.push_back(util::processTemplate(Templ, repl));
    }
    return util::strListToString(elems, "", "");
}

} // namespace commsdsl2swig
