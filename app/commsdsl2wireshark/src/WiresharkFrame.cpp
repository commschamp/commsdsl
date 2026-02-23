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

#include "WiresharkFrame.h"

#include "WiresharkGenerator.h"
#include "WiresharkInterface.h"
#include "WiresharkNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkFrame::WiresharkFrame(WiresharkGenerator& generator, ParseFrame parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

WiresharkFrame::~WiresharkFrame() = default;

std::string WiresharkFrame::wiresharkDissectName() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkDissectNameFor(*this);
}

std::string WiresharkFrame::wiresharkDissectCode() const
{
    static const std::string Templ =
        "#^#LAYERS#$#\n"
        "#^#PREPEND#$#\n"
        "local function #^#NAME#$##^#SUFFIX#$#(tvb, tree)\n"
        "    #^#REPLACE#$#\n"
        "    #^#BODY#$#\n"
        "end\n"
        "#^#EXTEND#$#\n"
        ;

    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    auto relPath = wiresharkGenerator.wiresharkInputDissectRelPathFor(*this);
    auto replaceFileName = relPath + strings::genReplaceFileSuffixStr();
    auto prependFileName = relPath + strings::genPrependFileSuffixStr();
    auto extendFileName = relPath + strings::genExtendFileSuffixStr();

    bool replaced = false;
    bool extended = false;
    util::GenReplacementMap repl = {
        {"LAYERS", wiresharkLayersDissectCodeInternal()},
        {"NAME", wiresharkDissectName()},
        {"REPLACE", wiresharkGenerator.genReadCodeInjectCode(replaceFileName, "Replace this function body", &replaced)},
        {"PREPEND", wiresharkGenerator.genReadCodeInjectCode(prependFileName, "Prepend here")},
        {"EXTEND", wiresharkGenerator.genReadCodeInjectCode(extendFileName, "Extend function above", &extended)},
    };

    if (!replaced) {
        repl["BODY"] = wiresharkDissectBodyInternal();
    }

    if (extended) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkFrame::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    bool success = true;
    auto reorderedLayers = genCommsOrderOfLayers(success);
    if (!success) {
        return false;
    }

    for (auto* l : reorderedLayers) {
        m_wiresharkLayers.push_back(WiresharkLayer::wiresharkCast(l));
    }

    auto* iFace = wiresharkInterfaceInternal();
    do {
        if (iFace == nullptr) {
            genGenerator().genLogger().genDebug("No valid interface for frame " + genName());
            break;
        }

        m_validFrame =
            std::all_of(
                m_wiresharkLayers.begin(), m_wiresharkLayers.end(),
                [iFace](auto* l)
                {
                    return l->wiresharkIsInterfaceSupported(*iFace);
                });
    } while (false);
    return true;
}

std::string WiresharkFrame::wiresharkDissectBodyInternal() const
{
    // TODO:
    return std::string();
}

std::string WiresharkFrame::wiresharkLayersDissectCodeInternal() const
{
    util::GenStringsList layers;
    for (auto* l : m_wiresharkLayers) {
        auto str = l->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        layers.push_back(std::move(str));
    }

    return util::genStrListToString(layers, "\n", "\n");
}

const WiresharkInterface* WiresharkFrame::wiresharkInterfaceInternal() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);
    assert(parent->genElemType() == commsdsl::gen::GenElem::GenType_Namespace);
    auto* parentNs = WiresharkNamespace::wiresharkCast(static_cast<const commsdsl::gen::GenNamespace*>(parent));
    return parentNs->wiresharkInterface();
}

} // namespace commsdsl2wireshark
