//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "CNamespace.h"

#include "CField.h"
#include "CFrame.h"
#include "CGenerator.h"
#include "CInterface.h"
#include "CMessage.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

namespace
{

template <typename TElem, typename TList>
void cAddSourceFilesInternal(const TList& list, util::GenStringsList& sources)
{
    for (auto& elemPtr : list) {
        auto* elem = TElem::cCast(elemPtr.get());
        elem->cAddSourceFiles(sources);
    }
}

} // namespace

CNamespace::CNamespace(CGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    m_msgId(generator, *this),
    m_msgHandler(generator, *this),
    m_input(generator, *this)
{
}

CNamespace::~CNamespace() = default;

const CInterface* CNamespace::cInterface() const
{
    auto iFace = genFindSuitableInterface();
    if (iFace == nullptr) {
        return nullptr;
    }

    return CInterface::cCast(iFace);
}

void CNamespace::cAddSourceFiles(GenStringsList& sources) const
{
    if (!cCodeGenerationAllowed()) {
        genGenerator().genLogger().genDebug("No suitable interface for namespace, i.e. no includes " + genName());
        return;
    }

    cAddSourceFilesInternal<CNamespace>(genNamespaces(), sources);
    cAddSourceFilesInternal<CField>(genFields(), sources);
    cAddSourceFilesInternal<CMessage>(genMessages(), sources);
    cAddSourceFilesInternal<CFrame>(genFrames(), sources);

    auto* iFace = cInterface();
    iFace->cAddSourceFiles(sources);

    if (cHasValidInterfaceInternal()) {
        m_msgHandler.cAddSourceFiles(sources);
    }
}

bool CNamespace::cCodeGenerationAllowed() const
{
    auto* iFace = cInterface();
    return (iFace != nullptr) && (iFace->cIsPresent());
}

std::string CNamespace::cPrefixName() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto name = cGenerator.cNameFor(*this);
    if ((!name.empty()) && (name.back() == '_')) {
        name.resize(name.size() - 1U);
    }
    return name;
}

const CMsgId* CNamespace::cMsgId() const
{
    auto* iFace = cInterface();
    if (iFace == nullptr) {
        return nullptr;
    }

    auto* parent = iFace->genGetParent();
    auto* iFaceNs = cCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    if (iFaceNs != this) {
        return iFaceNs->cMsgId();
    }

    if (!iFace->cIsPresent()) {
        return nullptr;
    }

    return &m_msgId;
}

const CMsgHandler* CNamespace::cMsgHandler() const
{
    auto* iFace = cInterface();
    if (iFace == nullptr) {
        return nullptr;
    }

    auto* parent = iFace->genGetParent();
    auto* iFaceNs = cCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    if (iFaceNs != this) {
        return iFaceNs->cMsgHandler();
    }

    if (!iFace->cIsPresent()) {
        return nullptr;
    }

    return &m_msgHandler;
}

const CInputMessages* CNamespace::cInputMessages() const
{
    if (!cCodeGenerationAllowed()) {
        return nullptr;
    }

    return &m_input;
}

bool CNamespace::genWriteImpl() const
{
    auto* iFace = cInterface();
    if (iFace == nullptr) {
        return true;
    }

    if (!m_input.cWrite()) {
        return false;
    }

    auto* parent = iFace->genGetParent();
    auto* iFaceNs = cCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    if ((iFaceNs != this) || (!iFace->cIsPresent())) {
        return true;
    }

    return
        m_msgId.cWrite() &&
        m_msgHandler.cWrite();
}

bool CNamespace::cHasValidInterfaceInternal() const
{
    auto* iFace = cInterface();
    if ((iFace == nullptr) || (!iFace->cIsPresent())) {
        return false;
    }

    auto* parent = iFace->genGetParent();
    auto* iFaceNs = cCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    return iFaceNs == this;
}

} // namespace commsdsl2c
