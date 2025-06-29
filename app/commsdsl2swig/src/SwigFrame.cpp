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

#include "SwigFrame.h"

#include "SwigComms.h"
#include "SwigDataBuf.h"
#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigLayer.h"
#include "SwigMsgHandler.h"
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

SwigFrame::SwigFrame(SwigGenerator& generator, commsdsl::parse::ParseFrame dslObj, commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent)
{
}   

SwigFrame::~SwigFrame() = default;

void SwigFrame::swigAddCodeIncludes(StringsList& list) const
{
    if (!m_validFrame) {
        return;
    }

    list.push_back(comms::genRelHeaderPathFor(*this, genGenerator()));
}

void SwigFrame::swigAddCode(StringsList& list) const
{
    if (!m_validFrame) {
        return;
    }

    for (auto* l : m_swigLayers) {
        l->swigAddCode(list);
    }

    list.push_back(swigAllFieldsInternal());
    list.push_back(swigFrameCodeInternal());
}

void SwigFrame::swigAddDef(StringsList& list) const
{
    if (!m_validFrame) {
        return;
    }

    for (auto* l : m_swigLayers) {
        l->swigAddDef(list);
    }

    list.push_back(SwigGenerator::swigDefInclude(comms::genRelHeaderPathFor(*this, genGenerator())));
}

bool SwigFrame::genPrepareImpl()
{
    if (!Base::genPrepareImpl()) {
        return false;
    }

    bool success = true;
    auto reorderedLayers = getCommsOrderOfLayers(success);
    if (!success) {
        return false;
    }

    for (auto* l : reorderedLayers) {
        auto* swigLayer = SwigLayer::cast(l);
        assert(swigLayer != nullptr);
        m_swigLayers.push_back(const_cast<SwigLayer*>(swigLayer));
    }

    m_validFrame = 
        std::all_of(
            m_swigLayers.begin(), m_swigLayers.end(),
            [](auto* l)
            {
                return l->swigIsMainInterfaceSupported();
            });

    if (!m_validFrame) {
        return true;
    }

    return true;   
}

bool SwigFrame::genWriteImpl() const
{
    if (!m_validFrame) {
        return true;
    }

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
        "#^#LAYERS#$#\n"
        "#^#ALL_FIELDS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"LAYERS", swigLayerDeclsInternal()},
        {"ALL_FIELDS", swigAllFieldsInternal()},
        {"DEF", swigClassDeclInternal()},
    };
    
    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

std::string SwigFrame::swigLayerDeclsInternal() const
{
    util::StringsList elems;
    for (auto* l : m_swigLayers) {

        auto code = l->swigDeclCode();
        if (!code.empty()) {
            elems.push_back(std::move(code));
        }
    }
    return util::genStrListToString(elems, "\n", "");
}

std::string SwigFrame::swigClassDeclInternal() const
{
    static const std::string Templ =
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    #^#LAYERS#$#\n\n"
        "    #^#SIZE_T#$# processInputData(const #^#DATA_BUF#$#& buf, #^#HANDLER#$#& handler);\n"
        "    #^#SIZE_T#$# processInputDataSingleMsg(const #^#DATA_BUF#$#& buf, #^#HANDLER#$#& handler, #^#CLASS_NAME#$#_AllFields* allFields = nullptr);\n"
        "    #^#DATA_BUF#$# writeMessage(const #^#INTERFACE#$#& msg);\n"
        "    #^#ERR_STATUS#$# appendMessage(const #^#INTERFACE#$#& msg, #^#DATA_BUF#$#& buf);\n"
        "    #^#CUSTOM#$#\n"
        "};\n";    

    auto& gen = SwigGenerator::cast(genGenerator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*iFace)},
        {"LAYERS", swigLayersAccDeclInternal()},
        {"CUSTOM", util::genReadFileContents(gen.swigInputCodePathFor(*this) + strings::genAppendFileSuffixStr())},
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
        {"HANDLER", SwigMsgHandler::swigClassName(gen)},
        {"ERR_STATUS", SwigComms::swigErrorStatusClassName(gen)}
    };

    return util::genProcessTemplate(Templ, repl);            
}

std::string SwigFrame::swigLayersAccDeclInternal() const
{
    auto& gen = SwigGenerator::cast(genGenerator());
    util::StringsList elems;
    for (auto& l : genLayers()) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#& layer_#^#ACC_NAME#$#();\n";

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(*l)},
            {"ACC_NAME", comms::genAccessName(l->genParseObj().parseName())}
        };

        elems.push_back(util::genProcessTemplate(Templ, repl));
    }
    return util::genStrListToString(elems, "", "");
}

std::string SwigFrame::swigLayersAccCodeInternal() const
{
    auto& gen = SwigGenerator::cast(genGenerator());
    util::StringsList elems;
    for (auto& l : genLayers()) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#& layer_#^#ACC_NAME#$#() { return static_cast<#^#CLASS_NAME#$#&>(m_frame.layer_#^#ACC_NAME#$#()); }\n";

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(*l)},
            {"ACC_NAME", comms::genAccessName(l->genParseObj().parseName())}
        };

        elems.push_back(util::genProcessTemplate(Templ, repl));
    }
    return util::genStrListToString(elems, "", "");
}

std::string SwigFrame::swigFrameCodeInternal() const
{
    static const std::string Templ =
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    #^#LAYERS#$#\n\n"
        "    #^#SIZE_T#$# processInputData(const #^#DATA_BUF#$#& buf, #^#HANDLER#$#& handler)\n"
        "    {\n"
        "        if (buf.empty()) { return 0U; }\n"
        "        return static_cast<#^#SIZE_T#$#>(comms::processAllWithDispatch(buf.begin(), buf.size(), m_frame, handler));\n"
        "    }\n\n"
        "    #^#SIZE_T#$# processInputDataSingleMsg(const #^#DATA_BUF#$#& buf, #^#HANDLER#$#& handler, #^#CLASS_NAME#$#_AllFields* allFields = nullptr)\n"
        "    {\n"
        "        if (buf.empty()) { return 0U; }\n"
        "        #^#SIZE_T#$# consumed = 0U;\n"
        "        Frame::MsgPtr msg;\n"
        "        Frame::AllFields frameFields;\n"
        "        while (consumed < buf.size()) {\n"
        "            auto begIter = buf.begin() + consumed;\n"
        "            auto iter = begIter;\n\n"
        "            auto es = comms::ErrorStatus::Success;\n"
        "            auto len = buf.size() - consumed;\n"
        "            std::size_t idx = 0U;\n"
        "            if (allFields == nullptr) {\n"
        "                es = m_frame.read(msg, iter, len, comms::frame::msgIndex(idx));\n"
        "            }\n"
        "            else {\n"
        "                es = m_frame.readFieldsCached(frameFields, msg, iter, len, comms::frame::msgIndex(idx));\n"
        "            }\n\n"
        "            if (es == comms::ErrorStatus::NotEnoughData) {\n"
        "                return consumed;\n"
        "            }\n\n"
        "            if (es == comms::ErrorStatus::ProtocolError) {\n"
        "                ++consumed;\n"
        "                continue;\n"
        "            }\n\n"
        "            if (allFields != nullptr) {\n"
        "                auto allFieldsValues =\n"
        "                    std::tie(\n"
        "                        #^#ALL_FIELDS_VALUES#$#);\n\n"
        "                auto frameFieldsValues =\n"
        "                    std::forward_as_tuple(\n"
        "                        #^#FRAME_FIELDS_VALUES#$#);\n\n"        
        "                allFieldsValues = std::move(frameFieldsValues);\n"
        "            }\n\n"
        "            consumed += static_cast<decltype(consumed)>(std::distance(begIter, iter));\n\n"
        "            if (es == comms::ErrorStatus::Success) {\n"
        "                msg->dispatch(handler);\n"
        "            }\n"
        "            break;\n"
        "        }\n"
        "        return consumed;\n"
        "    }\n\n"        
        "    #^#DATA_BUF#$# writeMessage(const #^#INTERFACE#$#& msg)\n"
        "    {\n"
        "        #^#DATA_BUF#$# outBuf;\n"
        "        outBuf.reserve(m_frame.length(msg));\n"
        "        auto writeIter = std::back_inserter(outBuf);\n"
        "        auto es = m_frame.write(msg, writeIter, outBuf.max_size());\n"
        "        static_cast<void>(es);\n"
        "        assert(es == comms::ErrorStatus::Success);\n"
        "        return outBuf;\n"
        "    }\n\n"
        "    #^#ERR_STATUS#$# appendMessage(const #^#INTERFACE#$#& msg, #^#DATA_BUF#$#& buf)\n"
        "    {\n"
        "        buf.reserve(buf.size() + m_frame.length(msg));\n"
        "        auto writeIter = std::back_inserter(buf);\n"
        "        return m_frame.write(msg, writeIter, buf.max_size() - buf.size());\n"
        "    }\n\n"        
        "    #^#CUSTOM#$#\n\n"
        "private:\n"
        "    using Frame = #^#COMMS_CLASS#$#<#^#INTERFACE#$#, AllMessages#^#OPTS#$#>;\n"
        "    Frame m_frame;\n"
        "};\n";    

    util::StringsList allFieldsAcc;
    for (auto* l : m_swigLayers) {
        allFieldsAcc.push_back("allFields->" + l->swigFieldAccName() + ".value()");
    }

    util::StringsList frameFieldsAcc;
    for (auto idx = 0U; idx < m_swigLayers.size(); ++idx) {
        frameFieldsAcc.push_back("std::move(std::get<" + std::to_string(idx) + ">(frameFields).value())");
    }    

    auto& gen = SwigGenerator::cast(genGenerator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*iFace)},
        {"LAYERS", swigLayersAccCodeInternal()},
        {"CUSTOM", util::genReadFileContents(gen.swigInputCodePathFor(*this) + strings::genAppendFileSuffixStr())},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
        {"COMMS_CLASS", comms::genScopeFor(*this, gen)},
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"MAIN_NS", gen.genProtocolSchema().genMainNamespace()},
        {"PROT_OPTS", SwigProtocolOptions::swigClassName(gen)},
        {"ALL_FIELDS_VALUES", util::genStrListToString(allFieldsAcc, ",\n", "")},
        {"FRAME_FIELDS_VALUES", util::genStrListToString(frameFieldsAcc, ",\n", "")},
        {"HANDLER", SwigMsgHandler::swigClassName(gen)},
        {"ERR_STATUS", SwigComms::swigErrorStatusClassName(gen)},
    };

    if (SwigProtocolOptions::swigIsDefined(gen)) {
        repl["OPTS"] = ", " + SwigProtocolOptions::swigClassName(gen);
    }

    return util::genProcessTemplate(Templ, repl);   
}

std::string SwigFrame::swigAllFieldsInternal() const
{
    static const std::string Templ = 
        "struct #^#CLASS_NAME#$#_AllFields\n"
        "{\n"
        "   #^#FIELDS#$#\n"
        "};\n";

    StringsList fields;
    for (auto* l : m_swigLayers) {
        l->swigAddToAllFieldsDecl(fields);
    }

    auto& gen = SwigGenerator::cast(genGenerator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"FIELDS", util::genStrListToString(fields, "", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2swig
