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

#include "SwigNamespace.h"

#include "SwigFrame.h"
#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigMessage.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{


SwigNamespace::SwigNamespace(SwigGenerator& generator, commsdsl::parse::Namespace dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

SwigNamespace::~SwigNamespace() = default;

void SwigNamespace::swigAddCodeIncludes(StringsList& list) const
{
    for (auto& ns : namespaces()) {
        SwigNamespace::cast(ns.get())->swigAddCodeIncludes(list);
    }

    for (auto* f : m_swigFields) {
        f->swigAddCodeIncludes(list);
    }

    for (auto& i : interfaces()) {
        SwigInterface::cast(i.get())->swigAddCodeIncludes(list);
    }

    for (auto& m : messages()) {
        SwigMessage::cast(m.get())->swigAddCodeIncludes(list);
    }   

    for (auto& f : frames()) {
        SwigFrame::cast(f.get())->swigAddCodeIncludes(list);
    }    
}

void SwigNamespace::swigAddCode(StringsList& list) const
{
    for (auto* f : m_swigFields) {
        f->swigAddCode(list);
    }

    for (auto& m : messages()) {
        SwigMessage::cast(m.get())->swigAddCode(list);
    }   

    // for (auto& f : frames()) {
    //     SwigFrame::cast(f.get())->swigAddCode(list);
    // }    

    for (auto& ns : namespaces()) {
        SwigNamespace::cast(ns.get())->swigAddCode(list);
    }    
}

void SwigNamespace::swigAddDef(StringsList& list) const
{
    for (auto* f : m_swigFields) {
        f->swigAddDef(list);
    }

    for (auto& i : interfaces()) {
        SwigInterface::cast(i.get())->swigAddDef(list);
    }

    for (auto& m : messages()) {
        SwigMessage::cast(m.get())->swigAddDef(list);
    }   

    // for (auto& f : frames()) {
    //     SwigFrame::cast(f.get())->swigAddDef(list);
    // } 

    for (auto& ns : namespaces()) {
        SwigNamespace::cast(ns.get())->swigAddDef(list);
    }    
}

bool SwigNamespace::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_swigFields = SwigField::swigTransformFieldsList(fields());
    return true;
}


} // namespace commsdsl2swig
