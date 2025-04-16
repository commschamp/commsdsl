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

#include <algorithm>
#include <cassert>

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

namespace 
{

template <typename TElem, typename TList>
void emscriptenAddSourceFilesInternal(const TList& list, util::StringsList& sources)
{
    for (auto& elemPtr : list) {
        auto* elem = TElem::cast(elemPtr.get());
        elem->emscriptenAddSourceFiles(sources);
    }    
}

} // namespace 
    

EmscriptenNamespace::EmscriptenNamespace(EmscriptenGenerator& generator, commsdsl::parse::Namespace dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_msgId(generator, *this),
    m_handler(generator, *this),
    m_input(generator, *this)
{
}   

EmscriptenNamespace::~EmscriptenNamespace() = default;

void EmscriptenNamespace::emscriptenAddSourceFiles(StringsList& sources) const
{
    emscriptenAddSourceFilesInternal<EmscriptenNamespace>(namespaces(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenField>(fields(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenInterface>(interfaces(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenMessage>(messages(), sources);
    emscriptenAddSourceFilesInternal<EmscriptenFrame>(frames(), sources);

    if (!interfaces().empty()) {
        m_msgId.emscriptenAddSourceFiles(sources);
        m_handler.emscriptenAddSourceFiles(sources);
    }    
}

void EmscriptenNamespace::emscriptenAddCommsMessageIncludes(StringsList& includes) const
{
    if (emscriptenHasInput()) {
        includes.push_back(comms::relHeaderForInput(strings::allMessagesStr(), generator(), *this));
        return;
    }

    for (auto& ns : namespaces()) {
        EmscriptenNamespace::cast(ns.get())->emscriptenAddCommsMessageIncludes(includes);
    }
}

void EmscriptenNamespace::emscriptenAddInputMessageFwdIncludes(StringsList& includes) const
{
    if (emscriptenHasInput()) {
        includes.push_back(m_input.emscriptenRelFwdHeader());
        return;
    }

    for (auto& ns : namespaces()) {
        EmscriptenNamespace::cast(ns.get())->emscriptenAddInputMessageFwdIncludes(includes);
    }
}

void EmscriptenNamespace::emscriptenAddInputMessageIncludes(StringsList& includes) const
{
    if (emscriptenHasInput()) {
        includes.push_back(m_input.emscriptenRelHeader());
        return;
    }

    for (auto& ns : namespaces()) {
        EmscriptenNamespace::cast(ns.get())->emscriptenAddInputMessageIncludes(includes);
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
    return (hasFramesRecursive() && hasMessagesRecursive());
}

bool EmscriptenNamespace::writeImpl() const
{
    if (!interfaces().empty()) {
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
