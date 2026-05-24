//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "WiresharkCustomLayer.h"

#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"

namespace strings = commsdsl::gen::strings;

namespace commsdsl2wireshark
{

WiresharkCustomLayer::WiresharkCustomLayer(WiresharkGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkCustomLayer::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(*this);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    bool replaced = false;
    auto dissectCode = wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace dissect code with", &replaced);
    if (!replaced) {
        wiresharkGenerator.genLogger().genError("Inject custom layer dissect code via \"" + replaceFileName + "\".");
        return false;
    }

    return true;
}

bool WiresharkCustomLayer::wiresharkNeedsCrcCalcImpl() const
{
    auto parseObj = genCustomLayerParseObj();
    return parseObj.parseSemanticLayerType() == ParseLayer::ParseKind::Checksum;
}

} // namespace commsdsl2wireshark
