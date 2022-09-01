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

#include "SwigMessage.h"

#include "SwigGenerator.h"

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
        {"DEF", swigClassDefInternal()},
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
        fields.push_back(f->swigClassDef());
    }

    return util::strListToString(fields, "\n", "");
}

std::string SwigMessage::swigClassDefInternal() const
{
    static const std::string Templ = 
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    #^#FIELDS#$#\n"
        "    static const char* doName();\n"
        "    comms_ErrorStatus doRead(const unsigned char*& iter, unsigned long len);\n"
        "    comms_ErrorStatus doWrite(unsigned char*& iter, unsigned long len) const;\n"
        "    bool doRefresh();\n"
        "    unsigned long doLength();\n"
        "    bool doValid() const;\n"
        "    #^#CUSTOM#$#\n"
        "};\n";

    auto& gen = SwigGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"FIELDS", swigFieldsAccessInternal()},
        {"CUSTOM", util::readFileContents(gen.swigInputCodePathFor(*this) + strings::appendFileSuffixStr())}
    };

    return util::processTemplate(Templ, repl);    
}

std::string SwigMessage::swigFieldsAccessInternal() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigFields.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* f : m_swigFields) {
        static const std::string Templ = {
            "#^#CLASS_NAME#$#& field_#^#ACC_NAME#$#();\n"
            "const #^#CLASS_NAME#$#& field_#^#ACC_NAME#$#() const;\n"
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
