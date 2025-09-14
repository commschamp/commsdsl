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

#include "CNamespace.h"

#include "CField.h"
#include "CGenerator.h"
// #include "CInterface.h"
// #include "CMessage.h"
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
    GenBase(generator, parseObj, parent)
{
}   

CNamespace::~CNamespace() = default;

void CNamespace::cAddSourceFiles(GenStringsList& sources) const
{
    // TODO
    // cAddSourceFilesInternal<CNamespace>(genNamespaces(), sources);
    cAddSourceFilesInternal<CField>(genFields(), sources);
    // cAddSourceFilesInternal<CInterface>(genInterfaces(), sources);
    // cAddSourceFilesInternal<CMessage>(genMessages(), sources);
    // cAddSourceFilesInternal<CFrame>(genFrames(), sources);

    // if (!genInterfaces().empty()) {
    //     m_msgId.cAddSourceFiles(sources);
    //     m_handler.cAddSourceFiles(sources);
    // }    
}

} // namespace commsdsl2c
