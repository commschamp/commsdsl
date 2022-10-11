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

SwigFrame::SwigFrame(SwigGenerator& generator, commsdsl::parse::Frame dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

SwigFrame::~SwigFrame() = default;

void SwigFrame::swigAddCodeIncludes(StringsList& list) const
{
    if (!m_validFrame) {
        return;
    }

    list.push_back(comms::relHeaderPathFor(*this, generator()));
}

void SwigFrame::swigAddCode(StringsList& list) const
{
    if (!m_validFrame) {
        return;
    }

    list.push_back(swigAllMessagesCodeInternal());    

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

    list.push_back(SwigGenerator::swigDefInclude(comms::relHeaderPathFor(*this, generator())));
}

bool SwigFrame::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    for (auto& l : layers()) {
        m_swigLayers.push_back(const_cast<SwigLayer*>(SwigLayer::cast(l.get())));
        assert(m_swigLayers.back() != nullptr);
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

    assert(!m_swigLayers.empty());
    while (true) {
        bool rearanged = false;
        for (auto* l : m_swigLayers) {
            bool success = false;
            rearanged = l->swigReorder(m_swigLayers, success);

            if (!success) {
                return false;
            }

            if (rearanged) {
                // Order has changed restart from the beginning
                break;
            }
        }

        if (!rearanged) {
            // reordering is complete
            break;
        }
    }    

    return true;   
}

bool SwigFrame::writeImpl() const
{
    if (!m_validFrame) {
        return true;
    }

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
        "#^#ALL_FIELDS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"LAYERS", swigLayerDeclsInternal()},
        {"ALL_FIELDS", swigAllFieldsInternal()},
        {"DEF", swigClassDeclInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
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
    return util::strListToString(elems, "\n", "");
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
        "    #^#CUSTOM#$#\n"
        "};\n";    

    auto& gen = SwigGenerator::cast(generator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*iFace)},
        {"LAYERS", swigLayersAccDeclInternal()},
        {"CUSTOM", util::readFileContents(gen.swigInputCodePathFor(*this) + strings::appendFileSuffixStr())},
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
        {"HANDLER", SwigMsgHandler::swigClassName(gen)},
    };

    return util::processTemplate(Templ, repl);            
}

std::string SwigFrame::swigLayersAccDeclInternal() const
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

std::string SwigFrame::swigLayersAccCodeInternal() const
{
    auto& gen = SwigGenerator::cast(generator());
    util::StringsList elems;
    for (auto& l : layers()) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#& layer_#^#ACC_NAME#$#() { return static_cast<#^#CLASS_NAME#$#&>(m_frame.layer_#^#ACC_NAME#$#()); }\n";

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(*l)},
            {"ACC_NAME", comms::accessName(l->dslObj().name())}
        };

        elems.push_back(util::processTemplate(Templ, repl));
    }
    return util::strListToString(elems, "", "");
}

std::string SwigFrame::swigAllMessagesCodeInternal() const
{
    auto& gen = SwigGenerator::cast(generator());
    auto allMessages = gen.getAllMessagesIdSorted();
    util::StringsList msgList;
    msgList.reserve(allMessages.size());

    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    auto interfaceClassName = gen.swigClassName(*iFace);


    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }
        msgList.push_back(gen.swigClassName(*m));
    }

    const std::string Templ = 
        "using #^#NAME#$# =\n"
        "    std::tuple<\n"
        "        #^#MESSAGES#$#\n"
        "    >;\n";

    util::ReplacementMap repl = {
        {"NAME", strings::allMessagesStr()},
        {"MESSAGES", util::strListToString(msgList, ",\n", "")}
    };

    return util::processTemplate(Templ, repl);
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
        "                es = m_frame.read(msg, iter, len, comms::protocol::msgIndex(idx));\n"
        "            }\n"
        "            else {\n"
        "                es = m_frame.readFieldsCached(frameFields, msg, iter, len, comms::protocol::msgIndex(idx));\n"
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
        "        #^#DATA_BUF#$# outBuf(m_frame.length(msg));\n"
        "        auto writeIter = outBuf.begin();"
        "        auto es = m_frame.write(msg, writeIter, outBuf.size());\n"
        "        static_cast<void>(es);\n"
        "        assert(es == comms::ErrorStatus::Success);\n"
        "        return outBuf;\n"
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

    auto& gen = SwigGenerator::cast(generator());
    auto* iFace = gen.swigMainInterface();
    assert(iFace != nullptr);
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"INTERFACE", gen.swigClassName(*iFace)},
        {"LAYERS", swigLayersAccCodeInternal()},
        {"CUSTOM", util::readFileContents(gen.swigInputCodePathFor(*this) + strings::appendFileSuffixStr())},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
        {"COMMS_CLASS", comms::scopeFor(*this, gen)},
        {"DATA_BUF", SwigDataBuf::swigClassName(gen)},
        {"MAIN_NS", gen.protocolSchema().mainNamespace()},
        {"PROT_OPTS", SwigProtocolOptions::swigClassName(gen)},
        {"ALL_FIELDS_VALUES", util::strListToString(allFieldsAcc, ",\n", "")},
        {"FRAME_FIELDS_VALUES", util::strListToString(frameFieldsAcc, ",\n", "")},
        {"HANDLER", SwigMsgHandler::swigClassName(gen)},
    };

    if (SwigProtocolOptions::swigIsDefined(gen)) {
        repl["OPTS"] = ", " + SwigProtocolOptions::swigClassName(gen);
    }

    return util::processTemplate(Templ, repl);   
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

    auto& gen = SwigGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"FIELDS", util::strListToString(fields, "", "")}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig