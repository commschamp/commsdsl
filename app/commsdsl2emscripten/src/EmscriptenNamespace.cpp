//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenNamespace.h"

#include "EmscriptenField.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"
#include "EmscriptenMessage.h"
#include "EmscriptenFrame.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

namespace
{

template <typename TElem, typename TList>
void emscriptenAddSourceFilesInternal(const TList& list, util::GenStringsList& sources)
{
    for (auto& elemPtr : list) {
        auto* elem = TElem::emscriptenCast(elemPtr.get());
        elem->emscriptenAddSourceFiles(sources);
    }
}

} // namespace

EmscriptenNamespace::EmscriptenNamespace(EmscriptenGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    m_msgId(generator, *this),
    m_handler(generator, *this),
    m_input(generator, *this)
{
}

EmscriptenNamespace::~EmscriptenNamespace() = default;

void EmscriptenNamespace::emscriptenAddSourceFiles(GenStringsList& sources) const
{
    emscriptenAddSourceFilesInternal<EmscriptenNamespace>(genNamespaces(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenField>(genFields(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenInterface>(genInterfaces(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenMessage>(genMessages(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenFrame>(genFrames(), sources);

    if (!genInterfaces().empty()) {
        m_msgId.emscriptenAddSourceFiles(sources);
        m_handler.emscriptenAddSourceFiles(sources);
    }
}

void EmscriptenNamespace::emscriptenAddCommsMessageIncludes(GenStringsList& includes) const
{
    if (emscriptenHasInput()) {
        includes.push_back(comms::genRelHeaderForInput(strings::genAllMessagesStr(), genGenerator(), *this));
        return;
    }

    for (auto& ns : genNamespaces()) {
        EmscriptenNamespace::emscriptenCast(ns.get())->emscriptenAddCommsMessageIncludes(includes);
    }
}

void EmscriptenNamespace::emscriptenAddInputMessageFwdIncludes(GenStringsList& includes) const
{
    if (emscriptenHasInput()) {
        includes.push_back(m_input.emscriptenRelFwdHeader());
        return;
    }

    for (auto& ns : genNamespaces()) {
        EmscriptenNamespace::emscriptenCast(ns.get())->emscriptenAddInputMessageFwdIncludes(includes);
    }
}

void EmscriptenNamespace::emscriptenAddInputMessageIncludes(GenStringsList& includes) const
{
    if (emscriptenHasInput()) {
        includes.push_back(m_input.emscriptenRelHeader());
        return;
    }

    for (auto& ns : genNamespaces()) {
        EmscriptenNamespace::emscriptenCast(ns.get())->emscriptenAddInputMessageIncludes(includes);
    }
}

std::string EmscriptenNamespace::emscriptenHandlerRelHeader() const
{
    return m_handler.emscriptenRelHeader();
}

std::string EmscriptenNamespace::emscriptenHandlerClassName() const
{
    return m_handler.emscriptenClassName();
}

std::string EmscriptenNamespace::emscriptenInputRelHeader() const
{
    assert(emscriptenHasInput());
    return m_input.emscriptenRelHeader();
}

std::string EmscriptenNamespace::emscriptenInputClassName() const
{
    assert(emscriptenHasInput());
    return m_input.emscriptenClassName();
}

bool EmscriptenNamespace::emscriptenHasInput() const
{
    return (genHasFramesRecursive() && genHasMessagesRecursive());
}

bool EmscriptenNamespace::genWriteImpl() const
{
    if (!genInterfaces().empty()) {
        if ((!m_msgId.emscriptenWrite()) ||
            (!m_handler.emscriptenWrite())) {
            return false;
        }
    }

    if (emscriptenHasInput()) {
        if (!m_input.emscriptenWrite()) {
            return false;
        }
    }

    return true;
}

} // namespace commsdsl2emscripten
