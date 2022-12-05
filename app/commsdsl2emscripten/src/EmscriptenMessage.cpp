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

#include "EmscriptenMessage.h"

#include "EmscriptenGenerator.h"

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


EmscriptenMessage::EmscriptenMessage(EmscriptenGenerator& generator, commsdsl::parse::Message dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

EmscriptenMessage::~EmscriptenMessage() = default;

bool EmscriptenMessage::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    assert(isReferenced());

    m_emscriptenFields = EmscriptenField::emscriptenTransformFieldsList(fields());
    return true;
}

bool EmscriptenMessage::writeImpl() const
{
    // TODO
    return true;
 
}

} // namespace commsdsl2emscripten
