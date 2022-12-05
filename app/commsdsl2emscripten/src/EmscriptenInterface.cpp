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

#include "EmscriptenInterface.h"

// #include "EmscriptenComms.h"
// #include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
// #include "EmscriptenMsgHandler.h"
// #include "EmscriptenMsgId.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{


EmscriptenInterface::EmscriptenInterface(EmscriptenGenerator& generator, commsdsl::parse::Interface dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

EmscriptenInterface::~EmscriptenInterface() = default;

bool EmscriptenInterface::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    assert(isReferenced());

    m_emscriptenFields = EmscriptenField::emscriptenTransformFieldsList(fields());
    return true;
}

bool EmscriptenInterface::writeImpl() const
{
    // TODO
    return true;
    // assert(isReferenced());

    // auto filePath = comms::headerPathFor(*this, generator());
    // auto dirPath = util::pathUp(filePath);
    // assert(!dirPath.empty());
    // if (!generator().createDirectory(dirPath)) {
    //     return false;
    // }       

    // auto& logger = generator().logger();
    // logger.info("Generating " + filePath);

    // std::ofstream stream(filePath);
    // if (!stream) {
    //     logger.error("Failed to open \"" + filePath + "\" for writing.");
    //     return false;
    // }

    // static const std::string Templ = 
    //     "#^#GENERATED#$#\n"
    //     "#pragma once\n\n"
    //     "#^#FIELDS#$#\n"
    //     "#^#DEF#$#\n"
    // ;

    // util::ReplacementMap repl = {
    //     {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
    //     {"FIELDS", emscriptenFieldDeclsInternal()},
    //     {"DEF", emscriptenClassDeclInternal()},
    // };
    
    // stream << util::processTemplate(Templ, repl, true);
    // stream.flush();
    // return stream.good();   
}

} // namespace commsdsl2emscripten
