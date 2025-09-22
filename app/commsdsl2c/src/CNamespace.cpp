//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CNamespace.h"

#include "CField.h"
#include "CGenerator.h"
#include "CInterface.h"
#include "CMessage.h"
// #include "CFrame.h"

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
    m_msgId(generator, *this)
{
}   

CNamespace::~CNamespace() = default;

const CInterface* CNamespace::cInterface() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto* iFace = cGenerator.cForcedInterface();

    if (iFace == nullptr) {
        return cFindSuitableInterfaceInternal();
    }

    if (cIsSuitableInterface(*iFace)) {
        return iFace;
    }

    return nullptr;
}

bool CNamespace::cIsSuitableInterface(const CInterface& iFace) const
{
    auto& thisNsInterfaces = genInterfaces();
    auto iter = 
        std::find_if(
            thisNsInterfaces.begin(), thisNsInterfaces.end(), 
            [&iFace](auto& iFacePtr)
            {
                return &iFace == iFacePtr.get();
            });

    if (iter != thisNsInterfaces.end()) {
        return true;
    }

    auto* parent = genGetParent();
    if (parent->genElemType() == GenElem::GenType_Namespace) {
        return CNamespace::cCast(static_cast<const GenNamespace*>(parent))->cIsSuitableInterface(iFace);
    }    

    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto allNamespaces = cGenerator.genGetAllNamespaces();
    auto nsIter = 
        std::find_if(
            allNamespaces.begin(), allNamespaces.end(),
            [](auto* ns)
            {
                return ns->genName().empty();
            });

    if (nsIter == allNamespaces.end()) {
        return false;
    }

    auto defaultNsInterfaces = (*nsIter)->genGetAllInterfaces();
    auto iFaceIter = std::find(defaultNsInterfaces.begin(), defaultNsInterfaces.end(), &iFace);
    return iFaceIter != defaultNsInterfaces.end();
}

void CNamespace::cAddSourceFiles(GenStringsList& sources) const
{
    if (!cCodeGenerationAllowed()) {
        genGenerator().genLogger().genDebug("No suitable interface for namespace, i.e. no includes " + genName());
        return;
    }

    // TODO
    cAddSourceFilesInternal<CNamespace>(genNamespaces(), sources);
    cAddSourceFilesInternal<CField>(genFields(), sources);
    cAddSourceFilesInternal<CMessage>(genMessages(), sources);
    // cAddSourceFilesInternal<CFrame>(genFrames(), sources);

    auto* iFace = cInterface();
    iFace->cAddSourceFiles(sources);
    
    // if (!genInterfaces().empty()) {
    //     m_msgId.cAddSourceFiles(sources);
    //     m_handler.cAddSourceFiles(sources);
    // }    
}

bool CNamespace::cCodeGenerationAllowed() const
{
    auto* iFace = cInterface();
    return iFace != nullptr;
}

std::string CNamespace::cPrefixName() const
{
    auto& cGenerator = CGenerator::cCast(genGenerator());
    return cGenerator.cStructNameFor(*this);
}

const CMsgId* CNamespace::cMsgId() const
{
    if (!cCodeGenerationAllowed()) {
        return nullptr;
    }

    return &m_msgId;
}

bool CNamespace::genWriteImpl() const 
{
    if (!cCodeGenerationAllowed()) {
        return true;
    }

    return 
        m_msgId.cWrite();
}

const CInterface* CNamespace::cFindSuitableInterfaceInternal() const
{
    auto& thisNsInterfaces = genInterfaces();
    if (!thisNsInterfaces.empty()) {
        return CInterface::cCast(thisNsInterfaces.front().get());
    }

    auto* parent = genGetParent();
    if (parent->genElemType() == GenElem::GenType_Namespace) {
        return CNamespace::cCast(static_cast<const GenNamespace*>(parent))->cInterface();
    }

    auto& cGenerator = CGenerator::cCast(genGenerator());
    auto allNamespaces = cGenerator.genGetAllNamespaces();
    auto iter = 
        std::find_if(
            allNamespaces.begin(), allNamespaces.end(),
            [](auto* ns)
            {
                return ns->genName().empty();
            });

    if (iter == allNamespaces.end()) {
        return nullptr;
    }

    auto defaultNsInterfaces = (*iter)->genGetAllInterfaces();
    if (defaultNsInterfaces.empty()) {
        return nullptr;
    }

    return CInterface::cCast(defaultNsInterfaces.front());
}

} // namespace commsdsl2c
