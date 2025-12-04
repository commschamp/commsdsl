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

SwigNamespace::SwigNamespace(SwigGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    m_msgId(generator, *this),
    m_handler(generator, *this),
    m_input(generator, *this)
{
}

SwigNamespace::~SwigNamespace() = default;

void SwigNamespace::swigAddCodeIncludes(GenStringsList& list) const
{
    for (auto& ns : genNamespaces()) {
        SwigNamespace::swigCast(ns.get())->swigAddCodeIncludes(list);
    }

    for (auto* f : m_swigFields) {
        f->swigAddCodeIncludes(list);
    }

    for (auto& i : genInterfaces()) {
        SwigInterface::swigCast(i.get())->swigAddCodeIncludes(list);
    }

    if (!genInterfaces().empty()) {
        m_msgId.swigAddCodeIncludes(list);
    }

    for (auto& m : genMessages()) {
        SwigMessage::swigCast(m.get())->swigAddCodeIncludes(list);
    }

    for (auto& f : genFrames()) {
        SwigFrame::swigCast(f.get())->swigAddCodeIncludes(list);
    }

}

void SwigNamespace::swigAddCode(GenStringsList& list) const
{
    for (auto* f : m_swigFields) {
        f->swigAddCode(list);
    }

   if (!genInterfaces().empty()) {
        m_msgId.swigAddCode(list);
    }

    auto* msgHandler = swigMsgHandler();
    if (msgHandler != nullptr) {
        msgHandler->swigAddFwdCode(list);
    }

    auto* iFace = swigInterface();
    if (iFace != nullptr) {
        iFace->swigAddCode(list);
    }

    for (auto& i : genInterfaces()) {
        SwigInterface::swigCast(i.get())->swigAddCode(list);
    }

    for (auto& m : genMessages()) {
        SwigMessage::swigCast(m.get())->swigAddCode(list);
    }

    if (swigHasInput()) {
        m_input.swigAddCode(list);
    }

    // for (auto& f : genFrames()) {
    //     SwigFrame::swigCast(f.get())->swigAddCode(list);
    // }

    for (auto& ns : genNamespaces()) {
        SwigNamespace::swigCast(ns.get())->swigAddCode(list);
    }
}

void SwigNamespace::swigAddDef(GenStringsList& list) const
{
    if (!genInterfaces().empty()) {
        m_msgId.swigAddDef(list);
    }

    for (auto* f : m_swigFields) {
        f->swigAddDef(list);
    }

    for (auto& i : genInterfaces()) {
        SwigInterface::swigCast(i.get())->swigAddDef(list);
    }

    for (auto& m : genMessages()) {
        SwigMessage::swigCast(m.get())->swigAddDef(list);
    }

    if (!genInterfaces().empty()) {
        m_handler.swigAddDef(list);
    }

    // for (auto& f : genFrames()) {
    //     SwigFrame::swigCast(f.get())->swigAddDef(list);
    // }

    for (auto& ns : genNamespaces()) {
        SwigNamespace::swigCast(ns.get())->swigAddDef(list);
    }
}

std::string SwigNamespace::swigMsgIdClassName() const
{
    assert(!genInterfaces().empty());
    return m_msgId.swigClassName();
}

const SwigMsgId* SwigNamespace::swigMsgId() const
{
    auto* iFace = swigInterface();
    if (iFace == nullptr) {
        return nullptr;
    }

    auto* parent = iFace->genGetParent();
    auto* iFaceNs = swigCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    if (iFaceNs != this) {
        return iFaceNs->swigMsgId();
    }

    return &m_msgId;
}

const SwigMsgHandler* SwigNamespace::swigMsgHandler() const
{
    auto* iFace = swigInterface();
    if (iFace == nullptr) {
        return nullptr;
    }

    auto* parent = iFace->genGetParent();
    auto* iFaceNs = swigCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    if (iFaceNs != this) {
        return iFaceNs->swigMsgHandler();
    }

    return &m_handler;
}

const SwigInputMessages* SwigNamespace::swigInputMessages() const
{
    if (!swigHasInput()) {
        return nullptr;
    }

    return &m_input;
}

bool SwigNamespace::swigHasInput() const
{
    return (genHasFramesRecursive() && genHasMessagesRecursive());
}

const SwigInterface* SwigNamespace::swigInterface() const
{
    auto* iFace = genFindSuitableInterface();
    if (iFace == nullptr) {
        return nullptr;
    }

    return SwigInterface::swigCast(iFace);
}

bool SwigNamespace::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_swigFields = SwigField::swigTransformFieldsList(genFields());
    return true;
}

bool SwigNamespace::genWriteImpl() const
{
    if (!genInterfaces().empty()) {
        if ((!m_msgId.swigWrite()) ||
            (!m_handler.swigWrite())) {
            return false;
        }
    }

    return true;
}

} // namespace commsdsl2swig
