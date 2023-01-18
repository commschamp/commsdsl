//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenFrame.h"

#include "EmscriptenAllMessages.h"
#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"
#include "EmscriptenLayer.h"
#include "EmscriptenMsgHandler.h"
#include "EmscriptenProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

EmscriptenFrame::EmscriptenFrame(EmscriptenGenerator& generator, commsdsl::parse::Frame dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

EmscriptenFrame::~EmscriptenFrame() = default;

void EmscriptenFrame::emscriptenAddSourceFiles(StringsList& sources) const
{
    if (!m_validFrame) {
        return;
    }

    auto& gen = EmscriptenGenerator::cast(generator());
    sources.push_back(gen.emscriptenRelSourceFor(*this));
}

bool EmscriptenFrame::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    bool success = true;
    auto reorderedLayers = getCommsOrderOfLayers(success);
    if (!success) {
        return false;
    }

    for (auto* l : reorderedLayers) {
        auto* emscriptenLayer = EmscriptenLayer::cast(l);
        assert(emscriptenLayer != nullptr);
        m_emscriptenLayers.push_back(const_cast<EmscriptenLayer*>(emscriptenLayer));
    }

    m_validFrame = 
        std::all_of(
            m_emscriptenLayers.begin(), m_emscriptenLayers.end(),
            [](auto* l)
            {
                return l->emscriptenIsMainInterfaceSupported();
            });

    return true;
}

bool EmscriptenFrame::writeImpl() const
{
    if (!m_validFrame) {
        return true;
    }

    return 
        emscriptenWriteHeaderInternal() &&
        emscriptenWriteSourceInternal();
}

bool EmscriptenFrame::emscriptenWriteHeaderInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    auto filePath = gen.emscriptenAbsHeaderFor(*this);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n\n"
        "#^#LAYERS#$#\n"
        "#^#ALL_FIELDS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"INCLUDES", emscriptenHeaderIncludesInternal()},
        {"LAYERS", emscriptenHeaderLayersInternal()},
        {"ALL_FIELDS", emscriptenHeaderAllFieldsInternal()},
        {"DEF", emscriptenHeaderClassInternal()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

bool EmscriptenFrame::emscriptenWriteSourceInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    auto filePath = gen.emscriptenAbsSourceFor(*this);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#include <iterator>\n\n"
        "#include <emscripten/bind.h>\n\n"
        "#include \"comms/process.h\"\n\n"
        "#^#LAYERS#$#\n"
        "#^#CODE#$#\n"
        "#^#BIND#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"LAYERS", emscriptenSourceLayersInternal()},
        {"CODE", emscriptenSourceCodeInternal()},
        {"BIND", emscriptenSourceBindInternal()},
        {"HEADER", gen.emscriptenRelHeaderFor(*this)},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();   
}

std::string EmscriptenFrame::emscriptenHeaderIncludesInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::StringsList includes {
        comms::relHeaderPathFor(*this, gen),
        EmscriptenDataBuf::emscriptenRelHeader(gen),
        EmscriptenMsgHandler::emscriptenRelHeader(gen),
        EmscriptenAllMessages::emscriptenRelHeader(gen),
        iFace->emscriptenRelHeader(),
    };

    EmscriptenProtocolOptions::emscriptenAddInclude(gen, includes);

    for (auto* l : m_emscriptenLayers) {
        l->emscriptenAddHeaderInclude(includes);
    }

    comms::prepareIncludeStatement(includes);
    return util::strListToString(includes, "\n", "\n");
}

std::string EmscriptenFrame::emscriptenHeaderLayersInternal() const
{
    util::StringsList fields;
    for (auto* l : m_emscriptenLayers) {
        fields.push_back(l->emscriptenHeaderClass());
    }

    return util::strListToString(fields, "\n", "\n");
}

std::string EmscriptenFrame::emscriptenHeaderAllFieldsInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    util::StringsList fields;
    util::StringsList accFuncs;

    for (auto* l : m_emscriptenLayers) {
        static const std::string FieldTempl = 
            "#^#CLASS_NAME#$#::Field #^#NAME#$#;";

        static const std::string AccTempl = 
            "#^#CLASS_NAME#$#::Field* #^#FUNC_NAME#$#()\n"
            "{\n"
            "    return &#^#NAME#$#;\n"
            "}\n";            

        util::ReplacementMap fieldRepl = {
            {"CLASS_NAME", gen.emscriptenClassName(l->layer())},
            {"NAME", l->emscriptenFieldAccName()},
            {"FUNC_NAME", l->emscriptenFieldAccFuncName()},
        };

        fields.push_back(util::processTemplate(FieldTempl, fieldRepl));
        accFuncs.push_back(util::processTemplate(AccTempl, fieldRepl));
    }

    static const std::string Templ = 
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    #^#FUNCS#$#\n"
        "private:\n"
        "    #^#FIELDS#$#\n"
        "};\n";


    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenHeaderAllFieldsNameInternal()},
        {"FIELDS", util::strListToString(fields, "\n", "")},
        {"FUNCS", util::strListToString(accFuncs, "\n", "\n")}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenFrame::emscriptenHeaderAllFieldsNameInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    return gen.emscriptenClassName(*this) + "_AllFields";
}

std::string EmscriptenFrame::emscriptenHeaderClassInternal() const
{
    static const std::string Templ = 
    "class #^#CLASS_NAME#$#\n"
    "{\n"
    "public:\n"
    "    #^#ACC#$#\n"
    "    std::size_t processInputData(const #^#DATA_BUF#$#& buf, #^#MSG_HANDER#$#& handler);\n"
    "    std::size_t processInputJsArray(const emscripten::val& buf, #^#MSG_HANDER#$#& handler);\n"
    "    std::size_t processInputDataSingleMsg(const #^#DATA_BUF#$#& buf, #^#MSG_HANDER#$#& handler, #^#ALL_FIELDS#$#* allFields = nullptr);\n"
    "    std::size_t processInputJsArraySingleMsg(const emscripten::val& buf, #^#MSG_HANDER#$#& handler, #^#ALL_FIELDS#$#* allFields = nullptr);\n"
    "    comms::ErrorStatus writeMessage(const #^#INTERFACE#$#& msg, #^#DATA_BUF#$#& buf);\n"
    "\n"
    "private:\n"
    "    using Frame = #^#COMMS_CLASS#$#<#^#INTERFACE#$#, #^#ALL_MESSAGES#$##^#OPTS#$#>;\n"
    "    Frame m_frame;\n"
    "};\n";    

    auto& gen = EmscriptenGenerator::cast(generator());
    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
        {"ACC", emscriptenHeaderLayersAccessInternal()},
        {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(gen)},
        {"MSG_HANDER", EmscriptenMsgHandler::emscriptenClassName(gen)},
        {"ALL_FIELDS", emscriptenHeaderAllFieldsNameInternal()},
        {"INTERFACE", gen.emscriptenClassName(*iFace)},
        {"ALL_MESSAGES", EmscriptenAllMessages::emscriptenClassName(gen)},
        {"COMMS_CLASS", comms::scopeFor(*this, gen)},
    };

    if (EmscriptenProtocolOptions::emscriptenIsDefined(gen)) {
        repl["OPTS"] = ", " + EmscriptenProtocolOptions::emscriptenClassName(gen);
    }

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenFrame::emscriptenHeaderLayersAccessInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    util::StringsList result;
    for (auto* l : m_emscriptenLayers) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#* layer_#^#NAME#$#()\n"
            "{\n"
            "    return static_cast<#^#CLASS_NAME#$#*>(&m_frame.layer_#^#NAME#$#());\n"
            "}";

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.emscriptenClassName(l->layer())},
            {"NAME", comms::accessName(l->layer().dslObj().name())},
        };

        result.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(result, "\n", "\n");
}

std::string EmscriptenFrame::emscriptenSourceLayersInternal() const
{
    util::StringsList result;
    for (auto* l : m_emscriptenLayers) {
        result.push_back(l->emscriptenSourceCode());
    }

    return util::strListToString(result, "\n", "\n");
}

std::string EmscriptenFrame::emscriptenSourceAllFieldsInternal() const
{
    util::StringsList fields;
    for (auto* l : m_emscriptenLayers) {
        static const std::string FieldTempl = 
            ".function(\"#^#NAME#$#\", &#^#CLASS_NAME#$#::#^#NAME#$#, emscripten::allow_raw_pointers())";

        util::ReplacementMap fieldRepl = {
            {"CLASS_NAME", emscriptenHeaderAllFieldsNameInternal()},
            {"NAME", l->emscriptenFieldAccFuncName()},
        };

        fields.push_back(util::processTemplate(FieldTempl, fieldRepl));
    }

    static const std::string Templ = 
        "emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "    .constructor<>()"
        "    .constructor<const #^#CLASS_NAME#$#&>()"
        "    #^#FIELDS#$#\n"
        "    ;\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenHeaderAllFieldsNameInternal()},
        {"FIELDS", util::strListToString(fields, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenFrame::emscriptenSourceCodeInternal() const
{
    static const std::string Templ = 
        "std::size_t #^#CLASS_NAME#$#::processInputData(const #^#DATA_BUF#$#& buf, #^#HANDLER#$#& handler)\n"
        "{\n"
        "    if (buf.empty()) { return 0U; }\n"
        "    return static_cast<std::size_t>(comms::processAllWithDispatch(buf.begin(), buf.size(), m_frame, handler));\n"
        "}\n\n"
        "std::size_t #^#CLASS_NAME#$#::processInputJsArray(const emscripten::val& buf, #^#HANDLER#$#& handler)\n"
        "{\n"
        "    return processInputData(#^#JS_ARRAY#$#(buf), handler);\n"
        "}\n\n"        
        "std::size_t #^#CLASS_NAME#$#::processInputDataSingleMsg(const #^#DATA_BUF#$#& buf, #^#HANDLER#$#& handler, #^#ALL_FIELDS#$#* allFields)\n"
        "{\n"
        "    if (buf.empty()) { return 0U; }\n"
        "    std::size_t consumed = 0U;\n"
        "    Frame::MsgPtr msg;\n"
        "    Frame::AllFields frameFields;\n"
        "    while (consumed < buf.size()) {\n"
        "        auto begIter = buf.begin() + consumed;\n"
        "        auto iter = begIter;\n\n"
        "        auto es = comms::ErrorStatus::Success;\n"
        "        auto len = buf.size() - consumed;\n"
        "        std::size_t idx = 0U;\n"
        "        if (allFields == nullptr) {\n"
        "            es = m_frame.read(msg, iter, len, comms::protocol::msgIndex(idx));\n"
        "        }\n"
        "        else {\n"
        "            es = m_frame.readFieldsCached(frameFields, msg, iter, len, comms::protocol::msgIndex(idx));\n"
        "        }\n\n"
        "        if (es == comms::ErrorStatus::NotEnoughData) {\n"
        "            return consumed;\n"
        "        }\n\n"
        "        if (es == comms::ErrorStatus::ProtocolError) {\n"
        "            ++consumed;\n"
        "            continue;\n"
        "        }\n\n"
        "        if (allFields != nullptr) {\n"
        "            auto allFieldsValues =\n"
        "                std::tie(\n"
        "                    #^#ALL_FIELDS_VALUES#$#);\n\n"
        "            auto frameFieldsValues =\n"
        "                std::forward_as_tuple(\n"
        "                    #^#FRAME_FIELDS_VALUES#$#);\n\n"    
        "            allFieldsValues = std::move(frameFieldsValues);\n"
        "        }\n\n"
        "        consumed += static_cast<decltype(consumed)>(std::distance(begIter, iter));\n\n"
        "        if (es == comms::ErrorStatus::Success) {\n"
        "            msg->dispatch(handler);\n"
        "        }\n"
        "        break;\n"
        "    }\n"
        "    return consumed;\n"
        "}\n\n"    
        "std::size_t #^#CLASS_NAME#$#::processInputJsArraySingleMsg(const emscripten::val& buf, #^#HANDLER#$#& handler, #^#ALL_FIELDS#$#* allFields)\n"
        "{\n"
        "    return processInputDataSingleMsg(#^#JS_ARRAY#$#(buf), handler, allFields);\n"
        "}\n\n"        
        "comms::ErrorStatus #^#CLASS_NAME#$#::writeMessage(const #^#INTERFACE#$#& msg, #^#DATA_BUF#$#& buf)\n"
        "{\n"
        "    buf.reserve(buf.size() + m_frame.length(msg));\n"
        "    auto writeIter = std::back_inserter(buf);\n"
        "    return m_frame.write(msg, writeIter, buf.max_size() - buf.size());\n"
        "}\n";

    util::StringsList allFieldsAcc;
    for (auto* l : m_emscriptenLayers) {
        allFieldsAcc.push_back("allFields->" + l->emscriptenFieldAccFuncName() + "()->value()");
    }

    util::StringsList frameFieldsAcc;
    for (auto idx = 0U; idx < m_emscriptenLayers.size(); ++idx) {
        frameFieldsAcc.push_back("std::move(std::get<" + std::to_string(idx) + ">(frameFields).value())");
    }    

    auto& gen = EmscriptenGenerator::cast(generator());
    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
        {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(gen)},
        {"HANDLER", EmscriptenMsgHandler::emscriptenClassName(gen)},
        {"ALL_FIELDS", emscriptenHeaderAllFieldsNameInternal()},
        {"ALL_FIELDS_VALUES", util::strListToString(allFieldsAcc, ",\n", "")},
        {"FRAME_FIELDS_VALUES", util::strListToString(frameFieldsAcc, ",\n", "")},
        {"INTERFACE", gen.emscriptenClassName(*iFace)},
        {"JS_ARRAY", EmscriptenDataBuf::emscriptenJsArrayToDataBufFuncName()}
    };
    return util::processTemplate(Templ, repl);
}

std::string EmscriptenFrame::emscriptenSourceBindInternal() const
{
    static const std::string Templ =
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    #^#ALL_FIELDS#$#\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$#&>()\n"
        "        #^#LAYERS_ACC#$#\n"
        "        .function(\"processInputData\", &#^#CLASS_NAME#$#::processInputData)\n"
        "        .function(\"processInputJsArray\", &#^#CLASS_NAME#$#::processInputJsArray)\n"
        "        .function(\"processInputDataSingleMsg\", &#^#CLASS_NAME#$#::processInputDataSingleMsg, emscripten::allow_raw_pointers())\n"
        "        .function(\"processInputJsArraySingleMsg\", &#^#CLASS_NAME#$#::processInputJsArraySingleMsg, emscripten::allow_raw_pointers())\n"
        "        .function(\"writeMessage\", &#^#CLASS_NAME#$#::writeMessage)\n"
        "        ;\n"
        "}\n";

    auto& gen = EmscriptenGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"ALL_FIELDS", emscriptenSourceAllFieldsInternal()},
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
        {"LAYERS_ACC", emscriptenSourceLayersAccBindInternal()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenFrame::emscriptenSourceLayersAccBindInternal() const
{
    auto& gen = EmscriptenGenerator::cast(generator());
    util::StringsList result;

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)},
    };
        
    for (auto* l : m_emscriptenLayers) {
        static const std::string Templ = 
            ".function(\"layer_#^#NAME#$#\", &#^#CLASS_NAME#$#::layer_#^#NAME#$#, emscripten::allow_raw_pointers())";

        repl["NAME"] = comms::accessName(l->layer().dslObj().name());
        result.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(result, "\n", "");
}

} // namespace commsdsl2emscripten
